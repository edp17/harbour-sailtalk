#include "webrtcmanager.h"

#include <QDebug>

WebRtcManager::WebRtcManager(QObject *parent)
    : QObject(parent)
{
}

WebRtcManager::~WebRtcManager()
{
    destroyPipeline();
}

void WebRtcManager::createPipeline()
{
    destroyPipeline();

    GError *error = nullptr;

    const gchar *pipelineDesc =
        "webrtcbin name=webrtc stun-server=stun://stun.l.google.com:19302 "
        "autoaudiosrc ! audioconvert ! audioresample ! opusenc ! rtpopuspay pt=111 ! "
        "application/x-rtp,media=audio,encoding-name=OPUS,payload=111 ! webrtc.";

    qDebug() << "SAILTALK createPipeline";

    m_pipeline = gst_parse_launch(pipelineDesc, &error);
    if (!m_pipeline) {
        const QString message = error
                ? QString::fromUtf8(error->message)
                : QStringLiteral("Pipeline creation failed");
        if (error)
            g_error_free(error);
        emit errorOccurred(message);
        return;
    }

    m_webrtcbin = gst_bin_get_by_name(GST_BIN(m_pipeline), "webrtc");
    if (!m_webrtcbin) {
        emit errorOccurred(QStringLiteral("Failed to find webrtc element in pipeline"));
        destroyPipeline();
        return;
    }

    g_signal_connect(m_webrtcbin, "on-negotiation-needed",
                     G_CALLBACK(WebRtcManager::onNegotiationNeeded), this);

    g_signal_connect(m_webrtcbin, "on-ice-candidate",
                     G_CALLBACK(WebRtcManager::onIceCandidate), this);

    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

void WebRtcManager::destroyPipeline()
{
    if (m_webrtcbin) {
        gst_object_unref(m_webrtcbin);
        m_webrtcbin = nullptr;
    }

    if (m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
    }
}

void WebRtcManager::startCall(const QString &peerId)
{
    qDebug() << "SAILTALK startCall peerId =" << peerId;

    m_currentPeer = peerId;
    createPipeline();

    m_isCaller = true;
    emit callStateChanged(QStringLiteral("dialing"));
}

void WebRtcManager::createOffer()
{
    qDebug() << "SAILTALK createOffer";

    GstPromise *promise = gst_promise_new_with_change_func(
        [](GstPromise *promise, gpointer user_data) {
            WebRtcManager *self = static_cast<WebRtcManager *>(user_data);

            const GstStructure *reply = gst_promise_get_reply(promise);
            if (!reply) {
                emit self->errorOccurred(QStringLiteral("create-offer returned no reply"));
                gst_promise_unref(promise);
                return;
            }

            GstWebRTCSessionDescription *offer = nullptr;
            gst_structure_get(reply,
                              "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer,
                              nullptr);

            if (!offer) {
                emit self->errorOccurred(QStringLiteral("create-offer returned no offer"));
                gst_promise_unref(promise);
                return;
            }

            // Important: set local description too
            g_signal_emit_by_name(self->m_webrtcbin, "set-local-description", offer, nullptr);

            gchar *sdp = gst_sdp_message_as_text(offer->sdp);
            emit self->localOfferReady(self->m_currentPeer, QString::fromUtf8(sdp));

            g_free(sdp);
            gst_webrtc_session_description_free(offer);
            gst_promise_unref(promise);
        },
        this,
        nullptr);

    g_signal_emit_by_name(m_webrtcbin, "create-offer", nullptr, promise);
}

void WebRtcManager::createAnswer()
{
    qDebug() << "SAILTALK createAnswer";

    GstPromise *promise = gst_promise_new_with_change_func(
        [](GstPromise *promise, gpointer user_data) {
            WebRtcManager *self = static_cast<WebRtcManager *>(user_data);

            const GstStructure *reply = gst_promise_get_reply(promise);
            if (!reply) {
                emit self->errorOccurred(QStringLiteral("create-answer returned no reply"));
                gst_promise_unref(promise);
                return;
            }

            GstWebRTCSessionDescription *answer = nullptr;
            gst_structure_get(reply,
                              "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answer,
                              nullptr);

            if (!answer) {
                emit self->errorOccurred(QStringLiteral("create-answer returned no answer"));
                gst_promise_unref(promise);
                return;
            }

            // Important: set local description too
            g_signal_emit_by_name(self->m_webrtcbin, "set-local-description", answer, nullptr);

            gchar *sdp = gst_sdp_message_as_text(answer->sdp);
            emit self->localAnswerReady(self->m_currentPeer, QString::fromUtf8(sdp));
            emit self->callStateChanged(QStringLiteral("connected"));

            g_free(sdp);
            gst_webrtc_session_description_free(answer);
            gst_promise_unref(promise);
        },
        this,
        nullptr);

    g_signal_emit_by_name(m_webrtcbin, "create-answer", nullptr, promise);
}

void WebRtcManager::onNegotiationNeeded(GstElement *, gpointer user_data)
{
    auto *self = static_cast<WebRtcManager *>(user_data);
    if (!self->m_isCaller) {
        qDebug() << "SAILTALK ignoring negotiation-needed on callee side";
        return;
    }
    self->createOffer();
}

void WebRtcManager::onIceCandidate(GstElement *, guint mlineindex, gchar *candidate, gpointer user_data)
{
    auto *self = static_cast<WebRtcManager *>(user_data);

    emit self->localIceCandidateReady(
        self->m_currentPeer,
        static_cast<int>(mlineindex),
        QString::fromUtf8(candidate)
    );
}

void WebRtcManager::handleRemoteOffer(const QString &fromPeer, const QString &sdp)
{
    qDebug() << "SAILTALK received offer from" << fromPeer;

    m_currentPeer = fromPeer;
    createPipeline();
    m_isCaller = false;

    emit callStateChanged(QStringLiteral("incoming"));

    GstSDPMessage *sdpMsg = nullptr;
    gst_sdp_message_new(&sdpMsg);

    const QByteArray sdpUtf8 = sdp.toUtf8();
    gst_sdp_message_parse_buffer(reinterpret_cast<const guint8 *>(sdpUtf8.constData()),
                                 static_cast<guint>(sdpUtf8.size()),
                                 sdpMsg);

    GstWebRTCSessionDescription *offer =
        gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, sdpMsg);

    g_signal_emit_by_name(m_webrtcbin, "set-remote-description", offer, nullptr);

    createAnswer();

    emit callStateChanged(QStringLiteral("connecting"));

    gst_webrtc_session_description_free(offer);
}

void WebRtcManager::handleRemoteAnswer(const QString &fromPeer, const QString &sdp)
{
    qDebug() << "SAILTALK received answer from" << fromPeer;

    const QByteArray sdpUtf8 = sdp.toUtf8();

    GstSDPMessage *sdpMsg = nullptr;
    gst_sdp_message_new(&sdpMsg);
    gst_sdp_message_parse_buffer(reinterpret_cast<const guint8 *>(sdpUtf8.constData()),
                                 static_cast<guint>(sdpUtf8.size()),
                                 sdpMsg);

    GstWebRTCSessionDescription *answer =
        gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_ANSWER, sdpMsg);

    g_signal_emit_by_name(m_webrtcbin, "set-remote-description", answer, nullptr);

    emit callStateChanged(QStringLiteral("connected"));

    gst_webrtc_session_description_free(answer);
}

void WebRtcManager::handleRemoteIceCandidate(const QString &fromPeer, int mlineIndex, const QString &candidate)
{
    qDebug() << "SAILTALK received ICE from" << fromPeer
             << "mline =" << mlineIndex
             << "candidate =" << candidate;

    if (!m_webrtcbin)
        return;

    g_signal_emit_by_name(m_webrtcbin, "add-ice-candidate",
                          static_cast<guint>(mlineIndex),
                          candidate.toUtf8().constData());
}

void WebRtcManager::hangUp()
{
    qDebug() << "SAILTALK hangUp";

    destroyPipeline();
    m_currentPeer.clear();
    emit callStateChanged(QStringLiteral("idle"));
}

void WebRtcManager::setMute(bool mute)
{
    m_muted = mute;
}

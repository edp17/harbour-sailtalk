#include "appengine.h"
#include "webrtcmanager.h"
#include "signalingclient.h"

#include <QProcess>
#include <QUuid>
#include <QSettings>

AppEngine::AppEngine(QObject *parent)
    : QObject(parent),
      m_callState("idle"),
      m_webrtc(new WebRtcManager(this)),
      m_signaling(new SignalingClient(this))
{
    QSettings settings;

    // Persistent own device ID
    m_ownId = settings.value("identity/ownId").toString();
    if (m_ownId.isEmpty()) {
        m_ownId = QUuid::createUuid().toString().remove("{").remove("}");
        settings.setValue("identity/ownId", m_ownId);
    }

    // Persistent last-used peer ID
    m_peerId = settings.value("identity/peerId").toString();

    m_speakerMode = settings.value("audio/speakerMode", false).toBool();

    // Hardcoded signaling server
    m_signalingUrl = QStringLiteral("ws://192.168.1.90:8585");

    connect(m_signaling, &SignalingClient::connectedChanged,
            this, [this]() {
                emit connectedToSignalingChanged();

                if (!m_signaling->isConnected() && m_callState != QStringLiteral("idle")) {
                    m_webrtc->hangUp();
                }
            });

    connect(m_signaling, &SignalingClient::errorOccurred,
            this, &AppEngine::errorOccurred);

    connect(m_webrtc, &WebRtcManager::callStateChanged,
            this, [this](const QString &state) {
                m_callState = state;
                emit callStateChanged();
            });

    connect(m_webrtc, &WebRtcManager::errorOccurred,
            this, &AppEngine::errorOccurred);

    connect(m_signaling, &SignalingClient::offerReceived,
            m_webrtc, &WebRtcManager::handleRemoteOffer);

    connect(m_signaling, &SignalingClient::answerReceived,
            m_webrtc, &WebRtcManager::handleRemoteAnswer);

    connect(m_signaling, &SignalingClient::iceCandidateReceived,
            m_webrtc, &WebRtcManager::handleRemoteIceCandidate);

    connect(m_webrtc, &WebRtcManager::localOfferReady,
            m_signaling, &SignalingClient::sendOffer);

    connect(m_webrtc, &WebRtcManager::localAnswerReady,
            m_signaling, &SignalingClient::sendAnswer);

    connect(m_webrtc, &WebRtcManager::localIceCandidateReady,
            m_signaling, &SignalingClient::sendIceCandidate);

    connect(m_signaling, &SignalingClient::hangupReceived,
            m_webrtc, &WebRtcManager::handleRemoteHangup);

    connect(m_signaling, &SignalingClient::rejectReceived,
            m_webrtc, &WebRtcManager::handleRemoteReject);

    connect(m_webrtc, &WebRtcManager::rejectOutgoingCallRequested,
            m_signaling, &SignalingClient::sendReject);

    connect(m_signaling, &SignalingClient::peerDisconnected,
            m_webrtc, &WebRtcManager::handlePeerDisconnected);

    connect(m_signaling, &SignalingClient::presenceReceived,
            this, [this](const QStringList &online) {
                m_onlinePeers = online;
                emit onlinePeersChanged();
            });

    {
        const QString port = m_speakerMode
                ? QStringLiteral("output-speaker")
                : QStringLiteral("output-earpiece");

        QProcess::execute(QStringLiteral("pactl"),
                          QStringList()
                              << QStringLiteral("set-sink-port")
                              << QStringLiteral("sink.primary_output")
                              << port);
    }
}

AppEngine::~AppEngine() = default;

QString AppEngine::ownId() const { return m_ownId; }
QString AppEngine::peerId() const { return m_peerId; }
QString AppEngine::signalingUrl() const { return m_signalingUrl; }
QString AppEngine::callState() const { return m_callState; }
bool AppEngine::connectedToSignaling() const { return m_signaling->isConnected(); }

void AppEngine::setPeerId(const QString &value)
{
    if (m_peerId == value)
        return;

    m_peerId = value;

    QSettings settings;
    settings.setValue("identity/peerId", m_peerId);

    emit peerIdChanged();
}

void AppEngine::setSignalingUrl(const QString &value)
{
    if (m_signalingUrl == value)
        return;
    m_signalingUrl = value;
    emit signalingUrlChanged();
}

void AppEngine::connectSignaling()
{
    m_signaling->connectToServer(m_signalingUrl, m_ownId);
}

void AppEngine::disconnectSignaling()
{
    // Tear down any active or pending call locally first.
    m_webrtc->hangUp();

    m_signaling->disconnectFromServer();
}

void AppEngine::startOutgoingCall()
{
    m_webrtc->startCall(m_peerId);
}

void AppEngine::hangUp()
{
    if (!m_peerId.isEmpty())
        m_signaling->sendHangup(m_peerId);

    m_webrtc->hangUp();
}

void AppEngine::setMute(bool mute)
{
    m_webrtc->setMute(mute);
}

void AppEngine::acceptIncomingCall()
{
    m_webrtc->acceptIncomingCall();
}

void AppEngine::rejectIncomingCall()
{
    m_webrtc->rejectIncomingCall();
}

bool AppEngine::speakerMode() const
{
    return m_speakerMode;
}

void AppEngine::setSpeakerMode(bool enabled)
{
    if (m_speakerMode == enabled)
        return;

    m_speakerMode = enabled;

    QSettings settings;
    settings.setValue("audio/speakerMode", m_speakerMode);

    const QString port = m_speakerMode
            ? QStringLiteral("output-speaker")
            : QStringLiteral("output-earpiece");

    QProcess::execute(QStringLiteral("pactl"),
                      QStringList()
                          << QStringLiteral("set-sink-port")
                          << QStringLiteral("sink.primary_output")
                          << port);

    emit speakerModeChanged();
}

QStringList AppEngine::onlinePeers() const
{
    return m_onlinePeers;
}
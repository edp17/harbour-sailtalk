#include "appengine.h"
#include "webrtcmanager.h"
#include "signalingclient.h"

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

    // Hardcoded signaling server
    m_signalingUrl = QStringLiteral("ws://192.168.1.90:8585");

    connect(m_signaling, &SignalingClient::connectedChanged,
            this, &AppEngine::connectedToSignalingChanged);

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

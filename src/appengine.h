#pragma once

#include <QObject>
#include <QString>

class WebRtcManager;
class SignalingClient;

class AppEngine : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString ownId READ ownId NOTIFY ownIdChanged)
    Q_PROPERTY(QString peerId READ peerId WRITE setPeerId NOTIFY peerIdChanged)
    Q_PROPERTY(QString signalingUrl READ signalingUrl WRITE setSignalingUrl NOTIFY signalingUrlChanged)
    Q_PROPERTY(QString callState READ callState NOTIFY callStateChanged)
    Q_PROPERTY(bool connectedToSignaling READ connectedToSignaling NOTIFY connectedToSignalingChanged)

public:
    explicit AppEngine(QObject *parent = nullptr);
    ~AppEngine() override;

    QString ownId() const;
    QString peerId() const;
    void setPeerId(const QString &value);

    QString signalingUrl() const;
    void setSignalingUrl(const QString &value);

    QString callState() const;
    bool connectedToSignaling() const;

    Q_INVOKABLE void connectSignaling();
    Q_INVOKABLE void disconnectSignaling();
    Q_INVOKABLE void startOutgoingCall();
    Q_INVOKABLE void hangUp();
    Q_INVOKABLE void setMute(bool mute);

signals:
    void ownIdChanged();
    void peerIdChanged();
    void signalingUrlChanged();
    void callStateChanged();
    void connectedToSignalingChanged();
    void errorOccurred(const QString &message);
    void incomingCall(const QString &fromPeer);

private:
    QString m_ownId;
    QString m_peerId;
    QString m_signalingUrl;
    QString m_callState;

    WebRtcManager *m_webrtc;
    SignalingClient *m_signaling;
};

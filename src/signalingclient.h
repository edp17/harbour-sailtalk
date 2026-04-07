#pragma once

#include <QObject>
#include <QString>

class QWebSocket;

class SignalingClient : public QObject
{
    Q_OBJECT

public:
    explicit SignalingClient(QObject *parent = nullptr);
    ~SignalingClient() override;

    void connectToServer(const QString &url, const QString &ownId);
    void disconnectFromServer();
    bool isConnected() const;

public slots:
    void sendOffer(const QString &toPeer, const QString &sdp);
    void sendAnswer(const QString &toPeer, const QString &sdp);
    void sendIceCandidate(const QString &toPeer, int mlineIndex, const QString &candidate);
    void sendHangup(const QString &toPeer);

signals:
    void connectedChanged();
    void offerReceived(const QString &fromPeer, const QString &sdp);
    void answerReceived(const QString &fromPeer, const QString &sdp);
    void iceCandidateReceived(const QString &fromPeer, int mlineIndex, const QString &candidate);
    void hangupReceived(const QString &fromPeer);
    void errorOccurred(const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);

private:
    void sendJson(const QByteArray &json);

    QWebSocket *m_socket = nullptr;
    QString m_ownId;
    bool m_connected = false;
};

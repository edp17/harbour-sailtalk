#include "signalingclient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>
#include <QJsonArray>
#include <QStringList>
#include <QtWebSockets/QWebSocket>

SignalingClient::SignalingClient(QObject *parent)
    : QObject(parent),
      m_socket(new QWebSocket())
{
    m_socket->setParent(this);

    connect(m_socket, &QWebSocket::connected,
            this, &SignalingClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected,
            this, &SignalingClient::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived,
            this, &SignalingClient::onTextMessageReceived);
}

SignalingClient::~SignalingClient() = default;

void SignalingClient::connectToServer(const QString &url, const QString &ownId)
{
    qDebug() << "SAILTALK connectToServer url =" << url << "ownId =" << ownId;
    m_ownId = ownId;
    m_socket->open(QUrl(url));
}

void SignalingClient::disconnectFromServer()
{
    qDebug() << "SAILTALK disconnectFromServer";
    m_socket->close();
}

bool SignalingClient::isConnected() const
{
    return m_connected;
}

void SignalingClient::onConnected()
{
    qDebug() << "SAILTALK websocket connected";

    m_connected = true;
    emit connectedChanged();

    QJsonObject hello;
    hello["type"] = "register";
    hello["id"] = m_ownId;
    sendJson(QJsonDocument(hello).toJson(QJsonDocument::Compact));
}

void SignalingClient::onDisconnected()
{
    qDebug() << "SAILTALK websocket disconnected";

    m_connected = false;
    emit connectedChanged();
}

void SignalingClient::sendOffer(const QString &toPeer, const QString &sdp)
{
    QJsonObject obj;
    obj["type"] = "offer";
    obj["to"] = toPeer;
    obj["from"] = m_ownId;
    obj["sdp"] = sdp;
    sendJson(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SignalingClient::sendAnswer(const QString &toPeer, const QString &sdp)
{
    QJsonObject obj;
    obj["type"] = "answer";
    obj["to"] = toPeer;
    obj["from"] = m_ownId;
    obj["sdp"] = sdp;
    sendJson(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SignalingClient::sendIceCandidate(const QString &toPeer, int mlineIndex, const QString &candidate)
{
    QJsonObject obj;
    obj["type"] = "ice";
    obj["to"] = toPeer;
    obj["from"] = m_ownId;
    obj["sdpMLineIndex"] = mlineIndex;
    obj["candidate"] = candidate;
    sendJson(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SignalingClient::sendHangup(const QString &toPeer)
{
    QJsonObject obj;
    obj["type"] = "hangup";
    obj["to"] = toPeer;
    obj["from"] = m_ownId;
    sendJson(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SignalingClient::sendReject(const QString &toPeer)
{
    QJsonObject obj;
    obj["type"] = "reject";
    obj["to"] = toPeer;
    obj["from"] = m_ownId;
    sendJson(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SignalingClient::onTextMessageReceived(const QString &message)
{
    const auto doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        emit errorOccurred(QStringLiteral("Invalid JSON from server"));
        return;
    }

    const auto obj = doc.object();
    const QString type = obj.value("type").toString();
    const QString from = obj.value("from").toString();

    if (type == "offer") {
        emit offerReceived(from, obj.value("sdp").toString());
    } else if (type == "answer") {
        emit answerReceived(from, obj.value("sdp").toString());
    } else if (type == "ice") {
        emit iceCandidateReceived(
            from,
            obj.value("sdpMLineIndex").toInt(),
            obj.value("candidate").toString()
        );
    } else if (type == "hangup") {
        emit hangupReceived(from);
    } else if (type == "reject") {
        emit rejectReceived(from);
    } else if (type == "peer-disconnected") {
        emit peerDisconnected(from);
    } else if (type == "presence") {
        QStringList online;
        const QJsonArray arr = obj.value("online").toArray();
        for (const QJsonValue &v : arr)
            online.append(v.toString());
        emit presenceReceived(online);
    } else if (type == "registered") {
        qDebug() << "SAILTALK registered as" << obj.value("id").toString();
    } else if (type == "server-info") {
        qDebug() << "SAILTALK server-info:" << obj.value("message").toString();
    } else if (type == "error") {
        emit errorOccurred(obj.value("message").toString());
    } else {
        emit errorOccurred(QStringLiteral("Unknown message type from server: %1").arg(type));
    }
}

void SignalingClient::sendJson(const QByteArray &json)
{
    if (m_socket->isValid())
        m_socket->sendTextMessage(QString::fromUtf8(json));
}

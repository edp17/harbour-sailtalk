#pragma once

#include <QObject>
#include <QString>

#define GST_USE_UNSTABLE_API
#include <gst/gst.h>
#include <gst/webrtc/webrtc.h>

class WebRtcManager : public QObject
{
    Q_OBJECT

public:
    explicit WebRtcManager(QObject *parent = nullptr);
    ~WebRtcManager() override;

    void startCall(const QString &peerId);
    void hangUp();
    void setMute(bool mute);

public slots:
    void handleRemoteOffer(const QString &fromPeer, const QString &sdp);
    void handleRemoteAnswer(const QString &fromPeer, const QString &sdp);
    void handleRemoteIceCandidate(const QString &fromPeer, int mlineIndex, const QString &candidate);
    void handleRemoteHangup(const QString &fromPeer);

signals:
    void callStateChanged(const QString &state);
    void errorOccurred(const QString &message);

    void localOfferReady(const QString &toPeer, const QString &sdp);
    void localAnswerReady(const QString &toPeer, const QString &sdp);
    void localIceCandidateReady(const QString &toPeer, int mlineIndex, const QString &candidate);

private:
    void createPipeline();
    void destroyPipeline();

    void createOffer();
    void createAnswer();
    void addIncomingAudioBranch(GstPad *srcPad);

    static void onNegotiationNeeded(GstElement *webrtc, gpointer user_data);
    static void onIceCandidate(GstElement *webrtc, guint mlineindex, gchar *candidate, gpointer user_data);
    static void onPadAdded(GstElement *webrtc, GstPad *newPad, gpointer user_data);

    QString m_currentPeer;
    bool m_muted = false;
    bool m_isCaller = false;

    GstElement *m_pipeline = nullptr;
    GstElement *m_webrtcbin = nullptr;
};

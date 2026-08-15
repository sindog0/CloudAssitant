#pragma once

#include "PacketReader.h"
#include "TcpConnection.h"

#include <QCursor>
#include <QScreen>
#include <QString>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

struct PacketHead
{
    quint16 type = 0;
    quint32 length = 0;
};

class SigConnection : public TcpConnection
{
public:
    enum class UserType
    {
        Controlled,  // 被控端
        Controlling  // 控制端
    };

    enum class State
    {
        None,
        Idle,
        Puller,
        Pusher
    };

    enum class MessageType : quint16
    {
        Join = 1,//  连接请求
        ObtainStream,//  获取流请求
        PlayStream,//  播放流请求
        CreateStream,//  创建流请求
        DeleteStream,
        MouseEvent,//  鼠标事件
        MouseMoveEvent,//  鼠标移动事件
        KeyEvent,//  键盘事件
        WheelEvent//  滚轮事件
    };

    using StopStreamCallBack = std::function<void()>;
    using StartStreamCallBack = std::function<bool(const QString& streamAddr)>;

    SigConnection(TaskScheduler* scheduler,
                  int sockfd,
                  const QString& code,
                  UserType type = UserType::Controlled);
    ~SigConnection() override;

    bool isIdle() const { return state_ == State::Idle; }
    bool isPusher() const { return state_ == State::Pusher; }
    bool isPuller() const { return state_ == State::Puller; }
    bool isNone() const { return state_ == State::None; }

    void SetStartStreamCallBack(StartStreamCallBack cb) { startStreamCb_ = std::move(cb); }
    void SetStopStreamCallBack(StopStreamCallBack cb) { stopStreamCb_ = std::move(cb); }

private:
    bool OnRead(PacketReader& reader);
    void OnClose();
    void HandleMessage(const PacketHead& head, const std::vector<char>& payload);

    qint32 Join();
    qint32 obtainStream();

    void doJoin(const PacketHead& head, const std::vector<char>& payload);
    void doPlayStream(const PacketHead& head, const std::vector<char>& payload);
    void doCreateStream(const PacketHead& head, const std::vector<char>& payload);
    void doDeleteStream(const PacketHead& head, const std::vector<char>& payload);

    void doMouseEvent(const PacketHead& head, const std::vector<char>& payload);
    void doMouseMoveEvent(const PacketHead& head, const std::vector<char>& payload);
    void doKeyEvent(const PacketHead& head, const std::vector<char>& payload);
    void doWheelEvent(const PacketHead& head, const std::vector<char>& payload);

    qint32 SendPacket(MessageType type, const QByteArray& payload = {});

private:
    bool quit_ = false;
    State state_ = State::None;
    QString code_;
    UserType type_;
    QScreen* screen_ = nullptr;
    StopStreamCallBack stopStreamCb_;
    StartStreamCallBack startStreamCb_ = [](const QString&) { return true; };
    std::unique_ptr<std::thread> eventThread_;
};

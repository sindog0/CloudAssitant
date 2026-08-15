#include "SigConnection.h"

#include "PacketWriter.h"

#include <QByteArray>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace
{
constexpr quint32 kPacketHeadSize = sizeof(quint16) + sizeof(quint32);

QString payloadToString(const std::vector<char>& payload)
{
    return QString::fromUtf8(payload.data(), static_cast<int>(payload.size()));
}
}

SigConnection::SigConnection(TaskScheduler* scheduler,
                             int sockfd,
                             const QString& code,
                             UserType type)
    : TcpConnection(sockfd, scheduler),
      code_(code),
      type_(type),
      screen_(QGuiApplication::primaryScreen())
{
    SetReadCallback([this](TcpConnectionPtr, PacketReader& reader) {
        return OnRead(reader);
    });

    SetCloseCallback([this](TcpConnectionPtr) {
        OnClose();
    });

    Join();
}

SigConnection::~SigConnection()
{
    quit_ = true;
    if (eventThread_ && eventThread_->joinable()) {
        eventThread_->join();
    }
}

bool SigConnection::OnRead(PacketReader& reader)
{
    while (reader.ReadableBytes() >= kPacketHeadSize) {
        const auto type = reader.PeekUint16BE();
        const auto length = reader.PeekUint32BE(sizeof(quint16));

        if (reader.ReadableBytes() < kPacketHeadSize + length) {
            break;
        }

        reader.ReadUint16BE();
        reader.ReadUint32BE();
        std::vector<char> payload = reader.ReadBytes(length);

        HandleMessage(PacketHead{type, length}, payload);
    }

    return true;
}

void SigConnection::OnClose()
{
    quit_ = true;
    state_ = State::None;

    if (stopStreamCb_) {
        stopStreamCb_();
    }
}

void SigConnection::HandleMessage(const PacketHead& head, const std::vector<char>& payload)
{
    switch (static_cast<MessageType>(head.type)) {
    case MessageType::Join:
        doJoin(head, payload);
        break;
    case MessageType::PlayStream:
        doPlayStream(head, payload);
        break;
    case MessageType::CreateStream:
        doCreateStream(head, payload);
        break;
    case MessageType::DeleteStream:
        doDeleteStream(head, payload);
        break;
    case MessageType::MouseEvent:
        doMouseEvent(head, payload);
        break;
    case MessageType::MouseMoveEvent:
        doMouseMoveEvent(head, payload);
        break;
    case MessageType::KeyEvent:
        doKeyEvent(head, payload);
        break;
    case MessageType::WheelEvent:
        doWheelEvent(head, payload);
        break;
    case MessageType::ObtainStream:
        obtainStream();
        break;
    }
}

qint32 SigConnection::Join()
{
    const QByteArray payload = code_.toUtf8();
    return SendPacket(MessageType::Join, payload);
}

qint32 SigConnection::obtainStream()
{
    return SendPacket(MessageType::ObtainStream);
}

void SigConnection::doJoin(const PacketHead&, const std::vector<char>&)
{
    state_ = State::Idle;
}

void SigConnection::doPlayStream(const PacketHead&, const std::vector<char>& payload)
{
    const QString streamAddr = payloadToString(payload);
    if (startStreamCb_(streamAddr)) {
        state_ = State::Puller;
    }
}

void SigConnection::doCreateStream(const PacketHead&, const std::vector<char>& payload)
{
    const QString streamAddr = payloadToString(payload);
    if (startStreamCb_(streamAddr)) {
        state_ = State::Pusher;
    }
}

void SigConnection::doDeleteStream(const PacketHead&, const std::vector<char>&)
{
    if (stopStreamCb_) {
        stopStreamCb_();
    }
    state_ = State::Idle;
}

void SigConnection::doMouseEvent(const PacketHead&, const std::vector<char>&)
{
    // 保留原有事件入口，具体按钮映射建议等协议字段确定后再落地。
}

void SigConnection::doMouseMoveEvent(const PacketHead&, const std::vector<char>& payload)
{
    if (payload.size() < sizeof(qint32) * 2) {
        return;
    }

    const auto* data = reinterpret_cast<const qint32*>(payload.data());
    QCursor::setPos(data[0], data[1]);
}

void SigConnection::doKeyEvent(const PacketHead&, const std::vector<char>&)
{
    // 保留原有事件入口，具体 key/modifier 格式建议放到协议定义里统一解析。
}

void SigConnection::doWheelEvent(const PacketHead&, const std::vector<char>&)
{
    // 保留原有事件入口，具体滚轮 delta 格式建议放到协议定义里统一解析。
}

qint32 SigConnection::SendPacket(MessageType type, const QByteArray& payload)
{
    Buffer buffer(kPacketHeadSize + static_cast<quint32>(payload.size()));
    PacketWriter writer(buffer);

    writer.WriteUint16BE(static_cast<quint16>(type));
    writer.WriteUint32BE(static_cast<quint32>(payload.size()));
    writer.WriteBytes(payload.constData(), static_cast<size_t>(payload.size()));

    Send(buffer.Peek(), buffer.ReadableBytes());
    return 0;
}

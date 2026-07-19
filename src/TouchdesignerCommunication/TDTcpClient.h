#pragma once
#include "sv_qtcommon.h"
#include <QTcpSocket>
#include "DataToTDFormat/TDFormatDefs.h"
#include "Packets.h"


class TDTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TDTcpClient(QObject* parent = nullptr)
        : QObject(parent)
        , socket_(nullptr)
    {
        socket_ = new QTcpSocket(this);

        connect(socket_, &QTcpSocket::connected,        this, &TDTcpClient::onConnected);
        connect(socket_, &QTcpSocket::disconnected,     this, &TDTcpClient::onDisconnected);
        connect(socket_, &QTcpSocket::errorOccurred,    this, &TDTcpClient::onError);
    }

    void connectToTd(const QString& host = "localhost", quint16 port = 7062)
    {
        if (socket_->state() != QAbstractSocket::UnconnectedState)
        {
            disconnectFromTd();
        }

        SV_LOG(std::format("Executing connectToHost on host [{}]:[{}]", host, port));
        socket_->connectToHost(host, port);
    }

    void disconnectFromTd()
    {
        SV_LOG("Executing disconnectFromHost");
        socket_->disconnectFromHost();
    }

    void sendTreeData(const TreeAsVec4Array& treeData, const std::string& presetName)
    {
        if (auto packet = Packets::makeTreeAsVec4Packet(treeData, 0, treeData.size()-1, presetName))
        {
            sendPacket(*packet);
        }
        else SV_ERROR("sendTreeData error: couldnt create packet")
    }

    void sendPacket(const QByteArray& packet)
    {
        if (socket_->state() != QAbstractSocket::ConnectedState)
        {
            SV_ERROR("Cant sendPacket, not connected.");
            return;
        }

        SV_LOG(std::format("sendPacket: sent {} bytes", packet.size()));

        // non-blocking write, qt sends asynchronously:
        socket_->write(packet);
    }

signals:
    void connectionError(const QString& error);

private slots:
    void onConnected()
    {
        // Nothing special needed.
        SV_LOG("Connect succeeded.");
    }

    void onDisconnected()
    {
        SV_LOG("Disconnect succeeded.");
        // Optional: notify UI.
    }

    void onError(QAbstractSocket::SocketError error)
    {
        auto err = socket_->errorString();

        SV_MSGBOX_ERROR(std::format("Qt connection error: {}", err));

        emit connectionError(err);
    }

private:
    QTcpSocket* socket_ = nullptr;
};
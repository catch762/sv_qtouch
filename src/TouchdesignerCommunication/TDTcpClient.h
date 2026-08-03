#pragma once
#include "sv_qtcommon.h"
#include <QTcpSocket>
#include "DataToTDFormat/TDFormatDefs.h"
#include "Packets.h"


class TDTcpClient : public QObject
{
    Q_OBJECT

public:
    using PacketHandler = std::function<void(QByteArray contentBlock)>;

    explicit TDTcpClient(QObject* parent = nullptr)
        : QObject(parent)
        , socket(nullptr)
    {
        socket = new QTcpSocket(this);

        packetHandlers[PacketType::LoadGlslFiles] = std::bind(onLoadGlslPacketReceived, this, std::placeholders::_1);

        connect(socket, &QTcpSocket::connected,     this, &TDTcpClient::onConnected);
        connect(socket, &QTcpSocket::disconnected,  this, &TDTcpClient::onDisconnected);
        connect(socket, &QTcpSocket::errorOccurred, this, &TDTcpClient::onError);
        connect(socket, &QTcpSocket::readyRead,     this, &TDTcpClient::extractAndProcessAllAvailablePackets);
    }

    void connectToTd(const QString& host = "localhost", quint16 port = 7062)
    {
        if (socket->state() != QAbstractSocket::UnconnectedState)
        {
            disconnectFromTd();
        }

        SV_LOG(std::format("Executing connectToHost on host [{}]:[{}]", host, port));
        socket->connectToHost(host, port);
    }

    void disconnectFromTd()
    {
        SV_LOG("Executing disconnectFromHost");
        socket->disconnectFromHost();
    }

    void sendTreeData(const TreeAsVec4Array& treeData, const std::string& presetName, const TreeVarNames* optionalVarNames = nullptr)
    {
        if (auto packet = Packets::makeTreeAsVec4Packet(treeData, 0, treeData.size()-1, presetName, optionalVarNames))
        {
            SV_LOG(std::format("<sendTreeData Packet of {} bytes>\n{}\n</sendTreeData Packet>",
                packet->size(), QString::fromLatin1(packet->toHex())));

            sendPacket(*packet);
        }
        else SV_ERROR("sendTreeData error: couldnt create packet")
    }

    void sendPacket(const QByteArray& packet)
    {
        if (socket->state() != QAbstractSocket::ConnectedState)
        {
            SV_ERROR("Cant sendPacket, not connected.");
            return;
        }

        SV_LOG(std::format("sendPacket: sent {} bytes", packet.size()));

        // non-blocking write, qt sends asynchronously:
        socket->write(packet);
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
        auto err = socket->errorString();

        SV_MSGBOX_ERROR(std::format("Qt connection error: {}", err));

        emit connectionError(err);
    }

private:
    //returns true if packet was extracted (even if there was some error)
    //returns false if not enough data to extract next packet
    bool tryExtractAndProcessPacket()
    {
        if (socket->bytesAvailable() < Packets::PacketHeaderSize)
        {
            //dont even have enough data for header
            return false;
        }

        auto header = Packets::tryParseHeader(socket->peek(Packets::PacketHeaderSize));
        SV_ASSERT(header);

        if (socket->bytesAvailable() < header->packetSize)
        {
            //header parsed, but this packet didnt fully arrive yet
            return false;
        }

        //Ok, we have entire packet in memory, now lets extract it:

        socket->read(Packets::PacketHeaderSize); //extract header which we already parsed, so we discard the result

        const int contentBlockSize = header->packetSize - Packets::PacketHeaderSize;

        QByteArray contentBlockOfPacket = socket->read(contentBlockSize);
        SV_ASSERT(contentBlockOfPacket.size() == contentBlockSize);

        if (auto* handler = getValue(packetHandlers, header->packetType))
        {
            (*handler)(std::move(contentBlockOfPacket));
        }
        else
        {
            SV_ERROR(std::format("Couldnt find handler for arrived packet {}", *header));
        }

        //return true regardless of parsing error, we ate the data and thats all that matters
        return true;
    }

    void extractAndProcessAllAvailablePackets()
    {
        while (tryExtractAndProcessPacket())
        {
        }
    }

private:
    void onLoadGlslPacketReceived(QByteArray contentBlock)
    {


        //тут




        const char* next = contentBlock.constData();
    }

private:
    QTcpSocket* socket = nullptr;
    std::map<PacketType, PacketHandler> packetHandlers;
};
#pragma once
#include "sv_qtcommon.h"
#include "DataToTDFormat/TDFormatDefs.h"

enum class PacketType : uint32_t
{
    TreeData = 0
};

//Packet structure:
inline constexpr int PacketBytesOffset_PacketBytesCount = 0;   //uint32_t Field 1: size of entire packet
inline constexpr int PacketBytesOffset_PacketType       = 4;   //uint32_t Field 2: packet type
inline constexpr int PacketBytesOffset_PacketContent    = 8;   //         Field 3: actual content data of any size, even 0

//Everything before actualy content bytes.
inline constexpr int PacketHeaderSize = PacketBytesOffset_PacketContent;

inline QByteArray makeArrayForPacket(PacketType type, uint32_t contentSize)
{
    const uint32_t packetBytesCount = PacketHeaderSize + contentSize;
    const uint32_t packetType       = uint32_t(PacketType::TreeData);

    QByteArray packet(packetBytesCount, Qt::Uninitialized);

    std::memcpy(packet.data() + PacketBytesOffset_PacketBytesCount,
                &packetBytesCount,
                sizeof(packetBytesCount));

    std::memcpy(packet.data() + PacketBytesOffset_PacketType,
                &packetType,
                sizeof(packetType));

    return packet;
}


inline int packetContentSize(const QByteArray& packet)
{
    return packet.size() - PacketHeaderSize;
}

inline char* packetContentPtr(QByteArray& packet)
{
    SV_ASSERT(packetContentSize(packet) > 0);
    return packet.data() + PacketHeaderSize;
}

inline QByteArray makePacket(const TreeAsTDFormatData& treeData)
{
    const auto contentSize  = treeData.size() * sizeof(SUP_Vec4);
    QByteArray packet       = makeArrayForPacket(PacketType::TreeData, contentSize);

    SV_ASSERT(contentSize == packetContentSize(packet));

    std::memcpy(packetContentPtr(packet), treeData.data(), packetContentSize(packet));

    return packet;
}
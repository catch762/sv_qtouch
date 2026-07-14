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

// String section that will be written to 'memoryOfPacket':
// 
// uint32_t Field 1: string length, may be 0
//          Field 2: optional, actual bytes of the string
//
// Return value: pointer to next byte after data written. (may be invalid, if we are at the end)

uint32_t stringSectionSize(const std::string& str)
{
    return sizeof(uint32_t) + str.size();
}

char* writeStringSectionToPacket(char* memoryOfPacket, const std::string& str)
{
    const uint32_t stringLength = str.size();

    char* addressForStringLength    = memoryOfPacket;
    char* addressForStringSymbols   = memoryOfPacket + sizeof(stringLength);

    
    std::memcpy(addressForStringLength, &stringLength, sizeof(stringLength));

    if (stringLength > 0)
    {
        std::memcpy(addressForStringSymbols, str.c_str(), stringLength);
    }

    return memoryOfPacket + stringSectionSize(str);
};

inline QByteArrayOpt makePacket(const TreeAsVec4Array& treeData, const std::string& presetName)
{
    if (treeData.empty())
    {
        SV_ERROR("Surely you didnt mean to make packet with empty tree data");
        return {};
    }
    if (presetName.empty())
    {
        SV_ERROR("Surely you didnt mean to make packet with empty preset name");
        return {};
    }

    const auto presetNameSectionSize    = stringSectionSize(presetName);
    const auto vec4SectionSize          = treeData.size() * sizeof(SUP_Vec4);
    const auto contentSize              = presetNameSectionSize + vec4SectionSize;
    QByteArray packet                   = makeArrayForPacket(PacketType::TreeData, contentSize);
    SV_ASSERT(contentSize == packetContentSize(packet));

    char* presetNameSection = packetContentPtr(packet);
    char* vec4Section       = writeStringSectionToPacket(presetNameSection, presetName);

    std::memcpy(vec4Section, treeData.data(), vec4SectionSize);

    return packet;
}

inline QByteArrayOpt makePacket(const TreeVarNames& varNames, const std::string& presetName)
{
    if (varNames.empty())
    {
        SV_ERROR("Surely you didnt mean to make packet with empty varNames data");
        return {};
    }
    if (presetName.empty())
    {
        SV_ERROR("Surely you didnt mean to make packet with empty preset name");
        return {};
    }

    const auto presetNameSectionSize    = stringSectionSize(presetName);

    int varNamesSectionSize = 0;
    for (const auto& varName : varNames)
    {
        varNamesSectionSize += stringSectionSize(varName.toStdString());
    }

    const auto contentSize              = presetNameSectionSize + varNamesSectionSize;

    QByteArray packet                   = makeArrayForPacket(PacketType::TreeData, contentSize);
    SV_ASSERT(contentSize == packetContentSize(packet));

    char* presetNameSection     = packetContentPtr(packet);
    char* nextVarNameAddress    = writeStringSectionToPacket(presetNameSection, presetName);

    for (const auto& varName : varNames)
    {
        nextVarNameAddress = writeStringSectionToPacket(nextVarNameAddress, varName.toStdString());
    }

    //this now must point to next byte after packet data
    SV_ASSERT(nextVarNameAddress == packet.data() + packet.size());

    return packet;
}
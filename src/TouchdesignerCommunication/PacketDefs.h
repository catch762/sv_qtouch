#pragma once
#include "sv_qtcommon.h"
#include "DataToTDFormat/TDFormatDefs.h"
#include "QTouchDefs.h"

enum class PacketType : uint32_t
{
    TreeData = 0
};

//General packet structure that all packets follow:
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

inline uint32_t stringSectionSize(const std::string& str)
{
    return sizeof(uint32_t) + str.size();
}

inline char* writeStringSectionToPacket(char* memoryOfPacket, const std::string& str)
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

// Return value: pointer to next byte after data written. (may be invalid, if we are at the end)
template<typename T>
char* writeFixedVar(char* memory, T var)
{
    std::memcpy(memory, &var, sizeof(var));
    return memory + sizeof(var);
}

inline QByteArrayOpt makePacket(const TreeAsVec4Array& treeData, uint32_t firstIndex, uint32_t lastIndex, const std::string& presetName)
{
    if (treeData.empty())
    {
        SV_ERROR("makePacket error: Surely you didnt mean to make packet with empty tree data");
        return {};
    }
    if (presetName.empty())
    {
        SV_ERROR("makePacket error: Surely you didnt mean to make packet with empty preset name");
        return {};
    }

    const uint32_t totalEntriesInTreeCount = treeData.size();
    const int entriesToSendCount = lastIndex - firstIndex + 1;

    if (entriesToSendCount <= 0                             ||
        !isValidIndex(firstIndex, totalEntriesInTreeCount)  ||
        !isValidIndex(lastIndex,  totalEntriesInTreeCount) )
    {
        SV_ERROR(std::format("makePacket error: wrong indices first=[{}] last=[{}] for size=[{}]",
                                firstIndex, lastIndex, totalEntriesInTreeCount));
        return {};
    }

    const auto presetNameSectionSize    = stringSectionSize(presetName);
    const auto otherFixedFields         = sizeof(uint32_t) * 3;
    const auto vec4SectionSize          = entriesToSendCount * sizeof(SUP_Vec4);
    const auto contentSize              = presetNameSectionSize + otherFixedFields + vec4SectionSize;

    QByteArray packet                   = makeArrayForPacket(PacketType::TreeData, contentSize);
    SV_ASSERT(contentSize == packetContentSize(packet));

    char* next = packetContentPtr(packet);

    next = writeStringSectionToPacket(next, presetName);
    next = writeFixedVar(next, totalEntriesInTreeCount);
    next = writeFixedVar(next, firstIndex);
    next = writeFixedVar(next, lastIndex);
    std::memcpy(next, treeData.data() + firstIndex, vec4SectionSize);

    return packet;
}

inline QByteArrayOpt makePacket(const TreeVarNames& varNames, const std::string& presetName)
{
    if (varNames.empty())
    {
        SV_ERROR("makePacket error: Surely you didnt mean to make packet with empty varNames data");
        return {};
    }
    if (presetName.empty())
    {
        SV_ERROR("makePacket error: Surely you didnt mean to make packet with empty preset name");
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


// Just to avoid extra copies, we will simply return multiple bytearrays,
// which we then feed into tcp stream sequentially,
using PresetsExportPackets = std::vector<QByteArray>;
SV_DECL_OPT(PresetsExportPackets);

inline PresetsExportPacketsOpt makePresetExportPackets();
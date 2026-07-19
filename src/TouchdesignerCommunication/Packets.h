#pragma once
#include "sv_qtcommon.h"
#include "DataToTDFormat/TDFormatDefs.h"
#include "QTouchDefs.h"


//********************************************************************************
// 
// Every packet follows this structure:
//
//      Header section:
//          Field 0:    uint32_t    Size of entire packet (including this field)
//          Field 1:    uint32_t    PacketType enum value
//      Optional content section:
//          <any number of bytes, even 0. decrypting is based on PacketType field>
//
// As you can see, this means that smallest valid packet can have size of 8 bytes.
//
//********************************************************************************

enum class PacketType : uint32_t
{
    TreeData = 0,
    VarNames,
    PresetsExport
};

class Packets
{
public:
    static QByteArrayOpt makeTreeAsVec4Packet(const TreeAsVec4Array& treeData, uint32_t firstIndex, uint32_t lastIndex, const std::string& presetName)
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

        const uint32_t  totalEntriesInTreeCount = treeData.size();
        const int       entriesToSendCount      = lastIndex - firstIndex + 1;

        if (entriesToSendCount <= 0 ||
            !isValidIndex(firstIndex, totalEntriesInTreeCount) ||
            !isValidIndex(lastIndex, totalEntriesInTreeCount))
        {
            SV_ERROR(std::format("makePacket error: wrong indices first=[{}] last=[{}] for size=[{}]",
                                 firstIndex, lastIndex, totalEntriesInTreeCount));
            return {};
        }

        const auto vec4SectionSize          = entriesToSendCount * sizeof(SUP_Vec4);
        const auto contentSize              = stringSectionSize(presetName) + sizeof(uint32_t) * 3 + vec4SectionSize;

        QByteArray packet = makeArrayForPacket(PacketType::TreeData, contentSize);
        SV_ASSERT(contentSize == packetContentSize(packet));

        char* next = packetContentPtr(packet);

        //***************************
        //  HERE GO ACTUAL FIELDS:
        //***************************
        next = writeStringSectionToPacket   (next, presetName);
        next = writeFixedVar                (next, totalEntriesInTreeCount);
        next = writeFixedVar                (next, firstIndex);
        next = writeFixedVar                (next, lastIndex);
        next = writeBytes                   (next, treeData.data() + firstIndex, vec4SectionSize);

        return packet;
    }

    static QByteArrayOpt makeTreeVarnamesPacket(const TreeVarNames& varNames, const std::string& presetName)
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

        int varNamesSectionSize = 0;
        for (const auto& varName : varNames)
        {
            varNamesSectionSize += stringSectionSize(varName.toStdString());
        }

        const auto contentSize = stringSectionSize(presetName) + varNamesSectionSize;

        QByteArray packet = makeArrayForPacket(PacketType::VarNames, contentSize);
        SV_ASSERT(contentSize == packetContentSize(packet));

        char* next = packetContentPtr(packet);

        //***************************
        //  HERE GO ACTUAL FIELDS:
        //***************************
        next = writeStringSectionToPacket(next, presetName);
        for (const auto& varName : varNames)
        {
            next = writeStringSectionToPacket(next, varName.toStdString());
        }

        SV_ASSERT(pointerIsAtTheEnd(next, packet));

        return packet;
    }

    // Note: sending empty arguments should form perfectly valid packet meant for resetting exports on TD side.
    static QByteArrayOpt makePresetExportsPacket(const QByteArray& varNamesPacket, const std::vector<QByteArray>& vec4Packets)
    {
        // Instead of packing all packets into single QByteArray, we could just feed them one by one
        // to TCP stream and it would work, but i dont care, its a rare operation.

        int vec4PacketsSize = 0;
        for (auto& packet : vec4Packets)
        {
            vec4PacketsSize += packet.size();
        }

        const auto contentSize = varNamesPacket.size() + vec4PacketsSize;

        QByteArray packet = makeArrayForPacket(PacketType::PresetsExport, contentSize);

        char* next = packetContentPtr(packet);

        //***************************
        //  HERE GO ACTUAL FIELDS:
        //***************************
        next = writeBytes(next, varNamesPacket.data(), varNamesPacket.size());
        for (const auto& subPacket : vec4Packets)
        {
            next = writeBytes(next, subPacket.data(), subPacket.size());
        }

        SV_ASSERT(pointerIsAtTheEnd(next, packet));
        return packet;
    }

private:
    // When we are done building packet, we check that 'next' pointer now 
    // points at next byte after packet end -- this means we did everything correctly
    static bool pointerIsAtTheEnd(const char* nextPointer, const QByteArray& packet)
    {
        return nextPointer == packet.data() + packet.size();
    }

    // Returns packet which has header completely initialized,
    // and content memory allocated, but uninitialized
    static QByteArray makeArrayForPacket(PacketType type, uint32_t contentSize)
    {
        const uint32_t packetBytesCount = PacketHeaderSize + contentSize;
        const uint32_t packetType       = uint32_t(PacketType::TreeData);

        QByteArray packet(packetBytesCount, Qt::Uninitialized);

        char* next = packet.data();

        next = writeFixedVar(next, packetBytesCount);
        next = writeFixedVar(next, packetType);

        return packet;
    }

    // Doesnt do any checks, may return negative values for unexpected input
    static int packetContentSize(const QByteArray& packet)
    {
        return packet.size() - PacketHeaderSize;
    }

    // Returns pointer to first byte of content section.
    // Unexpected input will trigger an assert
    static char* packetContentPtr(QByteArray& packet)
    {
        SV_ASSERT(packetContentSize(packet) > 0);
        return packet.data() + PacketHeaderSize;
    }

// Functions dedicated to writing specific data types to packet memory.
// [!]  Return value for writer functions is pointer to next byte after data written.
//      This pointer may be invalid, if we were already at the end of QByteArray
private:
    template<typename T>
    static char* writeFixedVar(char* dst, T var)
    {
        std::memcpy(dst, &var, sizeof(var));
        return dst + sizeof(var);
    }

    static char* writeBytes(char* dst, const void* source, int bytesCount)
    {
        SV_ASSERT(bytesCount >= 0);

        if (bytesCount > 0)
        {
            std::memcpy(dst, source, bytesCount);
        }
        return dst + bytesCount;
    }

    // To write a string, we write a following string section:
    // 
    //      Field 0:    uint32_t        string length, may be 0
    //      Field 1:    bytes[0; N]     optional, actual bytes of the string
    static uint32_t stringSectionSize(const std::string& str)
    {
        return sizeof(uint32_t) + str.size();
    }
    static char* writeStringSectionToPacket(char* dst, const std::string& str)
    {
        const uint32_t stringLength = str.size();

        dst = writeFixedVar(dst, stringLength);
        dst = writeBytes(dst, str.c_str(), stringLength);

        return dst;
    };

private:
    static constexpr int PacketHeaderSize = sizeof(uint32_t) * 2;
};















// Just to avoid extra copies, we will simply return multiple bytearrays,
// which we then feed into tcp stream sequentially,
using PresetsExportPackets = std::vector<QByteArray>;
SV_DECL_OPT(PresetsExportPackets);

inline PresetsExportPacketsOpt makePresetExportPackets();
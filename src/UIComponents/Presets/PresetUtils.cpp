#include "PresetUtils.h"
#include "DataToTDFormat\TDFormatTreeConverter.h"
#include "TouchdesignerCommunication/Packets.h"

QString Presets::getPresetJsonFileName(const PresetNameString& presetName)
{
	return presetName + ".json";
}

QString Presets::getPresetVec4FileName(const PresetNameString& presetName)
{
	return presetName + ".vec4_packet";
}

QString Presets::getPresetVarnamesFileName(const PresetNameString& presetName)
{
	return presetName + ".varnames_section";
}

inline StringErrOpt Presets::savePreset(const QDir& presetDir, const PresetNameString& presetName, const DataNodeShared& rootNode)
{
    auto wrapErr = [&](const std::string& err)
    {
        return std::format("saving preset [{}] to [{}] failed: {}", presetName, presetDir.absolutePath(), err);
    };

    if (!rootNode)
    {
        return wrapErr("rootNode is empty");
    }

    //if it existed, we already got confirmation to delete old files

    // 1) .json
    {
        auto json = SerializerForDataNodeTreeAndItsWidgets::toJson(rootNode);
        if (!json)
        {
            return wrapErr("serialization to json has failed");
        }

        auto filePath = presetDir.absoluteFilePath(getPresetJsonFileName(presetName));

        bool saved = saveJsonValueToFile(*json, filePath);
        if (!saved)
        {
            return wrapErr("writing to json file has failed");
        }
    }

    // 2) vec4 data file
    {
        TreeAsVec4Array treeAsVec4;
        if (auto err = convertTreeToVec4Array(rootNode, treeAsVec4))
        {
            return wrapErr(std::format("convertTreeToVec4Array failed with {}", *err));
        }

        auto packet = Packets::makeTreeAsVec4Packet(treeAsVec4, 0, treeAsVec4.size() - 1, presetName.toStdString());
        if (!packet)
        {
            return wrapErr("couldnt makePacket from TreeAsVec4Array");
        }

        auto filePath = presetDir.absoluteFilePath(getPresetVec4FileName(presetName));

        bool saved = writeByteArrayToFile(filePath, *packet);
        if (!saved)
        {
            return wrapErr("writing to vec4 file has failed");
        }
    }

    // 3) var names data file
    {
        TreeVarNames treeVarNames;
        if (auto err = getVarNamesFromTree(rootNode, treeVarNames))
        {
            return wrapErr(std::format("getVarNamesFromTree failed with {}", *err));
        }

        auto section = Packets::makeTreeVarnamesSection(treeVarNames);
        if (!section)
        {
            return wrapErr("couldnt make section from TreeVarNames");
        }

        auto filePath = presetDir.absoluteFilePath(getPresetVarnamesFileName(presetName));

        bool saved = writeByteArrayToFile(filePath, *section);
        if (!saved)
        {
            return wrapErr("writing to varnames file has failed");
        }
    }

    SV_LOG(std::format("Successfully saved preset {}", presetName));

    return {};
}

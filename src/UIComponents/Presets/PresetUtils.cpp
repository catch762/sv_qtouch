#include "PresetUtils.h"
#include "DataToTDFormat\TDFormatTreeConverter.h"
#include "TouchdesignerCommunication/Packets.h"

QString Presets::getPresetJsonFileName(const PresetNameString& presetName)
{
	return presetName + ".json";
}

QString Presets::getPresetJsonFilePath(const PresetNameString& presetName, const QDir& presetDir)
{
    return presetDir.absoluteFilePath(getPresetJsonFileName(presetName));
}

QString Presets::getPresetVec4FileName(const PresetNameString& presetName)
{
	return presetName + ".vec4_packet";
}

QString Presets::getPresetVec4FilePath(const PresetNameString& presetName, const QDir& presetDir)
{
    return presetDir.absoluteFilePath(getPresetVec4FileName(presetName));
}

QString Presets::getPresetVarnamesFileName(const PresetNameString& presetName)
{
	return presetName + ".varnames_section";
}

QString Presets::getPresetVarnamesFilePath(const PresetNameString& presetName, const QDir& presetDir)
{
    return presetDir.absoluteFilePath(getPresetVarnamesFileName(presetName));
}

StringErrOpt Presets::savePreset(const QDir& presetDir, const PresetNameString& presetName, const DataNodeShared& rootNode)
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

        bool saved = saveJsonValueToFile(*json, getPresetJsonFilePath(presetName, presetDir));
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

        bool saved = writeByteArrayToFile(getPresetVec4FilePath(presetName, presetDir), *packet);
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

        bool saved = writeByteArrayToFile(getPresetVarnamesFilePath(presetName, presetDir), *section);
        if (!saved)
        {
            return wrapErr("writing to varnames file has failed");
        }
    }

    return {};
}

std::vector<PresetNameString> Presets::getAllPresetNames(const QDir& presetDir)
{
    std::vector<PresetNameString> presetNames;

    QDirIterator it(
        presetDir.absolutePath(),
        QStringList() << "*.json",
        QDir::Files | QDir::Readable,
        QDirIterator::NoIteratorFlags
    );

    while (it.hasNext())
    {
        it.next();

        PresetNameString name = getFileNameWithoutLastExtension(it.filePath());

        presetNames.push_back(name);
    }

    return presetNames;
}

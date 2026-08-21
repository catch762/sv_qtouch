#pragma once
#include "sv_qtcommon.h"

#include "DataNode/DataNodeHeader.h"

//We often send tree data along with its preset name.
//Here's what we pass for it, when we are sending live data from current app state, not from preset
inline constexpr auto QTouchUITreePresetName = "__ui_tree__";

//Filename without extension. Relates to preset files within "<project folder>/presets/..."
using PresetNameString = QString;

class Presets
{
public:
	// Saved preset data is 3 files:

	// 1. Actual preset state in json - this is file containing serialized DataNode tree
	static QString getPresetJsonFileName	(const PresetNameString& presetName);
	static QString getPresetJsonFilePath	(const PresetNameString& presetName, const QDir& presetDir);

	// However, we sometimes need to take a list of presets and export them to Touchdesigner
	// (via TCP, currently). So it would be expensive to load each preset tree, then produce
	// data in network format... So when we save preset, we save this binary data too, and
	// then we can just dump it to TCP stream:

	// 2. File with binary packet containing preset tree state as, essentially, float array of variables
	static QString getPresetVec4FileName	(const PresetNameString& presetName);
	static QString getPresetVec4FilePath	(const PresetNameString& presetName, const QDir& presetDir);

	// 3. File with binary section of a packet, containing variable names for entries of array mentioned above
	static QString getPresetVarnamesFileName(const PresetNameString& presetName);
	static QString getPresetVarnamesFilePath(const PresetNameString& presetName, const QDir& presetDir);


	static StringErrOpt savePreset(const QDir& presetDir, const PresetNameString& presetName, const DataNodeShared& tree);

	static std::vector<PresetNameString> getAllPresetNames(const QDir& presetDir);
};
#pragma once
#include "sv_qtcommon.h"

inline constexpr auto QtouchRootName = "root";

//We often send tree data along with its preset name.
//Here's what we pass for it, when we are sending live data from current app state, not from preset
inline constexpr auto QTouchUITreePresetName = "__ui_tree__";

//Filename without extension. Relates to preset files within "<project folder>/presets/..."
using PresetNameString = QString;

inline QString getPresetJsonFileName(const PresetNameString& presetName)
{
	return presetName + ".json";
}
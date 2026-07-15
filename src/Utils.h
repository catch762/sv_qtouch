#pragma once
#include "sv_qtcommon.h"

inline constexpr auto QtouchRootName = "root";

//We often send tree data along with its preset name.
//Here's what we pass for it, when we are sending live data from current app state, not from preset
inline constexpr auto QTouchUITreePresetName = "__ui_tree__";


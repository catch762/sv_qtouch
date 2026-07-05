#pragma once
#include "sv_common.h"
#include "TDFormatDefs.h"



template<typename T>
class TDFormatConverter
{
public:
	static SUP_Vec4Opt convert(const T& val);
};
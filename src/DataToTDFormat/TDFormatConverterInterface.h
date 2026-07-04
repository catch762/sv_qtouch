#pragma once
#include <array>

//***********************************************************************************
//
// So, we have a DataNode tree, containing our C++ types data.
//
// But on receiving end we have this: <snippet from sup.glsl>:
// 
//		//everything goes through this
//		uniform vec4 sup_data[];
//
//		float 	sup_as_float(vec4 v) { return v.x; }
//		vec2 	sup_as_vec2(vec4 v) { return v.xy; }
//		vec3 	sup_as_vec3(vec4 v) { return v.xyz; }
//		vec4 	sup_as_vec4(vec4 v) { return v; }
//		int 	sup_as_int(vec4 v) { return int(v.x + 0.5); }
//		ivec2 	sup_as_ivec2(vec4 v) { return ivec2(v.xy + vec2(0.5)); }
//		ivec3 	sup_as_ivec3(vec4 v) { return ivec3(v.xyz + vec3(0.5)); }
//		ivec4 	sup_as_ivec4(vec4 v) { return ivec4(v + vec4(0.5)); }
//
// So, we have to convert every variable to vec4 (even if its smaller)
// and then turn our tree into array of vec4. And then we send this to Touchdesigner.
//
// vec4 is just four floats.
//
//***********************************************************************************

using SUP_Vec4 = std::array<float, 4>;
SV_DECL_OPT(SUP_Vec4);

template<typename T>
class TDFormatConverter
{
public:
	static SUP_Vec4Opt convert(const T& val);
};
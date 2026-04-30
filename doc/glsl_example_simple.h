// The most simple example of data we expect to find in GLSL code conforming to SUP lib.
#define STRDECL(x)
#define STR(x)
#define VAR(x)
#define ooo(x) 1
#define ui(x)
#define SUP_VARS_BEGIN
#define SUP_VARS_END


#define STRMETA_Hello(V,N) STRMAIN_ARG2(Hello,V,N,\
float, 	a,		ui("lims = [0, 1, 0.5]")\
vec4,	b)		ui("")
STRDECL(Hello)

SUP_VARS_BEGIN
		VAR(float, a)	ui("")
		VAR(vec3, b)	ui("")
		VAR(int, c)		ui("")
		VAR(vec4, d)	ui("[10, 30, 20]")
SUP_VARS_END


SUP_VARS_BEGIN
		STR(Hello, hello) 	ui("")
#line	ooo(Hello)
		VAR(vec4, theVar)	ui("")
SUP_VARS_END
// The most simple example of data we expect to find in GLSL code conforming to SUP lib.
#define STRDECL(x)
#define STR(x)
#define VAR(x)
#define ooo(x) 1
#define ui(x)
#define SUP_VARS_BEGIN
#define SUP_VARS_END

/*
SUP_ADDTODICT_BEGIN
cr 				= 10,
camdata 		= [cr, 20, 15],
Ny 				= 'no yes'
SUP_ADDTODICT_END
*/


#define STRMETA_Hello(V,N) STRMAIN_ARG4(Hello,V,N,\
int, 	a,		ui("camdata")\
ivec4,	xxx,	ui("rad = [ ['a','d','ddd'], ['x','xx','ss','dd',>,'ff','eeeeeee'] ]")\ 
int,	b,		ui("rad = ['hey', >,'bro', 'a', 'bbbbbbbbbbbbbbbbbbbb', 'c','ddd','e']")\
vec4,	c)		ui("[[0, 5, 2], camdata]")
STRDECL(Hello)


SUP_VARS_BEGIN
		STR(Hello, hello) 	ui("")
#line	ooo(Hello)
		VAR(vec4, theVar)	ui("")
SUP_VARS_END


SUP_VARS_BEGIN
		VAR(float, a)	ui("")
		VAR(vec3, b)	ui("")
		VAR(int, c)		ui("")
		VAR(vec4, d)	ui("[10, 30, 20]")
SUP_VARS_END
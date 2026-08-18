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
cr 				= 10
camdata 		= [-cr, cr, 777777]
Ny 				= 'no yes'
SUP_ADDTODICT_END
*/

//a: different creation string due to camdata
//b -> bbb: renamed
//c: different creation string
//XXX: new added var
//d: same

//theVar: diff type

#define STRMETA_Hello(V,N) STRMAIN_ARG5(Hello,V,N,\
ivec2, 	a,		ui("lims=camdata")\
ivec4,	bbb,	ui("rad = [ ['a','d','ddd'], ['x','xx','ss','dd',>,'ff','eeeeeee'] ]")\ 
int,	c,		ui("[1, 2, 1]")\
int,	XXX,	ui("rad = ['hey', >,'bro', 'a', 'bbbbbbbbbbbbbbbbbbbb', 'c','ddd','e']")\
vec4,	d)		ui("[0, 5, 2]")
STRDECL(Hello)


SUP_VARS_BEGIN
		STR(Hello, hello) 	ui("")
#line	ooo(Hello)
		VAR(vec3, theVar)	ui("tab = 3")
SUP_VARS_END
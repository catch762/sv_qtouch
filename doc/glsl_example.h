/*
SUP_PYTHON_BEGIN
################
cr 				= 20
camdata 		= [[0,2,1],[-cr,cr,0],[-cr,cr,0], [0.0001, 4.5, 1.5]]
camrad			= [ [-1,1,0], [-0.5,0.5,0.5], [-1,1,0], [-1,1,0] ]
ls 				= 4
lookat = [[-ls,ls,0],[-ls,ls,0],[-ls,ls,0]]

pd = 1
snm_pos = [[-pd,pd,0],[-pd,pd,0],[-pd,pd,0]]

sceneid = [0, 1, 0]
sceneid_wtest = [-1, 10, 0]

snm_spacing 	= [1, 8, 1]
snm_spacings 	= [snm_spacing, snm_spacing, snm_spacing]
snm_repidx 		= [0, 16, 1]
snm_repidxs 	= [snm_repidx, snm_repidx, snm_repidx]

s01 = [0, 1, 1]
s010 = [0, 1, 0]
s01h = [0, 1, 0.5]
s11 = [-1, 1, 0]
fbm4 = [ [0.01, 4, 0.05], [0.5, 1, 0.5], [0, 1, 0], [0, 2, 0] ]
Ny = 'no yes'

################
SUP_PYTHON_END
*/




SUP_STRUCTS_BEGIN
//---------------

#define STRMETA_Rmain(V,N) STRMAIN_ARG11(Rmain,V,N,\
vec4, 	camera,			ui("lims=camdata"			)\
vec4, 	camera_rad,		ui("tox = 'sv_xys', lims = [-1,1,0]"			)\
vec3,	lookat,			ui("lims=lookat"			)\
float,	normalmix,		ui("lims = [0, 1, 0.3]"		)\
ivec3,	RootidIqnelidIqneloct,	ui("lims = [sceneid_wtest, [-100,100,0], [0,9,0]]"	)\
vec3,	testdims,		ui("lims = [0.1,2,1]"		)\
int,	depth,			ui("lims = [0, 6, 0]"		)\
float,	colorseed,		ui("lims = [0, 8, 2]"		)\
float,	phaselen,		ui("lims = [0, 16, 4]"		)\
float,	manphase,		ui("lims = [0, 2, 0]"		)\
ivec3,	renmode)		ui("rad=['base >+wire', 'manpos >radial', '>manphase timephase']")
STRDECL(Rmain)

#define STRMETA_RenFin(V,N) STRMAIN_ARG3(RenFin,V,N,\
vec4, 	stepcNvalc,		ui("tox = 'sv_xys', lims = [[0,1,1],[0,1,0]]"	)\
vec2,	step_powmul, 	ui("lims=[[0.01,3,1],[0.01,33,10]]"				)\
vec2,	val_powmul) 	ui("lims=[[0.01,3,1],[0.01,33,10]]"				)
STRDECL(RenFin)

#define STRMETA_SNChild(V,N) STRMAIN_ARG7(SNChild,V,N,\
vec3, 	pos,			ui("lims=snm_pos"		)\
float, 	scale,			ui("lims=[0.1, 1, 0.5]"	)\
float, 	rot,			ui("lims=[0, 1, 0]"		)\
int, 	id,				ui("lims=sceneid"		)\
vec3, 	repspacing,		ui("lims=snm_spacings"	)\
ivec3, 	repidxfrom,		ui("lims=[-8,8,0]"		)\
ivec3, 	repidxto)		ui("lims=snm_repidxs"	)
STRDECL(SNChild)

#define STRMETA_IQNData(V,N) STRMAIN_ARG15(IQNData,V,N,\
int, 	octaves,									ui("lims=[0, 12, 0]"				)\
vec4, 	scale2_smooth2,								ui("tox = 'sv_xys', lims=fbm4"		)\
vec2, 	NeighbOctmod,								ui("lims=[[0,2,0.1], [0,2,1]]"		)\
ivec3, 	mode_elemId_octopId,						ui("rad=['sub add', 'sp wb wbco A B C D', 'no iqrot shrot 1 2 3']" )\
vec4, 	elem_prm_Size,								ui("lims=[0,1,1]"					)\
vec4, 	elem_prm_Rot,								ui("lims=[0,1,0]"					)\
vec4, 	elem_prm_Trans,								ui("lims=s11"						)\
vec4, 	elem_prm_Custom,							ui("lims=s11"						)\
vec4, 	elem_prm_Extra,								ui("lims=s11"						)\
vec4, 	elem_prm_Caleidrot,							ui("lims=s11"						)\
vec4, 	elem_prm_Caleidtrans,						ui("lims=s11"						)\
vec4, 	elem_prm_RoundClipformClipsizeNoisetodepth,	ui("lims=[[-0.15, 0.15, 0], s010, [0,1,1], s010]")\
vec4, 	elem_prm_Pads,								ui("tox = 'sv_xys', lims = s11"		)\
vec4, 	elem_prm_PadsDelta,							ui("tox = 'sv_xys', lims = s11"		)\
vec4, 	prm_octop)									ui("lims=[0,1,0]")
STRDECL(IQNData)

#define STRMETA_Lighting(V,N) STRMAIN_ARG5(Lighting,V,N,\
vec4, 	fogcolor,			ui("color = [1,1,1,0.09]"	)\
float, 	fogdistfactor,		ui("lims=[0, 12, 1]"	)\
vec3, 	AmblightShadfromTo,	ui("lims=[[0, 1, 0.4], s010, s01]"	)\
float, 	using_mat,			ui("lims=[0, 1, 1]"		)\
float, 	using_lsource)		ui("lims=[0, 1, 1]"		)
STRDECL(Lighting)

#define STRMETA_Light(V,N) STRMAIN_ARG4(Light,V,N,\
vec4, 	pos,			ui("lims=s11"				)\
float, 	radius,			ui("lims=[0.5, 2, 2]"	)\
vec4, 	color,			ui("color = [1,1,1,1]"			)\
float, 	intensity01)	ui("lims=[0, 25, 5]"				)
STRDECL(Light)

#define STRMETA_Ring(V,N) STRMAIN_ARG9(Ring,V,N,\
vec4, 	FromToHeightSpacing,	ui("lims=[[0.15,0.5,0.15], [0.15,0.5,0.5], [0,1,0.1], [1,15,1]]")\
ivec2, 	SubdivsBilayers,		ui("lims=[[1, 32, 7], [0,4,1]]"									)\
vec4, 	elem_Size,				ui("lims = [0,1,1]"												)\
vec4, 	childPosScaleAngCross,	ui("lims = [[-1,1,1],[0,1,1],[0,1,1],[0,1,1]]"					)\
vec2, 	childSegcountPlatfh,	ui("lims = [[1,8,5],[0,0.5,0]]"									)\
vec4, 	aa,						ui("lims = [0,1,1]"												)\
vec4, 	bb,						ui("lims = [0,1,1]"												)\
vec4, 	cc,						ui("lims = [0,1,1]"												)\
vec4, 	dd)						ui("lims = [0,1,1]"												)
STRDECL(Ring)

#define STRMETA_RingParDeltas(V,N) STRMAIN_ARG8(RingParDeltas,V,N,\
vec4, 	d_Size,					ui("lims = [0,1,1]"								)\
vec4, 	d_PosScaleAngCross,		ui("lims = [[-1,1,1],[0,1,1],[0,1,1],[0,1,1]]"	)\
vec2, 	d_SegcountPlatfh,		ui("lims = [[1,8,5],[0,0.5,0]]"					)\
vec3, 	shuffle_SizePosSeg,		ui("lims = [0,1,1]"								)\
vec4, 	globshuffle,			ui("tox = 'sv_xys', lims = [[-1,1,1],[-1,1,1]]"	)\
vec4, 	MidtexWinsize,			ui("tox = 'sv_xys', lims = [[-1,1,1],[-1,1,1],[0.1,8,1],[0.1,8,1]]"	)\
vec4, 	ThreshNoneYsubdivMix,	ui("lims = [[0,1,1], [0,1,0.05], [2, 128, 16], [0,1,1]]"		)\
vec4, 	aaa)					ui("lims = [0,1,1]"								)
STRDECL(RingParDeltas)

//b: dist threshold, power
#define STRMETA_Vor(V,N) STRMAIN_ARG10(Vor,V,N,\
vec4, 	cellsize,				ui("lims = [0, 2, 1]"	)\
float, 	obj_drift01,			ui("lims = [0, 1, 0]"	)\
vec2, 	vorweight_total_main,	ui("lims = [0, 4, 0]"	)\
float, 	y_fogfactor,			ui("lims = [0, 16, 12]"		)\
float, 	phasepow,				ui("lims = [0.2, 4, 1]"		)\
ivec4, 	i,						ui("lims = [0, 10, 1]"		)\
vec4, 	a,						ui(""						)\
vec4, 	b,						ui(""						)\
vec4, 	c,						ui(""						)\
vec4, 	d)						ui("tox = 'sv_xys', lims = [[0,6,1],[0,6,1]]")
STRDECL(Vor)

#define STRMETA_Repwall(V,N) STRMAIN_ARG7(Repwall,V,N,\
vec4, 	dims,					ui("lims=[[0, 0.5, 0.125],[0, 0.5, 0.125],[0, 0.5, 0.125], [0, 2, 1]]")	\
vec4, 	A_TilexyDepthId,		ui("lims=[ [0,32,8], [0,48,12], [0.001, 0.5, 0.125], [0,8,0]]")			\
vec4, 	A_TexposWinsize,		ui("tox = 'sv_xys', lims=[s11, s11, [0,1,0.25], [0,1,0.25]]")			\
vec4, 	B_TilexyDepthId,		ui("lims=[ [0,32,8], [0,48,12], [0.001, 0.5, 0.125], [0,8,0]]")			\
vec4, 	B_TexposWinsize,		ui("tox = 'sv_xys', lims=[s11, s11, [0,1,0.25], [0,1,0.25]]")			\
vec4, 	AB_NoopvalTolerance,	ui("tox = 'sv_xys', lims=[s01]")										\
vec4, 	AB_Mirror)				ui("lims=[s01]")	
STRDECL(Repwall)

#define STRMETA_ProtoData(V,N) STRMAIN_ARG6(ProtoData,V,N,\
vec4, tower_WidthHeight_MinMax,					ui("lims=[[0, 1, 0.125],[0, 1, 0.125],s01h, s01h]")	\
vec4, segwall_WidthHeightLengthAngmod_Min,		ui("lims=[[0, 1, 0.05],[0, 1, 0.25],s01h, s01h]")	\
vec4, segwall_WidthHeightLengthAngmod_Max,		ui("lims=[[0, 1, 0.1],[0, 1, 0.5], s01, s01]")		\
vec4, Ylevel_SmK_Clipshape_Clipmore,			ui("lims=[[0, 1, 0.0],[0, 1, 0.0], s010, s01]")		\
vec4, Firsttowerx_Subdivs_XX,					ui("lims=[s010, [0,12,2], s010, s010 ]")			\
vec4, pads)										ui("tox = 'sv_xys', lims=[s11]")
STRDECL(ProtoData)


#define STRMETA_BPM(V,N) STRMAIN_ARG3(BPM,V,N,\
vec4, 	basecolor,				ui("color = [1,1,1,1]")	\
vec4, 	speccolor,				ui("color = [1,1,1,1]")			\
vec2, 	RoughnessRefraction)	ui("lims=[0, 2, 1]")	
STRDECL(BPM)


//-------------
SUP_STRUCTS_END


//absolutely no empty lines allowed or will break ^^
#line 	0
SUP_VARS_BEGIN
		STR(Rmain, 	rmain) 		ui("C1")
#line	ooo(Rmain)
		VAR(vec4, ta)			ui("lims = [0,2,1]")
		VAR(vec4, tb)			ui("lims = [0,2,1]")
		VAR(vec4, tc)			ui("lims = [0,2,1]")
		VAR(vec4, td)			ui("lims = [0,2,1]")
		VAR(vec4, te)			ui("tox = 'sv_xys', lims = [-1,1,0]")
		STR(RenFin, renfin) 	ui("C1")
#line	ooo(RenFin)		
		STR(Lighting, lighting)
#line	ooo(Lighting)	
		STR(Light, mainlight)
#line	ooo(Light)				
		STR(SNChild, childA) 	ui("C1")
#line	ooo(SNChild)
		STR(SNChild, childB) 	ui("C1")
#line	ooo(SNChild)
		STR(IQNData, iqn) 		ui("C2")
#line	ooo(IQNData)
		STR(IQNData, iqn2) 		ui("C2")
#line	ooo(IQNData)
		STR(Vor, vor) 			ui("C3")
#line	ooo(Vor)
		STR(Ring, ring1) 		ui("C3")
#line	ooo(Ring)
		STR(RingParDeltas, ring1deltas) 	ui("C3")
#line	ooo(RingParDeltas)
		STR(Repwall, repw1) 	ui("C1")
#line	ooo(Repwall)
		STR(BPM, bpm1) 	ui("C1")
#line	ooo(BPM)
		STR(BPM, bpm2) 	ui("C1")
#line	ooo(BPM)
		STR(ProtoData, proto) 	ui("C3")
#line	ooo(ProtoData)
SUP_VARS_END
#line 197
// <--- put THIS LINE NUMBER in #line above, for correct err msgs

//------------------------
//******** NEST **********
//------------------------

//Constant defines:

#define RENMODE_BASE 0
#define RENMODE_WIRE 1

#define GETBIT(bit) int(1 << (bit))
#define TESTBIT(theint, bit) bool((theint) & (1 << (bit)))
#define TESTFLAG(what, flagsToTest) bool(((what) & (flagsToTest)) == (flagsToTest))

#define MAINSCALE 0.33
#define INVMAINSCALE (1.0/MAINSCALE)
#define ROOTLEVELSCALE 5.0

#define MTL_PLANTS 0 
#define MTL_COOL 1 
#define MTL_DEFAULT 2 

//Settings defines:

//5
#define MAXKIDS 1

//1 to 8
#define MAXRENDEPTH 8
//-1 to use ui value
#define CONSTANTDEPTH 6
#define ONLY_TEST_SCENE 0
//0 = off and UI does nothing, 1 = 3 samples, 2 = 9 samples.
//Negative value is same as positive but always on, ignoring UI button.
#define CND_ENABLE_MULTISAMPLING 0
#define TONEMAP_ACES_ENABLED 1
#define LIGHTING_ENABLED 1
#define LIGHTING_ENABLE_SHADOWS 1
//0 to 2
#define SHOW_IQNEL_WINDOWS 0
#define IQNEL_WINDOW_SIZE 1
#define MTL_COLLECTION MTL_COOL
#define TS_DS_ENABLED 1
#define STEP01DATA_ENABLED 0 
 
//unconstrained, like '2.5 phase cycles'
float phasetime()
{
	int mode = rmain_renmode().z;
	
	float res = rmain_manphase();
	
	if (mode == 1)
	{
		res = time / rmain_phaselen();
	}
	
	return res;
}

//whole int of current phase index
int phaseidx()
{
	return int(phasetime());
}

float phase01()
{	
	return lim_tor(0,1,phasetime());
}	

//1.0 to MAINSCALE, based on phase01
float phasescale()
{
	return mix(1.0, MAINSCALE, phase01());
}

float maxdist()
{
	return SDF_MAX_DIST * phasescale();
}

float mindist()
{
	return SDF_MIN_DIST * phasescale();
}

vec3 get_eye(int mode, float xdist)
{
	float phase01 			= phase01();
	float depth_idx_from 	= 2;
	
	// 1) horizontal part:
	
	//Dist from origin	
	float edge_From			= 0.5 * pow(MAINSCALE, depth_idx_from);
	float edge_To			= edge_From * MAINSCALE;
		
	float cur_dist 			= mix(edge_From, edge_To, phase01);
	float y_rot01 			= rmain_camera_rad().x + int(phasetime()) * 0.25;
	vec2  xzpart 			= rotate01(vec2(cur_dist,0), y_rot01);
	
	// 2) vertical part:
	
	float ui_Y 	= rmain_camera_rad().y * rmain_camera_rad().y * sign(rmain_camera_rad().y);
	
	float y_1 	= ui_Y;
	float y_2 	= ui_Y * MAINSCALE;
	float y 	= mix(y_1, y_2, phase01);
	
	// 3) Finalizing:
	
	vec3  fin 	= vec3(xzpart.x, y, xzpart.y);
	
	/*if (mode == 0)
	{
		fin.y = 0;
		fin = normalize(fin) * xdist;
		fin.y = 1 + ui_Y * 4.5;
	}*/
	
	fin /= MAINSCALE;
	
	return fin;
}

vec3 get_lookat(vec3 eye)
{
	vec3 	lookat		= vec3(0, 0, 0);
	
	vec2 inrot = rmain_camera_rad().zw;
	inrot.x *= 0.25;
	vec3 diff = normalize(lookat - eye);
	diff.xz = rotate01(diff.xz, inrot.x);
	diff.y += inrot.y;
	lookat = eye + diff;
	
	return lookat;
}

vec4 gradclr(int idx, float place01)
{
	place01 = clamp(place01, 0.001, 0.999);

	const int 	GRAD_INP_IDX 	= 0;
	const float BLOCKH 			= 16.0;
	vec2 		wh 				= uTD2DInfos[GRAD_INP_IDX].res.zw;
	float 		y01 			= 1.0 - ( (float(idx)+0.5) * BLOCKH) / wh.y;
	
	return texture(sTD2DInputs[GRAD_INP_IDX], vec2(place01, y01));
}


float vorobj_val(ivec3 idx)
{
    vec3 f = idx;
    float a = nsin01(f.x * 0.68672 + f.y * 3.3234 - sin(f.z*4.1)*4.123, (lim_fold01(sin(f.x*9.1)+4.343243) + 1) * cos(f.y+2.5));
    
    return lim_fold01(a);
}
vec3 vorobj_drift(ivec3 idx, float drift01)
{
    float v = 7.55 * vorobj_val(idx);
    float v2 = sin(v*9.11) + 1.5;
    vec2 a = rotate(vec2(1, 0), v);
    vec2 b = rotate(vec2(1, 0), v2);
    
    vec3 findrift = normalize( vec3(a.x, 0.5*(a.y + b.x), b.y) ) * drift01;
    
    return findrift;
}

#define ELEMPARAM_COPY(STRUCTFROM, STRUCTTO, PARAM) {STRUCTTO.PARAM = STRUCTFROM.elem_prm_ ## PARAM ;}  	
#define ELEMPARAM_COPY_INST(PARAM) ELEMPARAM_COPY(Q, EP, PARAM)
vec2 do_iqnmat(vec3 p, float dist, IQNData Q)
{
	
	ElemParams EP;
	IQN_ELEM_PARAM_LIST(ELEMPARAM_COPY_INST)
		
	int mode 	= Q.mode_elemId_octopId.x;
	int elemId 	= Q.mode_elemId_octopId.y;
	int octopId = Q.mode_elemId_octopId.z;
	
	vec4 uiss = Q.scale2_smooth2;	
	vec4 scale_smooth2_octscale = vec4(uiss.x, uiss.z * uiss.z, uiss.w * uiss.w, uiss.y);
	
	return iqn_master(p, dist, elemId, octopId, mode,
					  Q.octaves, Q.NeighbOctmod, scale_smooth2_octscale, EP, Q.prm_octop);
}



SNChild MakeChild(int id)
{
	SNChild child;
	child.pos = vec3(0);
	child.scale = 0.5;
	child.rot = 0;
	child.id = id;
	child.repspacing = vec3(1);
	child.repidxfrom = ivec3(0);
	child.repidxto = ivec3(0);
	return child;
}


/*struct BPM
{
	vec3 basecolor;
	vec3 speccolor;
	vec2 RoughnessRefraction;
};*/
struct NM
{
	float 	dist;
	int 	id;
};
DECL_SOPS(NM)

NM NM_at(float dist)
{
	NM mat;
	mat.dist = dist;
	mat.id = 0;
	return mat;
}
NM NM_at(float dist, int id)
{
	NM mat;
	mat.dist = dist;
	mat.id = id;
	return mat;
}

struct Query
{
	ivec3 idx;
	int level;
};
Query Query_at(int level)
{
	Query q;
	q.idx = ivec3(0);
	q.level = level;
	return q;
}

struct SNMeta
{
	vec3 dim;
	SNChild children[MAXKIDS];
};

SNMeta mk_SNMeta(vec3 dim)
{
	SNMeta m;
	m.dim = dim;
	for (int i = 0; i < MAXKIDS; ++i)
	{
		m.children[i].id = -1;
	}
	return m;
}

vec3 idx2color(ivec3 idx, float seed)
{
	float sinarg_1 = seed * 3.4435;
	float sinarg_2 = seed *  8.7232;
	
	float s01 = nsin01(sinarg_1, sinarg_2);
	
	vec3 fidx = vec3(idx.x + s01, idx.y + s01 * 1.5, idx.z + s01 * 2.5);
	
	fidx = vec3(s01);
	
	float r = nsin01(fidx.x * 6.87, fidx.y * 3.765);
	float g = nsin01(fidx.y * 5.67, fidx.z * 2.765);
	float b = nsin01(fidx.z * 7.57, fidx.x * 4.765);
	
	return vec3(r,g,b);
}

vec3 idx2color(Query q, float seed)
{
	return idx2color(q.idx + ivec3(q.level * 3, q.level * 4, q.level * 5), seed);
}

vec3 getclr(float seed)
{
	return idx2color(ivec3(4,7,11), seed);
}

vec3 grad(int idx, float place01)
{
	place01 = clamp(place01, 0.001, 0.999);

	const int 	GRAD_INP_IDX 	= 0;
	const float BLOCKH 			= 16.0;
	vec2 		wh 				= uTD2DInfos[GRAD_INP_IDX].res.zw;
	float 		y01 			= 1.0 - ( (float(idx)+0.5) * BLOCKH) / wh.y;
	
	return texture(sTD2DInputs[GRAD_INP_IDX], vec2(place01, y01)).xyz;
}

NM axes(vec3 p)
{
	vec4 color_dist = sdf_axes(p);
	
	return NM_at(color_dist.w);
}

SNChild get_child(SNMeta m, int id)
{

			if (id == 0) return m.children[0];
#if MAXKIDS > 1			
	else 	if (id == 1) return m.children[1];
#endif
#if MAXKIDS > 2
	else 	if (id == 2) return m.children[2];
#endif	
#if MAXKIDS > 3	
	else 	if (id == 3) return m.children[3];
#endif
#if MAXKIDS > 4
	else 	if (id == 4) return m.children[4];
#endif
	
	return m.children[0];
}

mat4 get_imodel(SNChild data)
{
	mat4 ch_scale = m_scale(vec3(data.scale));
	mat4 ch_rot = m_rot(vec3(0,1,0), data.rot * 2.0 * PI);
	mat4 ch_trans = m_trans(data.pos);
	mat4 ch_model = ch_scale * ch_rot * ch_trans;
	return inverse(ch_model);
}


float rep_mir( float p, float s, int idxfrom, int idxto, out int id)
{	
	id = int(round(p/s));
	id = clamp(id, idxfrom, idxto);
	
	float  r = p - s*id;
	float m = ((int(id.x)&1)==0) ? r : -r;
	return m;
}

float rep_miry( float p, float s, float scenedim, int idxfrom, int idxto, out int id)
{	
	float m = rep_mir(p,s,idxfrom, idxto, id);
	
	float off = (s/scenedim - 1) * scenedim * 0.5;
	m += s * 0.5 - off;	
				
	return m;
}


vec3 rep_mir(vec3 p, vec3 spacing, vec3 scenedim, ivec3 idxfrom, ivec3 idxto, out ivec3 idx)
{
	int x_idx;
	float x = rep_mir(p.x, spacing.x, idxfrom.x, idxto.x, x_idx);
	
	int y_idx;
	float y = rep_miry(p.y, spacing.y, scenedim.y, idxfrom.y, idxto.y, y_idx);
	
	int z_idx;
	float z = rep_mir(p.z, spacing.z, idxfrom.z, idxto.z, z_idx);
	
	idx = ivec3(x_idx, y_idx, z_idx);
	return vec3(x,y,z);
}

vec3 do_repl(vec3 p, vec3 scenedim, SNChild child, out ivec3 idx)
{
	p.y -= scenedim.y * 0.5;
	
	vec3 final_spacing = child.repspacing * scenedim;
	
	//centering
	//vec2 xzspan = (child.repidxto.xz) * final_spacing.xz;
	//p.xz += xzspan * 0.5;
	
	return rep_mir(p, final_spacing, scenedim, child.repidxfrom, child.repidxto, idx);
}
vec3 do_repl_root(vec3 p, vec3 scenedim)
{
	p.y -= scenedim.y * 0.5;
	
	//if (false)
	{
		vec3 final_spacing = scenedim;
		
		const int YR = 1;
		
		ivec3 idx_unused;
		return rep_mir(p, final_spacing, scenedim, ivec3(0,-YR,0), ivec3(0,YR,0), idx_unused);
	}
}
vec3 rep_kaleid(vec3 p, int DoubleSubdivs, float xoffset, float ysize, ivec2 yidx_from_to, float yspacing, float rotphase01)
{
    p.xz = kaleid(p.xz, DoubleSubdivs * 2, rotphase01);
	
	int y_idx;
	p.y = rep_miry(p.y, yspacing, ysize, yidx_from_to.x, yidx_from_to.y, y_idx);
	
	p.x -= xoffset;
	
	return p;
}
vec3 rep_kaleid(vec3 p, int DoubleSubdivs, float xoffset, float rotphase01)
{
    p.xz = kaleid(p.xz, DoubleSubdivs * 2, rotphase01);
	
	p.x -= xoffset;
	
	return p;
}

float 	sop_repl_unl	(float p, float size, out int idx)
{
    int     thesign = p > 0 ? 1 : -1;
    int     halfidx = int(p / (size * 0.5));
    int     theidx  = (halfidx + thesign) / 2;
    
    //theidx = clamp(theidx, min(idxmin, idxmax), max(idxmin, idxmax));
    
    float   inner   = p - theidx * size;
    
    idx = theidx;
    return  inner;
}

NM iqn_for_all(vec3 P_orig, NM mat)
{
	vec2 themat2 = do_iqnmat(P_orig, mat.dist, iqn());
	
	vec2 themat3 = do_iqnmat(P_orig, themat2.x, iqn2());
	mat.dist = themat3.x;
	
	float iqnmat_id = themat3.y > -0.5 ? themat3.y : themat2.y;
	if (iqnmat_id > -0.5)
	{
		mat.id = f2i(iqnmat_id);
	}
	
	float clip = sdfh_box(P_orig, vec3(1,2,1));
	mat.dist = sop_int(mat.dist, clip);
	
	return mat;
}

#if ONLY_TEST_SCENE

//#include "liza_glsl"

NM test(vec3 p_orig)
{
	vec3 p = p_orig;
	
	NM RES = NM_at(SDF_OUTSIDE);
	//ADD(axes(p));
	vec3 dims = rmain_testdims();
	
	p.y += dims.y - 1.0;
	
	NM base = NM_at(sdfh_box(p, dims));	
	base = iqn_for_all(p, base);
	ADD(base);
	
	
	if (rmain_renmode().x == RENMODE_WIRE)
	{
		float dist_border = sdfh_wirebox(p, dims, 0.006);
		ADD(NM_at(dist_border))
	}
	
	//ADD(HBlock(p, rmain_testdims()));
	
	
	//ADD(fwall(p, rmain_testdims(), fw1()));
	
	return RES;
}
#endif

float masktexture(float threshold, vec2 centerpos_ndc, vec2 winsize, vec2 ndc)
{
	const int 	FMASK_INP_IDX 	= 1;
	vec2 		fin_coord01 	= n11_to_01( lim_fold11(centerpos_ndc + ndc * winsize) );
	float 		pixel_red 		= texture(sTD2DInputs[FMASK_INP_IDX], fin_coord01).x;
	return 		pixel_red > threshold ? pixel_red : 0;
}

#include "content_glsl"

int get_rootid()
{
	int ui_id = rmain_RootidIqnelidIqneloct().x;
	return ui_id;
	
	//if (ui_id == 0)
	//{
	//	return phaseidx() % 4;
	//}
	//else if (ui_id == 1)
	//{
	//	return 4;
	//}
	//else return 5;
	//
	//return -1;
}

vec3 getsnmdim()
{
	return vec3(1,2,1);
}

int get_nest_depth()
{
#if CONSTANTDEPTH >= 0
	return CONSTANTDEPTH;
#else
	return rmain_depth();
#endif
}

NM nest(vec3 p_orig)
{
#if ONLY_TEST_SCENE
	return test(p_orig);
#else	
	vec3 p = p_orig;
	
	NM RES = NM_at(SDF_OUTSIDE);
	
	//ADD(axes(p));
	
	float scale = ROOTLEVELSCALE;
	int D = get_nest_depth();	
	int rootlevel = phaseidx();
	int rootid = get_rootid();
	
	SNChild rootchild;
	rootchild.id = rootid;
	rootchild.repidxto = ivec3(0);
	
	Query rootquery = Query_at(rootlevel); //idx 0,0,0
	SNMeta 	snm 	= meta_NodeGet(rootchild.id, rootquery);
	
	//p = do_repl_root(p, snm.dim * scale); //not even using index 
	
	p.y += ROOTLEVELSCALE;
	
	bool 	outside = false;
	p /= scale;
	NM 		res 	= sdf_NodeGet(p, rootquery, rootchild.id, snm.dim, scale, outside);
	
	ADD(res);
	
	if (rootchild.id < 0) return RES;
	if (outside) return RES;
	
#define ITER_END } //just for notepad++ code folding	
#define NEST_ITER_LEVEL(LEVEL)																\
	for (int idx1 = 0; idx1 < MAXKIDS && D >= LEVEL; ++idx1)								\
	{																						\
		SNChild 	child 		= get_child(snm, idx1);										\
		if (child.id < 0) continue;															\
		float 		scale		= scale * child.scale;										\
		mat4 		ch_imodel 	= get_imodel(child);										\
		vec3 		p 			= (vec4(p, 1.0) * ch_imodel).xyz;							\
		Query 		query 		= Query_at(rootlevel + LEVEL);								\
		p 						= do_repl(p, getsnmdim(), child, query.idx);				\
		SNMeta 		snm 		= meta_NodeGet(child.id, query);							\
		bool 		outside 	= false;													\
		NM 			res 		= sdf_NodeGet(p, query, child.id, snm.dim, scale, outside);	\
		ADD(res);																			\
		if (outside) continue;
		
	#if MAXRENDEPTH >= 1
	NEST_ITER_LEVEL(1)		
		#if MAXRENDEPTH >= 2
		NEST_ITER_LEVEL(2)			
			#if MAXRENDEPTH >= 3
			NEST_ITER_LEVEL(3)	
				#if MAXRENDEPTH >= 4
				NEST_ITER_LEVEL(4)					
					#if MAXRENDEPTH >= 5
					NEST_ITER_LEVEL(5)
						#if MAXRENDEPTH >= 6						
						NEST_ITER_LEVEL(6)							
							#if MAXRENDEPTH >= 7						
							NEST_ITER_LEVEL(7)
								#if MAXRENDEPTH >= 8						
								NEST_ITER_LEVEL(8)
								ITER_END
								#endif	
							ITER_END
							#endif	
						ITER_END
						#endif						
					ITER_END
					#endif
				ITER_END
				#endif
			ITER_END
			#endif
		ITER_END
		#endif
	ITER_END
	#endif
	
	return RES;
#endif
}
//DECL_NORM_MARCH(NM, nest)
//DECL_NORMAL(nest)

vec3 nest_normal(vec3 p)
{
    vec2 	e = vec2(1.0,-1.0)*SDF_EPSILON;
	float 	a = nest(p + e.xyy).dist;		
	float 	b = nest(p + e.yyx).dist;				
	float 	c = nest(p + e.yxy).dist;						
	float 	d = nest(p + e.xxx).dist;							
	return 	normalize(a*e.xyy + b*e.yyx + c*e.yxy + d*e.xxx);
}

void get_vorspace_samplepoints(vec3 p, out vec3 p_A, out vec3 p_B)
{
	int phaseidx = phaseidx();
	
	p_A 	= p;
	p_B 	= p_A / MAINSCALE;
	
	p_A.xz = rotate01(p_A.xz, - 0.25 * phaseidx);
	p_B.xz = rotate01(p_B.xz, - 0.25 * phaseidx);
}

struct MarchRes
{
	NM mat;
#if STEP01DATA_ENABLED	
	float step01;
#endif
};

MarchRes nest_march(Ray ray, float start, float end)
{	
	MarchRes res;
	
	float depth = start;												
    for (int i = 0; i < SDF_MAX_MARCH_STEPS; i++)
	{
		vec3 point = walkray(ray, depth);
        res.mat = nest(point);
		
#if STEP01DATA_ENABLED		
		res.step01 = i / float(SDF_MAX_MARCH_STEPS-1);
#endif

        if (res.mat.dist < SDF_EPSILON)
		{
			res.mat.dist = depth;
			return res;
		}		
        depth += res.mat.dist;												
        if (depth >= end)
		{
			res.mat = NM_at(end);
			return res;		
		}
    }
	
    res.mat = NM_at(end);
	
#if STEP01DATA_ENABLED	
	res.step01 = 1;
#endif

	return res;	
}	

float nest_shadowmarch_get_brightness01(Ray ray_from_light, float start, float end)
{	
#if LIGHTING_ENABLE_SHADOWS
	float depth = start;												
    for (int i = 0; i < SDF_MAX_MARCH_STEPS; i++)
	{
		vec3 point = walkray(ray_from_light, depth);

		float dist = nest(point).dist;
		
		depth += dist;
		
        if (dist < SDF_EPSILON)
		{
			//return 0.0;
			//thats the only place where we have shadow;
			float AtLight01 = /*1.0-*/(depth / (end - start));
			vec2 shadowFromTo = lighting_AmblightShadfromTo().yz;
			return mix(shadowFromTo.x, shadowFromTo.y, AtLight01);
			//break;
		}
		
        if (depth >= end)
		{
			break;		
		}
    }
#endif

	return 1;	
}	

float lightintensityat_OLD_BUT_NOTE_ANGFIX(Light light, vec3 p, vec3 n)
{
    vec3    vec2light       = mulw(light.pos)-p;

	float realang01 = ang01_between(vec2light, n);
	vec2 maxang01 = vec2(0.4, 0.48);
	float used_ang01 = realang01;
	if (realang01 > maxang01.x)
	{
		float phase = clampphase(realang01, maxang01.x, maxang01.y);
		used_ang01 = mix(maxang01.x, 1.0, phase);
	}
	
    //1 if normal faces light, 0 if opposite
    float   angfactor01     = 1.0 - used_ang01;
    
	float 	used_light_radius = /*phasescale() * */ light.radius;
	
    //1 if point is at light, 0 is beyond range
    float   rangefactor01   = 1.0 - clamp(length(vec2light) / used_light_radius, 0, 1);
    
    return angfactor01 * angfactor01 * rangefactor01 * rangefactor01 * light.intensity01;
}

float lightintensity_rangefactor(Light light, vec3 p)
{
	float   rangefactor01   = 1.0 - clamp01(distance(mulw(light.pos), p) / light.radius);
	return rangefactor01 * rangefactor01 * light.intensity01;
}	

vec3 bpm_lighting(vec3 n, vec3 rd, vec3 l, vec3 kl, vec3 kd, vec3 ks, float km, float kn)
{
	// n -> normal
	// l -> light direction
	// rd -> ray direction
	// kl -> light color
	// kd -> diffuse color
	// ks -> specular color
	// km -> roughness / microfacet amount
	// kn -> refraction index

    float ndl = clamp(dot(n, l), 0., 1.); // diffuse/lambert / N⋅L
    
	vec3 h = normalize(l - rd); // half vector
    float ndh = max(dot(n, h), 0.0);
    
    // ggx / Trowbridge and Reitz specular model approximation
    float g = ndh*ndh * (km*km - 1.) + 1.;
    float ggx = km*km / (3.141592*g*g);

    // shlick approximation
    float fre = 1.+dot(rd, n); // fresnel
	
    // fresnel amount
    float f0 = (kn-1.)/(kn+1.);
          f0 = f0*f0;
    float kr = f0 + (1.-f0)*(1.-km)*(1.-km) * pow(fre, 5.); // reflectivity
    
    return kl*ndl*(kd + ks*kr*ggx); // diffuse + specular
}

vec3 bpm_lighting(vec3 normal, vec3 raydir, vec3 lightdir, vec3 lightcolor, BPM mat)
{
	vec3 	kd = mulw(mat.basecolor);
	vec3 	ks = mulw(mat.speccolor);
	float 	km = mat.RoughnessRefraction.x;
	float 	kn = mat.RoughnessRefraction.y;
	
	return bpm_lighting(normal, raydir, lightdir, lightcolor, kd, ks, km, kn);
}

vec3 getlightcontribution_withshadow(Light light, vec3 p, vec3 n, vec3 raydir, BPM bpm, float centerdistfilter)
{
	float rawintensity  = lightintensity_rangefactor(light, p);
	
	vec3 rescolor = vec3(0);
	
	if (length(light.pos.xz) < centerdistfilter) return rescolor;
	
	if (rawintensity > 0.00001)
	{
		vec3 dir2light = normalize(mulw(light.pos)-p);
		Ray ray = Ray(p, dir2light);
		
		float brightness01 = nest_shadowmarch_get_brightness01(ray, mindist(), distance(ray.origin, mulw(light.pos)));
		//if (brightness01 > 0.99)
		{
			rescolor = bpm_lighting(n, raydir, dir2light, mulw(light.color) * rawintensity, bpm);
			rescolor *= brightness01;
		}
		
	}
	
	return rescolor;
}

vec3 get_vor_vorlightpos(vec3 p_orig, Vor par, out float lightradius)
{
    vec3 p = p_orig;
    
    vec3    cellsize        = mulw(par.cellsize);

	lightradius = min(cellsize.x, min(cellsize.y, cellsize.z)) * 0.5;

    vec3    query_coord01   = vec3(0);
    ivec3   query_idx       = to_cellspace(p, cellsize, query_coord01);
    
    float best_score = 999;
    vec3  best_objmid = vec3(0);
    
    const int CR =1;
    for (int dy = -CR; dy <= CR; ++dy)
    {
        for (int dx = -CR; dx <= CR; ++dx)
        {
            for (int dz = -CR; dz <= CR; ++dz)
            {
                ivec3   this_idx        = query_idx + ivec3(dx, dy, dz);
                vec3    this_midcoord   = (vec3(this_idx) + vorobj_drift(this_idx, par.obj_drift01)) * cellsize;
                
                vec3    vec2thismid     = this_midcoord - p;
				
                float this_score = length(vec2thismid/cellsize);
                
                if (this_score < best_score)
                {
                    best_score = this_score;
                    best_objmid = this_midcoord;
                }
            }
        }
    }

	return best_objmid;
}
vec3 vorlight_getlightcontribution(vec3 p_orig, vec3 n, vec3 raydir, BPM bpm)
{
	vec3 p = p_orig;
	
	float lightradiusmod = mainlight_radius();
	
	vec3 	vor_samplept_A;
	vec3 	vor_samplept_B;
	get_vorspace_samplepoints(p, vor_samplept_A, vor_samplept_B);
	
	Vor par = vor();
	float 	phase01 		= pow(phase01(), par.phasepow);
	int phaseidx = phaseidx();
	
	//vec3    cellsize_real        = mulw(par.cellsize) / par.scale;
	
	vec3 rescolor = vec3(0);
	
	Light light = mainlight();
	float baseintensity = light.intensity01;
	float basecenterdistfilter = 0.003; //play with it
	
	//test todo
	basecenterdistfilter = 0;
	
	{
		float maxAllowedRadius = 0;
		
		vec3 lightpos = get_vor_vorlightpos(vor_samplept_A, par, maxAllowedRadius);
		lightpos.xz = rotate01(lightpos.xz, 0.25 * phaseidx);
		
		light.pos = vec4(lightpos, 1.0); //make sure .w is always 1.0
		
		light.radius = maxAllowedRadius * lightradiusmod;
		
		light.intensity01 = baseintensity * (1.0-phase01);
	}
	
	rescolor += getlightcontribution_withshadow(light, p_orig, n, raydir, bpm, basecenterdistfilter);
	
	{
		float maxAllowedRadius = 0;
		
		vec3 lightpos = get_vor_vorlightpos(vor_samplept_B, par, maxAllowedRadius);

		lightpos *= MAINSCALE;
		lightpos.xz = rotate01(lightpos.xz, 0.25 * phaseidx);
		
		light.pos = vec4(lightpos, 1.0); //make sure .w is always 1.0	
		
		light.radius = maxAllowedRadius * MAINSCALE * lightradiusmod;
		
		light.intensity01 = baseintensity * phase01;
	}
	
	rescolor += getlightcontribution_withshadow(light, p_orig, n, raydir, bpm, basecenterdistfilter * MAINSCALE);
	
	return rescolor;
}

//example materials:
BPM iron      = BPM(vec4(0.6, 0.6, 0.6, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec2(0.5, 1.5));      
BPM dirt      = BPM(vec4(0.4, 0.3, 0.1, 1.0), vec4(0.02, 0.02, 0.02, 1.0), vec2(0.8, 1.3));   
BPM mirror    = BPM(vec4(0.0, 0.0, 0.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0), vec2(0.01, 1.0));     
BPM glass     = BPM(vec4(0.01, 0.01, 0.01, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec2(0.05, 1.5));  
BPM grass     = BPM(vec4(0.13, 0.3, 0.05, 1.0), vec4(0.03, 0.07, 0.03, 1.0), vec2(0.6, 1.33));
BPM plastic   = BPM(vec4(0.5, 0.5, 0.5, 1.0), vec4(0.3, 0.3, 0.3, 1.0), vec2(0.25, 1.49));    
BPM gold      = BPM(vec4(1.0, 0.8, 0.1, 1.0), vec4(1.0, 0.8, 0.1, 1.0), vec2(0.1, 0.47));     
BPM silver    = BPM(vec4(0.7, 0.7, 0.7, 1.0), vec4(0.9, 0.9, 1.0, 1.0), vec2(0.18, 0.15));    
BPM copper    = BPM(vec4(0.9, 0.4, 0.22, 1.0), vec4(0.9, 0.4, 0.22, 1.0), vec2(0.25, 0.26));  
BPM wood      = BPM(vec4(0.5, 0.34, 0.13, 1.0), vec4(0.1, 0.1, 0.09, 1.0), vec2(0.6, 1.55));  
BPM stone     = BPM(vec4(0.4, 0.4, 0.45, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec2(0.65, 1.55));   
BPM marble    = BPM(vec4(0.8, 0.8, 0.8, 1.0), vec4(0.5, 0.5, 0.5, 1.0), vec2(0.18, 1.49));    
BPM water     = BPM(vec4(0.01, 0.01, 0.05, 1.0), vec4(0.2, 0.2, 0.2, 1.0), vec2(0.05, 1.33)); 
BPM skin      = BPM(vec4(1.0, 0.76, 0.65, 1.0), vec4(0.10, 0.06, 0.05, 1.0), vec2(0.8, 1.38));
BPM asphalt   = BPM(vec4(0.07, 0.07, 0.07, 1.0), vec4(0.01, 0.01, 0.01, 1.0), vec2(0.6, 1.6));
BPM chrome    = BPM(vec4(0.55, 0.57, 0.58, 1.0), vec4(0.99, 0.99, 0.99, 1.0), vec2(0.08, 1.0));
BPM aluminum  = BPM(vec4(0.77, 0.77, 0.77, 1.0), vec4(0.9, 0.92, 0.98, 1.0), vec2(0.07, 1.45));
BPM ceramic   = BPM(vec4(0.85, 0.78, 0.68, 1.0), vec4(0.3, 0.3, 0.3, 1.0), vec2(0.3, 1.54));   
BPM paper     = BPM(vec4(0.9, 0.9, 0.8, 1.0), vec4(0.1, 0.1, 0.09, 1.0), vec2(0.45, 1.47));    
BPM rubber    = BPM(vec4(0.02, 0.02, 0.02, 1.0), vec4(0.05, 0.05, 0.05, 1.0), vec2(0.9, 1.51));

BPM grass_vivid     = BPM(vec4(0.16, 0.35, 0.07, 1.0), vec4(0.03, 0.08, 0.04, 1.0), vec2(0.5, 1.38));        // vivid grass blade
BPM clover_leaf     = BPM(vec4(0.20, 0.46, 0.15, 1.0), vec4(0.02, 0.06, 0.02, 1.0), vec2(0.5, 1.42));        // clover leaf
BPM fern            = BPM(vec4(0.13, 0.33, 0.14, 1.0), vec4(0.03, 0.06, 0.03, 1.0), vec2(0.5, 1.41));        // fern (green)
BPM bamboo_leaf     = BPM(vec4(0.28, 0.52, 0.15, 1.0), vec4(0.05, 0.07, 0.01, 1.0), vec2(0.4, 1.44));        // bamboo leaf
BPM spinach_leaf    = BPM(vec4(0.21, 0.40, 0.18, 1.0), vec4(0.03, 0.09, 0.03, 1.0), vec2(0.55, 1.40));       // spinach leaf

BPM moss            = BPM(vec4(0.24, 0.40, 0.12, 1.0), vec4(0.03, 0.09, 0.04, 1.0), vec2(0.65, 1.39));       // moss
BPM lettuce_leaf    = BPM(vec4(0.30, 0.68, 0.24, 1.0), vec4(0.05, 0.10, 0.02, 1.0), vec2(0.45, 1.36));       // lettuce leaf
BPM water_lily_leaf = BPM(vec4(0.17, 0.33, 0.08, 1.0), vec4(0.09, 0.17, 0.06, 1.0), vec2(0.38, 1.32));       // water lily pad
BPM ivy_leaf        = BPM(vec4(0.11, 0.23, 0.09, 1.0), vec4(0.04, 0.10, 0.04, 1.0), vec2(0.52, 1.46));       // ivy (dark green)
BPM cactus_skin     = BPM(vec4(0.18, 0.36, 0.14, 1.0), vec4(0.09, 0.11, 0.08, 1.0), vec2(0.62, 1.48));       // cactus skin

BPM withered_leaf   = BPM(vec4(0.23, 0.28, 0.13, 1.0), vec4(0.03, 0.05, 0.02, 1.0), vec2(0.68, 1.47));       // withered green leaf
BPM autumn_leaf     = BPM(vec4(0.65, 0.30, 0.10, 1.0), vec4(0.09, 0.07, 0.04, 1.0), vec2(0.72, 1.45));       // autumn leaf (orange)
BPM yellow_leaf     = BPM(vec4(0.72, 0.67, 0.13, 1.0), vec4(0.10, 0.12, 0.01, 1.0), vec2(0.77, 1.43));       // yellow dry leaf
BPM red_maple_leaf  = BPM(vec4(0.73, 0.21, 0.17, 1.0), vec4(0.10, 0.09, 0.06, 1.0), vec2(0.70, 1.44));       // red maple leaf
BPM poppy_petal     = BPM(vec4(0.94, 0.35, 0.15, 1.0), vec4(0.11, 0.08, 0.05, 1.0), vec2(0.58, 1.39));       // poppy (orange-red) petal

BPM marigold_petal  = BPM(vec4(0.98, 0.62, 0.12, 1.0), vec4(0.14, 0.13, 0.03, 1.0), vec2(0.62, 1.37));       // marigold (yellow-orange) petal
BPM sunflower_petal = BPM(vec4(0.97, 0.85, 0.20, 1.0), vec4(0.13, 0.16, 0.05, 1.0), vec2(0.60, 1.41));       // sunflower petal
BPM lily_petal      = BPM(vec4(0.98, 0.95, 0.92, 1.0), vec4(0.18, 0.17, 0.16, 1.0), vec2(0.64, 1.36));       // white lily petal
BPM dried_leaf = BPM(vec4(0.62, 0.48, 0.21, 1.0), vec4(0.07, 0.06, 0.03, 1.0), vec2(0.67, 1.41));  // dried autumn leaf
BPM daisy_petal     = BPM(vec4(0.96, 0.93, 0.89, 1.0), vec4(0.15, 0.15, 0.15, 1.0), vec2(0.70, 1.52));       // daisy (white) petal

BPM plantmats[20] = { grass_vivid, clover_leaf, fern, bamboo_leaf, spinach_leaf, moss, lettuce_leaf, water_lily_leaf, ivy_leaf, cactus_skin, withered_leaf, autumn_leaf, yellow_leaf, red_maple_leaf, poppy_petal, marigold_petal, sunflower_petal, lily_petal, dried_leaf, daisy_petal };

//marble instead of mirror
BPM defaultmats[20] = {iron, dirt, marble, glass, grass, plastic, gold, silver, copper, wood, stone, marble, water, skin, asphalt, chrome, aluminum, ceramic, paper, rubber};
BPM coolmats[8] = {stone, iron, ceramic, silver, skin, copper, marble, silver};


BPM get_final_bpm(NM mat)
{
	int id = mat.id + int(rmain_colorseed());
	
#if MTL_COLLECTION == MTL_PLANTS
	return plantmats[id % 20];
#elif MTL_COLLECTION == MTL_COOL
	return coolmats[id % 8];
#elif MTL_COLLECTION == MTL_DEFAULT
	return defaultmats[id % 20];
#endif
}

vec3 throughlights(BPM bpm, vec3 p, vec3 n, vec3 raydir)
{
    float   	ambient         = pow(lighting_AmblightShadfromTo().x, 2.0);
    
	vec3 color = mulw(bpm.basecolor) * ambient;
	
	vec3 mainlightpos = mulw(mainlight_pos());
	if (length(mainlightpos) < 0.00001)
	{
		color 		+= vorlight_getlightcontribution(p, n, raydir, bpm);
	}
	else
	{
		color 		+= getlightcontribution_withshadow(mainlight(), p, n, raydir, bpm, 0);
	}
    
   return color;
}

vec3 Tonemap_ACES(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 nest_main(Ray ray)
{
	float themaxdist = maxdist();
	const RenFin renfin = renfin();
	
    MarchRes 	marchres    = nest_march(ray, mindist(), themaxdist);
	
	NM 			mat 		= marchres.mat;
	BPM			bpm 		= get_final_bpm(mat);
	
    float   	dist    	= mat.dist;    
    vec3    	p   		= walkray(ray, dist);
	
	vec3		fogcolor	= mulw(lighting_fogcolor());
    
	

    float 		useLighting01 	= renfin.stepcNvalc.z;
    
	
	vec3 		stdrencolor 	= fogcolor;
    if (dist < themaxdist - SDF_EPSILON)
    {
		vec3    n       = nest_normal(p);
		
		#if LIGHTING_ENABLED
			vec3 	stdrenwithlight = throughlights(bpm, p, n, ray.dir);
		#else
			vec3 	stdrenwithlight = mix(mulw(bpm.basecolor), n, 0.3);
		#endif
		
		float	fogratio		= clamp((lighting_fogdistfactor() * dist) / themaxdist, 0, 1);
		vec3 	stdrenwithfog 	= mix(stdrenwithlight, fogcolor, fogratio);
		stdrencolor 			= stdrenwithfog;
	}
	
	vec3 fincolor = stdrencolor * useLighting01;
	
	#if STEP01DATA_ENABLED
	{	
		float 		step01 		= clamp01(powmul(marchres.step01, renfin.step_powmul));
		vec3 		stepcolor 	= grad(0, step01);
		float 		useStep01 	= renfin.stepcNvalc.w;
		
		fincolor += stepcolor * useStep01;
	}
	#endif	
	
	fincolor = pow(fincolor, vec3(renfin.val_powmul.x)) * renfin.val_powmul.y;
	
	#if TONEMAP_ACES_ENABLED
	{	
		fincolor = Tonemap_ACES(fincolor);
	}
	#endif
	
	return clamp01(fincolor);
}

#if SHOW_IQNEL_WINDOWS

#define DECL_SIMPLECOLOR(SDFFUNC) \
vec3 SDFFUNC ## _simplecolor(Ray ray){\
	const float mindist = SDF_MIN_DIST;\
	const float maxdist = SDF_MAX_DIST;\
	float 		   dist = SDFFUNC ## _march(ray, mindist, maxdist).dist;\
	if (dist < maxdist - SDF_EPSILON)\
	{\
		vec3    p   = walkray(ray, dist);\
		vec3    n   = SDFFUNC ## _normal(p);\
		return 	n11_to_01(n);\
	}\
	return vec3(0.1);\
}

NM iqnel_view(vec3 p, IQNData Q)
{
	ElemParams EP;
	IQN_ELEM_PARAM_LIST(ELEMPARAM_COPY_INST)
	
	float 			scale 		 = 0.66;
	int 			elemId 		 = Q.mode_elemId_octopId.y;	
	
	vec3 idx;
	{
		float uidx = float(rmain_RootidIqnelidIqneloct().y);
		int x = int(110 * hash(uidx * 0.623));
		int y = int(120 * hash(uidx * 0.721));
		int z = int(130 * hash(uidx * 0.524));
		idx = vec3(x,y,z);
	}
	
	float depth01 = depth01(rmain_RootidIqnelidIqneloct().z, 10);
	
	float iqnelem = iqn_sdf_single(p / scale, elemId, idx, EP, depth01) * scale;
	
	float RES = SDF_OUTSIDE;
	ADD(iqnelem);	
	//if (rmain_renmode().x == RENMODE_WIRE)
	{
		vec3 dims = vec3(1);
		float dist_border = sdf_wirebox(p / scale, dims * 0.5, 0.006) * scale;
		ADD(dist_border);
	}
	return NM_at(RES);
}

NM iqnel_view_A(vec3 p)
{
	return iqnel_view(p, iqn());
}
DECL_NORM_MARCH(NM, iqnel_view_A)
DECL_SIMPLECOLOR(iqnel_view_A)

NM iqnel_view_B(vec3 p)
{
	return iqnel_view(p, iqn2());
}
DECL_NORM_MARCH(NM, iqnel_view_B)
DECL_SIMPLECOLOR(iqnel_view_B)

#endif

vec4 nest_master(vec2 pixCoord)
{
	vec3	eye 	= get_eye(rmain_renmode().y, rmain_camera().x);
	vec3	lookat 	= get_lookat(eye);
	Ray 	ray 	= worldray_for_pixel(pixCoord,
										 agn_resolution(),
										 eye,
										 lookat,
										 45.0 * rmain_camera().w);
	
	#if SHOW_IQNEL_WINDOWS
	{
		#define SHOW_WINDOW(FUNC, WINIDX)  \
		{\
			vec2 	ndc 			= agn_ndc();\
			float	xspan 			= IQNEL_WINDOW_SIZE * (0.5 / aspect());\
			float 	yspan			= IQNEL_WINDOW_SIZE * (0.5);\
			vec2 	screenFrom 		= vec2(-1 + xspan * (WINIDX), -1);\
			vec2 	screenTo   		= vec2(screenFrom.x + xspan, -1 + yspan);\
			vec2 	screenUv 		= remap_ff(screenFrom, screenTo, vec2(0), vec2(1), ndc);\
			if (pt_in_01(screenUv))\
			{\
				vec3    eye   	= normalize(get_eye(0, 1.1));\
				vec3 	lookat	= vec3(0);\
				Ray 	ray 	= worldray_for_pixel(screenUv,\
													 vec2(1,1),\
													 eye,\
													 lookat,\
													 90.0);\
				return vec4( FUNC(ray), 1);\
			}\
		}
		
		#if SHOW_IQNEL_WINDOWS > 0
			SHOW_WINDOW(iqnel_view_A_simplecolor, 0);
		#endif
		#if SHOW_IQNEL_WINDOWS > 1
			SHOW_WINDOW(iqnel_view_B_simplecolor, 1);
		#endif
	}
	#endif
	
	return vec4( nest_main(ray), 1);
}

#if TS_DS_ENABLED
DS_Disp d_big  = DS_Disp(ivec2(0,0), ivec2(4,4), ds_vp_q1234(1,1));
DS_Disp d_num  = DS_Disp(ivec2(7,0), ivec2(1,1), ds_vp_q1234(1,1));
void do_ts_ds_stuff()
{
	if (false)
	{
		float x = ds_dspx(d_big);
		float y = nsinimpulse(x, 2);
		ds_ploty(d_big, y);
	}
	
	
	
	
	//ts_num.x = -1357.446;
	
	
	ts_ds_printnums(d_num);
}
#endif

vec4 thefinal()
{
	vec2 ndc = agn_ndc();
	vec2 pix = get_pix_ndc().xy;

	bool ui_MS_button_pressed = ds_chselector.y != 0;;
		
	bool do_multisampling = CND_ENABLE_MULTISAMPLING != 0 &&
							(ui_MS_button_pressed || CND_ENABLE_MULTISAMPLING < 0);
	vec4 RES;
	
	if (do_multisampling)	
	{
		#if CND_ENABLE_MULTISAMPLING == 1 || CND_ENABLE_MULTISAMPLING == -1
		{
			vec4 sum_samples = vec4(0);
			vec2 a = vec2(0.25, 0.25);
			vec2 b = vec2(-0.25, 0.25);
			vec2 c = vec2(0, -0.25);
			sum_samples += nest_master(pix + a);
			sum_samples += nest_master(pix + b);
			sum_samples += nest_master(pix + c);
			RES = sum_samples / 3.0;
		}
		#elif CND_ENABLE_MULTISAMPLING == 2 || CND_ENABLE_MULTISAMPLING == -2
		{
			vec4 sum_samples = vec4(0);
			for (int y = -1; y <= 1; ++y)
			{
				for (int x = -1; x <= 1; ++x)
				{
					vec2 off = vec2(float(x), float(y)) * 0.3333;
					sum_samples += nest_master(pix + off);
				}
			}		
			RES = sum_samples / 9.0;
		}
		#else
		{
			RES = vec4(1,0,1,1); //error
		}
		#endif
	}
	else
	{
		RES = nest_master(pix);
	}
	
	
#if TS_DS_ENABLED	
	do_ts_ds_stuff();
	RES = ds_apply(RES);
#endif	
	
	return RES;
}
/*void new_ts_ds_examples()
{
    ds_ploty(dc1, pow(ds_dspx(dc1), 2));
    ds_ploty(dn1, sin(ds_dspx(dn1)*PI));        
    ts_num.x = -1357.446;
    ts_num.w = time;
    ds_setc(1, vec4(length(agn_uv())));
}*/






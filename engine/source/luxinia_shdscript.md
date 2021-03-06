# Shader Script .SHD

## Definition
A shader allows multiple texture effects on a single surface. That can be blending different textures or complex operations such as Cg or Vertex/Fragment programs.\
The shader defines how textures should be blend, while a MTL-[[Api/Material]] sets the textures to be used, however the shader can have hardwired textures and special textures.

It is made of several stages, you should try to limit the amount of stages cause it will affect performance.

The header is necessary\
current layout version:\
luxinia_Shader_v310

### Syntax
All commands are case sensitive and closed by `;` or "newline". A keyword and its arguments may not span multiple lines.\
`<id>` ranges from 0 to 7 \
`<vector4>` is (float,float,float,float) \
`<bool>` is 1 = true, 0 = false

### Branching
You can do branching with following commands. Enclose the branches in curly brackets `{ }`

* `IF:<condition string>`
* `ELSEIF:<condition string>`
* `ELSE`

The condition string can be set from luxinia API with `resource.condition`, or it may be part of a "define" in the Cg Compiler string.\
You can also negate a statement with !`<condition string>`.\
Following conditions are automatically set if applicable:
  * `CAP_MODADD` capability for modulated add texture combiner
  * `CAP_COMBINE4` capability for combine4 texture combiner
  * `CAP_TEX3D` capability for accelerated 3d textures
  * `CAP_TEXFLOAT` capability for float textures formats (float32 and float16)

Be aware that the parser is not fully rock solid, so at best use
`COMMAND{<newline> <what><newline> }<newline>` , when problems occur.

### Annotations & Comments
You can add annotations anywhere in the file and later query them after load. 
* `<<_ "name";` serves as start  and is skipped by branching/comments or ignored TECHNIQUEs.
* `_>>` denotes the end of the custom string.
Anything between the two will become part of the annotation, so use with caution.

``` cpp 
// comment until lineend

// multi-line annotation
<<_ "A"	
test = a * b
// this will be part of annotation as well
blahblubb
_>>

/* block commenting ...
<<_ "B"; min=25,max=90 _>>;	
//inlined annotation but ignored, due to active block comment
*/


```

C-style comments with `//` and `/*...*/` may not start within command & keywords. 
So start comments always after `;` in a line that contains command words.



----
## Example

``` cpp 
luxinia_Shader_v310
Technique:VID_LOWDETAIL{
	Texture{
		TEX "Texture:0"
	}
}
Technique:VID_CG_TEX4{
	RenderFlag{
		//comment blah
		lit;
		nocull;
	}
	GpuProgram{
		BASE 0 "gpuprogs/test.cg" "unlit";
		SKIN 0 "gpuprogs/test.cg" "skin_unlit";
		BASE 1 "gpuprogs/test.cg" "lit1";
		SKIN 1 "gpuprogs/test.cg" "skin_lit1";
		
		VCG;
		
		param "angle" 0 VID_VALUE (45.0,0.0,0.0,1.0);
		tangents;
		
	}
	GpuProgram{
		BASE 0 "gpuprogs/test_pixel.cg" "main";
		FCG;
		param "color" 0 VID_LIGHTCOLOR (0.0,0.0,0.0,0.0) 0;
	}
	Color{
		RGBA "Color:0";
	}
	Texture{
		TEX "Texture:1";
	}
	Texture{
		ATTENUATE3D;
	}
	Texture{
		NORMALIZE;
	}
	/*	blockcomment
		...
	*/
}
Technique:VID_DEFAULT{
	RenderFlag{
		lit;
		nocull;
	}
	Color{
		RGBA "Color:0";
	}
	Texture{
		VTEX "Texture:0";
	}
	IF:DOSIMPLEANI{
		Texture{
			TEX "textures/fx/aniso.jpg";
			blendmode VID_ADD;
		}
	}
} 
```


----
## Technique
you can specify techniques if the same shader should look different depending on hardware capabilities, if no technique is specified VID_DEFAULT is assumed.\
If a technique is defined there also must be a VID_DEFAULT technique. The first technique that is valid for the system's hardware will be used, so you should put the "highest detail" ones above default in. If there is any VID_LOWDETAIL must come first.
It works like the IF in branching, just that the parser will stop after the first valild technique was over.

following techniques are defined:
* basic techniques:
  * `VID_DEFAULT`\
always works on all hardware (blendmode VID_NORMAL might not)
  * `VID_LOWDETAIL`\
is used when user set engine details to low.

* higher techniques in ascending priority:
  * `VID_ARB_TEXCOMB`\
requires ARB texture_env_combine, crossbar (or NV combine4), dot3 and cubemaps.
  * `VID_ARB_V`\
requires ARB vertex programs and combiners (sm1 cards)
  * `VID_ARB_VF`\
requires ARB vertex program and fragment program (sm2 cards)
  * `VID_CG_SM3`\
requires Cg support and shader model 3 capabilities along with 16 texture images
  * `VID_CG_SM4`\
requires Cg support and shader model 4 capabilities along with 32 texture images
  * `VID_CG_SM4_GS`\
requires Cg support and shader model 4 capabilities with geometry shader along with 32 texture images (requires luxinia.exe with SM4_GS support)
	
* `_TEX4`\
means it also needs 4 texture images (not for CG modes)
* `_TEX8`\
means it also needs 8 texture images (only for VID_ARB_VF)


----
## RenderFlag
These are independent from stages and used for all surfaces
assigned, so be careful.\
They are added to the object's renderflags when the material using
this shader is assigned to the object. You can change the
mesh/object's renderstate afterwards using the `renderinterface.renderflag` in luxinia.api.

* `nocolorarray;`\
turns of color array (often required when you want Color Stages)
* `sunlit;`\
surface is shaded by sunlights
* `unlit;`\
surface is not shaded by any lights (forces lights off)
* `nocull;`\
turns backface culling off
* `nodepthmask;`\
wont write into depthmask (ie. no depth information stored)
* `nodepthtest;`\
wont do depth testing (ie. always on top when drawn)
* `alphamask;`\
quick for: alphafunc GL_GREATER	(0.0)
* `alphaTEX "filename.ext" <texchannel uint>;`\
when a shader uses alphatesting and projectors are applied\
this texture's alpha will be used as mask for all. Alternatively it can be stored for baking processes.\
Same mechanism as for the other TEX commands at Texture is used.\
If shader blend-compilation creates problems, use newpass mechanism and manually define combiners to preserve the alpha you need.
  * `texchannel`\
optionally you can define which texcoord channel to use 0-3. See texcoord command below.
* `normals;`\
means the program needs normals as pervertex attrib

* `alphafunc <enum func> (<float ref>);`\
pixels are rejected if they dont pass the alphatest.
  * `func`\
same as in OGL\
GL_NEVER, GL_ALWAYS, GL_LESS, GL_GREATER, GL_LEQUAL, GL_GEQUAL, GL_EQUAL, GL_NOTEQUAL
* `layer <int 0-15>;`\
to move surfaces in certain draw layers. This allows control over when things are drawn.

* `blendmode <enum>;`
* `blendinvertalpha;`\
see [[# cmd | blendmode command]] for details. Be aware that "DrawPass" blendmodes,\
as well as blendmodes for first stages in blend-compile mode will override this
setting.


----
## Stages
There are 4 different types of stages. All stages are processed in the same order, it is written in the shader file.\
Limits: MAX_TEXSTAGES = 16,	MAX_COLORSTAGES = 1\
Beyond the limit will be ignored

### DrawPass
A pass is started from here on.\
Be warned that once you start using
DrawPass no automatic blend-compilation will be done. This is why you
must make sure, that the stages after and before newpass can actually
be processed within a single pass. This means on Techniques with 2
Texture units you must not use more than 2 TextureStages.

The advantage is that you can use a lot more blendmodes/combiners for texturing. You can also define your own here or with the lua api.

### Color
Just a simple color, overrides vertexcolors of surface. For most "models" you will need to use "nocolorarray" renderflag
to actually see an effect of the values specified here.

* `RGBA "Color:n";`\
`n` is id in material


### Texture
A single texturemap: 

* `TEX "filename.ext";`
  * `TEX` : RGB/RGBA
  * `TEXALPHA` : ALPHA
  * `V???` : auto sets vertexcolored flag	
* `TEX/VTEX "Texture:n";`\
`n` is id in Material
* System needs cubemap support:
  * `TEXPROJ`: RGB/RGBA with cubemap using given texture as +Z
  * `TEXDOTZ`: INTENSITY/RGB/RGBA with cubemap using given 1d texture as dot(vector,+z), -1 is left and +1 is right.
  * `TEXCUBE`: 6 textures separated by comma make this texture\
"pos_x.jpg,neg_x.jpg,pos_y.jpg,neg_y.jpg,pos_z.jpg,neg_z.jpg"\
or a single .dds file "cubemap.dds"	
* User Textures
  * if the name string looks like "USER_TEX(name)" then the engine will lookup\
user-created textures or registered textures with this name.			
* Special Textures
  * `LIGHTMAP;`\
the lightmap for the instance of current model\
there cannot be more than a single LIGHTMAP texture stage
* For higher Techniques:
  * `ATTENUATE3D;`\
a 3d texture with radial square falloff
  * `NORMALIZE;`\
a normalization cubemap
  * `SKYBOX;`\
the skybox cubemap
  * `SPECULAR;`\
specular cubemap lighting, intensity = (MAX (0,(texcoord dot (0,0,1)))^8
  * `DIFFUSE;`\
diffuse cubemap lighting, intensity = MAX(0,texcoord dot (0,0,1))
  * `DUMMY;`\
this texture serves as dummy whenever you need a texture-stage but no texture. This is mostly for activating texcoords for GpuPrograms when no textures themselves are needed.
						
### GpuProgram		
only allowed in higher techniques, you are responsible for pass setup as in DrawPass.\
To use more than one pair of GpuPrograms (vertex/fragment) you need to use DrawPass mechanism.\
Also, GpuPrograms of a pass must come next to each other. When Cg is used none of the "OpenGL" state
variables are allowed ("state.???" or "gl_???").
			
* `BASE <int lightnum> "filename.ext" "entryname"`
  * `lightnum`\
is not needed but can be specified from 0 to 4
  * `entryname`\
For Cg shaders this is the entryfunction name. The string can contain compilerarguments separated with a ^ "entryname^compilerargs". The actual entryname is the string before ^.\
Multiple Cg compiler settings can be set, e.g. "entryname^-Dsetting=0.5;-DPIXELLIT;". Each arg must be separated by ";".\
The engine can also load precompiled shaders, therefore the actual filename is changed accordingly. "filename.ext" becomes "filename.ext.entryname^compilerargs^profile.???",\
where ??? is glp for Programs or glsl for GLSL \
The compilerargs are concatted by settings passed by material or shader loading and the rendersystem.cgcompiler string. 

For Vertex Processing
* `SKIN <int lightnum> "filename.ext" "entryname"`\
optional with skinning support, same vertex programtype as the basic one. To really allow gpu skinning all vertex stages must specifcy a skin program.\
And all programs within shader must be of same type. Automatically adds "-DVID_BONES=???;", with ??? set by luxinia api, to the compilerargs.

For Fragment Processing
* `FOGGED <int lightnum> "filename.ext" "entryname"`\
optional, used when fog state is set
					
* `<enum type>;`
  * `type`\
`VPROG` ARB_VERTEX_PROGRAM (very common)\
`VCG` Cg Shader (very common)\
`FPROG` ARB_FRAGMENT_PROGRAM (SM2+)\
`FCG` Cg Shader (SM2+)\

* `lowCgProfile;`\
optional flag, allows compilation of the program to a low CgProfile (typically arbvp1 or arbfp1).\
All GpuPrograms in Techniques `<` VID_CG_SM3 will be compiled as "lowCgProfile". And all above with the maximum CgProfile,\
unless this flag is set. Typically you would use this flag on VertexPrograms, that are sufficient with the simpler \
profiles, as internally that is cheaper.
	
#### Notes:
* All (only Cg)\
`Connect Parameters`\
Cg requires you to use a few standard matrices. Any more special matrices can be queried with the "param" command, or shadercghostparam class.\
You can either use the variables directly (same name) or vie Semantics.
  * `float4x4 WorldMatrix` - the current local2world matrix\
semantics: *WORLD, WORLDMATRIX*
  * `float4x4 WorldViewProjMatrix` \
semantics: *WORLDVIEWPROJ, WORLDVIEWPROJECTION, WORLDVIEWPROJMATRIX, WORLDVIEWPROJECTIONMATRIX*
  * `float4x4 ViewMatrix` \
semantics: *VIEW, VIEWMATRIX*
  * `float4x4 ViewMatrixInv` \
semantics: *VIEWMATRIXINV, VIEWINV, VIEWINVERSE, VIEWINVERSEMATRIX, VIEWINVMATRIX*
  * `float4x4 ProjMatrix` \
semantics: *PROJ, PROJECTION, PROJECTIONMATRIX, PROJMATRIX*
  * `float4x4 ProjMatrixInv` \
semantics: *PROJMATRIXINV, PROJINV, PROJECTIONINV, PROJINVERSE, PROJECTIONINVERSE, PROJINVMATRIX, PROJECTIONINVMATRIX, PROJINVERSEMATRIX, PROJECTIONINVERSEMATRIX*
  * `float4x4 ViewProjMatrix` \
semantics: *VIEWPROJECTION, VIEWPROJ, VIEWPROJMATRIX, VIEWPROJECTIONMATRIX*
  * `float3 FogColor` \
semantics: *FOGCOLOR*
  * `float3 FogDistance` - (start,end,density)\
semantics: *FOGDISTANCE*
  * `float3 CameraWorldPos` \
semantics: *CAMERAWORLDPOS, VIEWPOSITION, VIEWPOS*
  * `float3 CameraWorldDir` \
semantics: *CAMERAWORLDDIR, VIEWDIRECTION, VIEWDIR*
  * `float2 ViewportSize` \
semantics: *VIEWPORTSIZE, VIEWPORTDIMENSION*
  * `float2 ViewportSizeInv`  - 1.0/size\
semantics: *VIEWPORTSIZEINV, VIEWPORTSIZEINVERSE, VIEWPORTDIMENSIONINV, VIEWPORTDIMENSIONINVERSE, INVERSEVIEWPORTDIMENSIONS*

* Skinning
  * program.env[0..VID_BONESLAST] (ARB_prog) or <br> `float4x3 Bonesmatrices[VID_BONES]` <br> semantics: *BONES, BONESMATRICES* \
matrices can contain up to VID_BONESMAX float4s saved as float4x3 matrices
    * VID_BONES = ((96 or 256 depending on hardware) - rendersystem_hwbonesparams)
    * VID_BONESMAX = 3* VID_BONES
    * VID_BONESLAST = VID_BONESMAX - 1

  * vertex.attrib15 = {index0, weight0, index1, weight1}

* Particle general\
no ViewMatrix needed, as particles are in viewspace\
Cg provides particle lightmap texgen matrix directly <br> `float3x4 PRTlmtexgen` <br> semantics: *PRTLMTEXGEN*.
								
* Particle non-instanced mesh
  * program.local[1..32] (ARB_prog) or <br> `float4 PRTvertexoffsets[32]` <br> semantics: *PRTVERTEXOFFSETS*  \
vertex offsets
  * program.local[33..64] (ARB_prog) or <br> `float4 PRTtexoffsets[32]` <br> semantics: *PRTTEXOFFSETS*  \
texture offsets 
  * program.local[0] (ARB_prog) or <br> `float2 PRTtexwidth` <br> semantics: *PRTTEXWIDTH*  \
texturewidth float2(texwidth,1)
  * program.local[65..67] (ARB_prog) or <br> `float3x4 PRTglobalaxis` <br> semantics: *PRTGLOBALAXIS*  \
global axis for aligned particles 
								
  * vertex.tex0 = { Index, Size, texnum, 1 }
  * vertex.attrib15 = { cos, -sin , age/life , speed }

* Particle instanced mesh
  * vertex.attrib9 - 11  = Instance World Matrix44 (float3x4)
  * vertex.attrib12  = {texoffset,relage,speed,0}
  * vertex.attrib13  = {color r,g,b,a}


----
## Commands
After the stage specific commands there is multiple possibilities to change the stage

### `blendmode  <mode enum>;`
to blend fragments on top of each other, enums start with VID_...

Allowed in RenderFlag-,DrawPass-,Texture- and Color-Stages (same for 'blendinvert', 'alphamode' only in Texture).

> RESULT = the fragment produced after blend\
TEX = current fragment, normally a texture but could as well be a plain color\
PREV = the previous fragment drawn (the one "below" TEX)\



* Replacement mode\
just overwrites previous fragment\
RESULT = (TEX)
  * `VID_REPLACE`
* Modulation mode\
multiplies fragments\
RESULT = (PREV) * (TEX) * (VALUE)
  * `VID_MODULATE`\
VALUE = 1
  * `VID_MODULATE2`\
VALUE = 2 (not allowed in texture combiners, use rgbscale instead)
* Decal modes\
A decal is drawn ontop of another surface and replaces the pixel below based on the alpha value used (think of a transparent sticker):\
RESULT = (PREV) * ( 1-ALPHA) + (TEX) * (ALPHA)
  * `VID_DECAL`\
ALPHA = TEX.ALPHA 
  * `VID_DECAL_PREV`\
ALPHA = PREV.ALPHA
  * `VID_DECAL_VERTEX`\
ALPHA = VERTEXCOLOR.ALPHA
  * `VID_DECAL_CONST`\
ALPHA = TEXCONSTANT.ALPHA (provided by texconst)    
* Additive modes\
The fragments are summed together:\
RESULT = (PREV) + (TEX) * FACTOR
  * `VID_ADD`\
FACTOR = 1
  * `VID_AMODADD`\
FACTOR = TEX.ALPHA
  * `VID_AMODADD_PREV`\
FACTOR = PREV.ALPHA
  * `VID_AMODADD_VERTEX`\
FACTOR = VERTEXCOLOR.ALPHA
  * `VID_AMODADD_CONST`\
FACTOR = CONSTANT.ALPHA (provided by texconst)
   
    
* Special modes:
> only in VID_ARB_V and higher techniques:
  * `VID_AMODADD_CONSTMOD`\
RESULT = (PREV) + (TEX) * FACTOR\
FACTOR = CONSTANT.ALPHA * TEX.ALPHA
  * `VID_DECAL_CONSTMOD`\
RESULT = (PREV) * ( 1-ALPHA) + (TEX) * (ALPHA)\
ALPHA = CONSTANT.ALPHA * TEX.ALPHA

* Advanced:
  * internal modes:\
the blendcompiler normally turns the basic modes above and generates other modes so that it uses least passes possible.\
In manual blendsetup mode those might be useful. All combiners are listed in the "vid_texcombiners.txt" file

  * custom modes:\
You can program your own texture combiners with the luxinia api and store them with your custom names (e.g. COMBINER_SPECIAL_ALPHA).\
See texturecombiner class for details

Alternatively you can generate a texture combiner from a string. This is equivalent to using texturecombiner class.\
The string is created like this:\
`VIDTC_<OPCODE>:<ARGSOURCE>.<ARGOPERAND>|...|...`

`OPCODE`
    * REP\
OUT = ARG0
    * ADD\
OUT = ARG0 + ARG1
    * ADDS\
OUT = ARG0 + ARG1 - 0.5
    * MOD\
OUT = ARG0 * ARG1
    * LERP\
OUT = ARG0 * (ARG2) +  ARG1 * (1-ARG2)
* Following need special capabilities
    * SUB\
OUT = ARG0 - ARG1
    * MODADD\
OUT = ARG0 * ARG2 + ARG1
    * MODADDS\
OUT = ARG0 * ARG2 + ARG1 - 0.5
    * COMB4\
OUT = ARG0 * ARG1 + ARG2 * ARG3
    * COMB4S\
OUT = ARG0 * ARG1 + ARG2 * ARG3 - 0.5
    * DOT3\
OUT = ARG0 dotproduct ARG1
    * DOT3A\
OUT = ARG0 dotproduct ARG1, also writes to ALPHA, alphamode is ignored

`ARGSOURCE`
    * TEX current texture
    * VERTEX current vertex color
    * CONST constant texture color of current textureunit
    * PREV previous
* require special capabilities (Technique TEXCOMB)
    * TEX0 texture unit 0
    * TEX1 texture unit 1
    * TEX2 texture unit 2
    * TEX3 texture unit 3

`ARGOPERAND`
    * C color
    * CINV color inverted
    * A alpha
    * AINV alpha inverted

  * `alphamode <mode enum>;`\
controls alpha separate from color blend. Only allowed in manual compiling (newpass mechanism)\
All combiners are listed in the "vid_texcombiners.txt" file. You can use custommodes and "from string" modes as well.\
Be aware that only alpha/inverted alpha is allowed as operand, and no support for DOT3 function.


* Notes:
  * `VID_REPLACE`\
is default when the first blend is not a effect will mostly need multiple passes when drawn
  * `_PREV`\
is not allowed in first stage and will be changed to VID_DECAL

* `blendinvertalpha;`\
this flag inverts the alpha used in AMODADD and DECAL blendmodes
    

### Texture flags
* texture coordinate generation modes
  * `spheremap;`\
turns the texcoord mapping into sphere mapped
  * `screenmap;`\
screen mapped texture generation
  * `reflectmap;`\
reflection mapped texture generation
  * `worldlinmap;`\
world linear mapped texture generation\
planes and coordinates are set with the texgenplane command
  * `objlinmap;`\
object linear mapped texture generation\
planes and coordinates are set with the texgenplane command
  * `normalmap;`\
normal mapped texture generation
  * `skyreflectmap;`\
special mode that uses reflection and skybox tracking, ideally for regular cubemap reflections
  * `lightreflectmapX`; \
X from 0 to 3. special mode that uses reflection and light tracking, mostly for specular cubemap reflections
  * `lightnormalmapX;`\
X from 0 to 3. special mode that uses normal and suntracking, mostly for custom cubemap lighting
  * `sunreflectmap;`\
same as lightreflectmap 0;
  * `sunnormalmap;`\
same as lightnormalmap 0;

* `nomipmap;`\
will load texture without mipmapping.

* `vertexcolored;`\
the current texture will be modulated with vertex colors (when first stage is a color stage set to REPLACE, and the second stage is texture stage set to MODULATE, then it is the same as texture stage set to REPLACE with "vertexcolored" on) auto-set when VTEX is used

* `rgbscale2;`\
result of texture blending operation is scaled by 2 (should only be used in newpass mechanism)
* `rgbscale4;`\
result of texture blending operation is scaled by 2 (should only be used in newpass mechanism)
* `alphascale2;`\
result of texture blending operation is scaled by 2 (should only be used in newpass mechanism)
* `alphascale4;`\
result of texture blending operation is scaled by 4 (should only be used in newpass mechanism)
* `lightmapscale;`\
when LIGHTMAP is used we will use the lightmap texture's lightmapscale property.

* `depthcompare;`\
texture must be a depthtexture, else errors will happen. Resulting pixel depends on R coordinate comparison with depthvalue.
* `depthvalue;`\
texture must be a depthtexture, else errors will happen. Resulting pixel value of the depthtexture.

### Texture functions
* `texcoord <uint channel>;`\
`channel` 0 = default (2D), -1 = disables passing texcoords. Depending on vertex-type additional channels may exist.
  * vertex16color\
has no texture channels at all
  * vertex32lit\
has only channel 0
  * vertex32lm
    * 1 = (auto used for LIGHTMAP) (2D)
    * `>`1 = is 0 & 1 combined (4D)
  * vertex64
    * 1 = (auto used for LIGHTMAP) (2D)
    * 2 = (2D)
    * 3 = (2D)
    * 4 = 0 & 1 combined (4D)
    * 5 = 2 & 3 combined (4D)
* `texclamp (<X bool>,<Y bool>,<Z bool>);`\
texture clamping (to edge) enabled for given axis. By default all are off and therefore set to repeat.\
Only applied to new loaded textures
* `texgenplane <axis uint> <vector4 plane>`\
sets the texgen plane for each axis (0 = X, 1 = Y,...) for world/objlinmap.\
The planes defined in current used material supercede the shader values, but not which planes are used.\
Be aware that only following axis combinations are legal:
  * XY,XYW,XYZ,XYZW
* `param...;`\
see functions below at GpuProgram, only VID_VALUE, VID_LIGHTCOLOR, VID_LIGHTDIR, VID_CAMDIR are supported and passed as TEXCONST

### GpuProgram flags
* `tangents;`\
means the program needs tangents as pervertex attrib\
tangents are stored in vertex.attrib14 (Cg :TANGENT semantic)
* `normals;`\
means the program needs always normals as pervertex attrib
otherwise normals are only enabled if lighting is active.
* `texbarrier;`\
clears caches for custom blending (reading and writing texture
at same time). Only allowed in Cg mode and requires
capability.texbarrier, otherwise ignored.
 
### GpuProgram functions:
* `param  "<name>" <uint id> <enum type> (<x float>,<y float>,<z float>,<w float>) [<uint upvalueA>] [<uint upvalueB>];`\
A Parameter that can be controlled via material or directly with the shader. Be aware that for Cg having the same parameter \
names in multiple programs within the same pass is not recommended.\
For greater variety in parameters (integer,matrices..) and efficient control on per-frame basis, you can also use the `shadercghostparam` class inside luxinia.api.
  * `name`\
variable name
  * `id`\
id number for ARB_PROGRAMs, ignored for rest.
  * `type`\
what type of variable 
    * `VID_VALUE` generic value, ie the 4 floats the user passes. If upvalueA == 1 then vector will be turned into localspace.
    * `VID_ARRAY` a float4 array, upvalueA is length of array and must be given.
    * `VID_STREAM` a custom vertex attrib stream. id serves as generic vertex.attrib id (in Cg that value is retrieved via name, e.g "InputStruct.someattr"). Streams are set with the moStream matobject interface call.
    * `VID_CAMPOS` camera position, in localspace
    * `VID_CAMDIR` camera view direction, in localspace
    * `VID_CAMUP` camera up vector, in localspace
    * `VID_LIGHTPOS` upvalueA(th) light position (0..3) in localspace,
    * `VID_LIGHTDIR` light direction in localspace,
    * `VID_LIGHTCOLOR` light diffuse color
    * `VID_LIGHTAMBIENT` light ambient color
    * `VID_LIGHTRANGE` (range,1/range,range/2,1/range*range)
    * `VID_RANDOM` result is (rand1*range,rand2*range,rand3*range,range)\
rand are random numbers [0,1] that are consistent with a drawnode\
`range` is given by user `<w>`
    * `VID_TIME` time values result is(time*scale,sin(time*scale),cos(time*scale),scale)\
`time` is in milliseconds,`scale` is given by user `<w>`
    * `VID_TEXCONST` texture constant id, upvalueA from 0 to 7. Stage's color as in material is used.
    * `VID_TEXMAT0` texture matrix row 0. upvalueA from 0 to 7 stage as defined in material. upvalueB == 1 worldmatrix 2 shared texmatrix 3 shared lightmapmatrix
    * `VID_TEXMAT1` texture matrix row 1. see above.
    * `VID_TEXMAT2` texture matrix row 2. see above.
    * `VID_TEXMAT3` texture matrix row 3. see above.
    * `VID_TEXGEN0` texturecoord generator row 0. upvalueA from 0 to 7 stage as defined in material. upvalueB == 1 worldmatrix 2 shared texmatrix 3 shared lightmapmatrix.
    * `VID_TEXGEN1` texturecoord generator row 1. see above.
    * `VID_TEXGEN2` texturecoord generator row 2. see above.
    * `VID_TEXGEN3` texturecoord generator row 3. see above.
    * `VID_TEXSIZE` size of a texture in pixels, use `<w>` as scale factor. Output is (width*scale,height*scale,depth*scale,scale). upvalueA is texture unit, cubemaps have depth of 6
    * `VID_TEXSIZEINV` 1/(scaled size of a texture in pixels), upvalueA is texture unit
    * `VID_TEXLMSCALE` lightmap rgb scale value of a texture (1,2 or 4), upvalueA is texture unit

  * `(x,y,z,w)`\
the vector, will be overwritten when type is not VID_VALUE\
some special types will keep the `<w float>` as function value
  * `upvalueA`
  * `upvalueB`\
used as parameter for some types, optional.

### DrawPass flags:
* `colorpass;`\
renders without setting a new color untextured the model again.
### DrawPass functions:
* `stateflag <enum type> <bool onoff>;`\
Overrides active state (will supercede "force/ignoreflags" of active at layer). Some states are implicitly set.
    * `type`\
what type state renderflag is changed.
    * `RENDER_BLEND` framebuffer blending. (imp. blendmode)
    * `RENDER_NOVERTEXCOLOR` per-vertex color is ignored.
    * `RENDER_ALPHATEST` pixels can be rejcted by color alpha (imp. alphafunc)
    * `RENDER_STENCILTEST` pixels can be rejcted by stencil-buffer
    * `RENDER_NODEPTHTEST` pixels cannot be rejcted by z-buffer
    * `RENDER_NOCULL` back- and frontfaces are drawn
    * `RENDER_FRONTCULL` faces are not culled
    * `RENDER_NOCOLORMASK` disables writing to color buffer
    * `RENDER_NODEPTHMASK` disables wiriting to depth buffer
    * `RENDER_STENCILMASK` enables writing to stencil buffer
    * `RENDER_LIT` enables fixedfunction lighting (lightcount which picks gpuprogram remains unchanged).
  * `onoff`\
1 enforces setting this state\
0 disables/reverts the state if active.



----
## History

- **1st Draft 17.4.2004:**  \
This is just a basic idea about functionality. Shaders may change on implentation
if too complex to a more rigid system with a hardcoded set of FX

- **2nd Draft 28.4.2004:**  \
moved stageflags to renderflags, moved alpha/blend to renderflag as well, this
is easier to work with in OGL, but needs a new system for texture blending
which needs to be done soon (mainly combine)

- **3rd Draft 1.5.2004:**  \
texture blending and combining is now set.

- **4th Draft 10.5.2004:**  \
added a new custom texmode called VID_COLORDECAL, removed GL_SUBTRACT cause it brings us
in trouble with older systems. CBF to do workaround.

- **5th Draft 12.5.2004:**  \
okay texcombine is removed, instead there are fixed effect types
possible for the combining of texlayers. depending on hardware
I will create the proper texcombiners/multipass renders.

- **6th Draft 2.6.2004:**  \
it will stay as is, although multiple textures per shader may not work well
on all hardware until a "brute force" rendering pipeline is part of the engine.

- **7th Draft 7.6.2004:**  \
added reflectmap,eyelinmap as texture coord generation mode

- **8th Draft 31.7.2004:**  \
blendfunc is not valid for all passes just the first pass (first tex + color)
also added LITMODADD and ADDMOD modes

- **9th Draft 13.8.2004:**  \
added LITDECAL_VERTEX and DECAL_VERTEX modes

- **10th Draft 30.8.2004:**  \
added BUMP mode

- **11th Draft 2.8.2004:**  \
added SPECBUMP mode

- **12th Draft 27.9.2004:**  \
changed BUMP to NORMAL and SPECBUMP to SPECNORMAL

- **13th Draft 15.10.2004:**  \
LIT renamed to COLOR

- **14th Draft 18.10.2004:**  \
ALPHA as texture mode to upload greyscales
added nocolorarray renderflag

- **15th Draft 20.10.2004:**  \
blendfunc taken out, heavy reorganisation of blends now part of every stage as blendmode

- **16th Draft 1.11.2004:**  \
added LowDetail stage 

- **17th Draft 6.12.2004:**  \
addded GPUProgram and FXShader stage, the latter is not implemented at all
and the first doesnt allow DXPSA yet

- **18th Draft 10.12.2004:**  \
layer renderflag added, to allow user to mark certain surfaces to be rendered
at certain times. allows Zsort, Glow, Haze, Top layer (thats also the order in which they are drawn)
removed DXPSA gpuprog type

- **19th Draft 19.12.2004:**  \
modifers need a name to be accessible from gamecode

- **20th Draft 22.12.2004:**  \
added techniques and shaders "values" need to be put in () vectors separated by ,
special Textures allowed in higher techniques (skybox, normalize...)
gpuprogram parameters will soon allow special values (campos, camdir..) to be auto linked with

- **21th Draft 29.12.2004:**  \
name is a unique identifier of the stage to allow easier access to stages from gamescript
LIGHTMAP added

- **22nd Draft 23.2.2005:**  \
split shader into "shader/material"
so that shader only contains info how textures are blend

- **23rd Draft 15.3.2005:**  \
added "skin" versions for vertex programs

- **24th Draft 31.3.2005:**  \
added "texcoord" to allow use of different texcoord channels

- **25th Draft 16.7.2005:**  \
added "lit" versions for gpu programs and entrynames

- **26th Draft 23.7.2005:**  \
added new Techniques for better hardware, also added new automatic shader parameters

- **27th Draft 6.1.2006:**  \
the NewPass Stage is introduced to allow manual 'compiling'

- **28th Draft 27.5.2006:**  \
added .dds support

- **29th Draft 20.6.2006:**  \
sunreflectmap and sunnormalmap\
added TEXDOTZ for special cubemaps

- **30th Draft 2.7.2006:**  \
lit renamed to sunlit\
added alphaTEX

- **31st Draft 28.7.2006:**  \
texture clamping and texgenplane added.\
eyelinmap renamed to worldlinmap, objlinmap added as well.

- **32nd Draft 1.8.2006:**  \
all combiners (alpha/color) are now accessible. alphamode was added\

- **33rd Draft 10.8.2006:**  \
added "preprocessor" branching

- **34th Draft 2.9.2006:**  \
param for regular stages

- **35th Draft 25.12.2006:**  \
param for regular stages got support for lightcolor/lightdir

- **36th Draft 31.1.2007:**  \
normals; flag also added to RenderFlag

- **37th Draft 17.3.2007:**  \
added VIDTC texture combiner from string generation

- **38th Draft 6.4.2007:**  \
shader parameters got an upvalue. Warning this renders some of the old VID_BLAHx versions illegal.

- **39th Draft 22.4.2007:**  \
lightnormalmapX & lightreflectmapX added.

- **40th Draft 2.6.2007:**  \
texchannel -1 disables passing texcoords

- **41th Draft 18.6.2007:**  \
depthcompare and depthvalue flags added

- **42th Draft 21.6.2007:**  \
compiled flag for cg programs

- **43th Draft 8.9.2007:**  \
added shader parameter arrays

- **44th Draft 14.10.2007:**  \
'nomipmap' added

- **45th Draft 30.10.2007:**  \
'VID_MODULATE2' added

- **46th Draft 26.12.2007:**  \
Cg 2.0 changes, version raised to 310, mandatory for Cg useage\
`VID_FOGCOLOR` gpu param removed, FFIXED gpu type removed.\
"|compiled" removed is done differently now\
VID_CG Technique removed, Cg auto-parameter names for Particles changed.\
VID_TEXSIZE, VID_TEXSIZEINV,VID_TEXLMSCALE as parameters and lowCgProfile added\
shadercghostparam class in Luxinia api

- **47th Draft 24.1.2008:**  \
proper block commenting with # { ... # }

- **48th Draft 23.4.2008:**  \
commenting now in C-Style, along with raised version number 310 now mandatory for all
// and /* ... */
renamed VID_NORMALMAP to VID_NORMALMAPTAN

- **49th Draft 18.5.2008:**  \
annotation system added

- **50th Draft 11.5.2008:**  \
DUMMY texture added,
texchannel 4 = 0&1 combined, 5 = 2&3 combined

- **51st Draft 24.9.2008:**  \
renamed NewPass to DrawPass (old name remains backward compatible)
added stateflag command to DrawPass,
use of gpuprograms always disables blend-compiler
added blendmode to RenderFlag
RenderFlag is now a full "default copy" in luxinia, no longer does
alphafunc in RenderFlag "override". However DrawPass states do.
alphafunc removed from DrawPass

- **52nd Draft 15.11.2008:**  \
VID_CG_SM3_TEX8 renamed to VID_CG_SM3, and VID_CG_SM4/VID_CG_SM4_GS added

- **53nd Draft 23.02.2009:**  \
Cg Semantics added

- **54th Draft 23.08.2009:**  \
removed VID_NORMALMAPTAN blendmode


#ifndef _VISUAL_MANAGER_GLSL_
#define _VISUAL_MANAGER_GLSL_

#define VERTEX_CONTENT \
"/**\n" \
" * Shader implementing: Clipping, Texturing (RGB and YUV), Lighting, Fog\n" \
" **/\n" \
"\n" \
"//version defined first, at shader compilation\n" \
"\n" \
"#if defined(GL_ES)\n" \
"#if defined(GL_FRAGMENT_PRECISION_HIGH)\n" \
"precision highp float;\t//ES2.0 supporting highp\n" \
"#else\n" \
"precision mediump float;\t//Default\n" \
"#endif\n" \
"#endif\n" \
"\n" \
"//#pragma STDGL invariant(all)\t//removed due to incompatibility with the emulator\n" \
"\n" \
"//LIGHTS_MAX and CLIP_MAX defined at shader compilation\n" \
"\n" \
"\n" \
"//lights definitions\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"\n" \
"#define FOG_TYPE_LINEAR 0\n" \
"#define FOG_TYPE_EXP    1\n" \
"#define FOG_TYPE_EXP2   2\n" \
"\n" \
"#define L_DIRECTIONAL\t0\n" \
"#define L_SPOT\t    1\n" \
"#define L_POINT\t\t2\n" \
"\n" \
"\n" \
"//Light Structure\n" \
"struct gfLight{\n" \
"#if defined(GL_ES)\n" \
"\tlowp int type;\n" \
"#else\n" \
"\tint type;\n" \
"#endif\n" \
"\tvec4 position;\n" \
"\tvec4 direction;\n" \
"\tvec3 attenuation;\n" \
"\tvec4 color;\n" \
"\tfloat ambientIntensity;\t//it is not used - we calculate it inside the shader\n" \
"\tfloat intensity;\n" \
"\tfloat beamWidth;\t//it is not used - we calculate it inside the shader\n" \
"\tfloat cutOffAngle;\n" \
"};\n" \
"#endif\n" \
"\n" \
"\n" \
"//Attributes\n" \
"attribute vec4 gfVertex;\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"attribute vec3 gfNormal;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_TEXTURE\n" \
"attribute vec4 gfMultiTexCoord;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_COLOR\n" \
"attribute vec4 gfMeshColor;\n" \
"#endif\n" \
"\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"//Generic (Scene) uniforms\n" \
"uniform gfLight lights[LIGHTS_MAX];\n" \
"#if defined(GL_ES)\n" \
"uniform lowp int gfNumLights;\n" \
"#else\n" \
"uniform int gfNumLights;\n" \
"#endif\n" \
"\n" \
"//Fog\n" \
"uniform bool gfFogEnabled;\n" \
"uniform vec3 gfFogColor;\n" \
"uniform float gfFogDensity;\n" \
"uniform int gfFogType;\n" \
"uniform float gfFogVisibility;\n" \
"\n" \
"#endif\n" \
"\n" \
"//Matrices\n" \
"uniform mat4 gfModelViewMatrix;\n" \
"uniform mat4 gfProjectionMatrix;\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"uniform mat4 gfNormalMatrix;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_TEXTURE\n" \
"uniform mat4 gfTextureMatrix;\n" \
"uniform bool hasTextureMatrix;\n" \
"#endif\n" \
"\n" \
"//Clipping\n" \
"#ifdef GF_GL_HAS_CLIP\n" \
"#if defined(GL_ES)\n" \
"uniform lowp int gfNumClippers;\n" \
"#else\n" \
"uniform int gfNumClippers;\n" \
"#endif\n" \
"uniform vec4 clipPlane[CLIPS_MAX];\n" \
"#endif\n" \
"\n" \
"//Varyings\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"\n" \
"varying vec4 gfEye;\t//camera\n" \
"\n" \
"varying vec3 m_normal;\t\t//normal\n" \
"varying vec3 lightVector[LIGHTS_MAX];\n" \
"varying float gfFogFactor;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_TEXTURE\n" \
"varying vec2 TexCoord;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_COLOR\n" \
"varying vec4 m_color;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_CLIP\n" \
"varying float clipDistance[CLIPS_MAX];\n" \
"#endif\n" \
"\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"float fog()\n" \
"{\n" \
"\tfloat fog, eyeDist = length(gfEye-gfVertex);\n" \
"\n" \
"\tif(gfFogType==FOG_TYPE_LINEAR){\n" \
"\t\tfog= (gfFogVisibility-eyeDist)/gfFogVisibility;\n" \
"\t}else if(gfFogType==FOG_TYPE_EXP){\n" \
"\t\tfog= exp(-(gfEye.z * gfFogDensity));\n" \
"\t}else if(gfFogType==FOG_TYPE_EXP2){\n" \
"\t\tfog= (gfEye.z * gfFogDensity);\n" \
"\t\tfog = exp(-(fog * fog));\n" \
"\t}\n" \
"\treturn clamp(fog, 0.0, 1.0);\n" \
"}\n" \
"#endif\n" \
"\n" \
"void main(void)\n" \
"{\n" \
"#ifndef GF_GL_HAS_LIGHT\n" \
"\tvec4 gfEye;\n" \
"#endif\n" \
"\n" \
"\tgfEye = gfModelViewMatrix * gfVertex;\n" \
"\n" \
"#ifdef GF_GL_HAS_COLOR\n" \
"\tm_color = gfMeshColor;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"\tm_normal = normalize( vec3(gfNormalMatrix * vec4(gfNormal, 0.0)) );\n" \
"\n" \
"\tfor(int i=0; i<LIGHTS_MAX; i++){\n" \
"\t\tif (i==gfNumLights) break;\n" \
"\n" \
"\t\tif ( lights[i].type == L_SPOT || lights[i].type == L_POINT ) {\n" \
"\t\t\tlightVector[i] = lights[i].position.xyz - gfEye.xyz;\n" \
"\t\t} else {\n" \
"\t\t\tlightVector[i] = lights[i].direction.xyz;\n" \
"\t\t}\n" \
"\t}\n" \
"\tgfFogFactor = gfFogEnabled ? fog() : 1.0;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_TEXTURE\n" \
"\tif (hasTextureMatrix) {\n" \
"\t\tTexCoord = vec2(gfTextureMatrix * gfMultiTexCoord);\n" \
"\t} else {\n" \
"\t\tTexCoord = vec2(gfMultiTexCoord);\n" \
"\t}\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_CLIP\n" \
"\tfor (int i=0; i<CLIPS_MAX; i++) {\n" \
"\t\tif (i==gfNumClippers) break;\n" \
"\t\tclipDistance[i] = dot(gfEye.xyz, clipPlane[i].xyz) + clipPlane[i].w;\n" \
"\t}\n" \
"#endif\n" \
"\n" \
"\tgl_Position = gfProjectionMatrix * gfEye;\n" \
"}\n"

#define FRAGMENT_CONTENT \
"/**\n" \
" * Shader implementing: Clipping, Texturing (RGB and YUV), Lighting, Fog\n" \
" **/\n" \
"\n" \
"//version defined first, at shader compilation\n" \
"#ifdef GF_GL_IS_ExternalOES\n" \
"#extension GL_OES_EGL_image_external : require\n" \
"#endif\n" \
"#if defined(GL_ES)\n" \
"#if defined(GL_FRAGMENT_PRECISION_HIGH)\n" \
"precision highp float;	//ES2.0 supporting highp\n" \
"#else\n" \
"precision mediump float;	//Default\n" \
"#endif\n" \
"#endif\n" \
"\n" \
"//#pragma STDGL invariant(all)	//removed due to incompatibility with the emulator\n" \
"\n" \
"//LIGHTS_MAX and CLIP_MAX defined at shader compilation\n" \
"\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"\n" \
"#define FOG_TYPE_LINEAR 0\n" \
"#define FOG_TYPE_EXP    1\n" \
"#define FOG_TYPE_EXP2   2\n" \
"\n" \
"#define L_DIRECTIONAL	0\n" \
"#define L_SPOT		    1\n" \
"#define L_POINT			2\n" \
"\n" \
"//Light Structure\n" \
"struct gfLight{\n" \
"#if defined(GL_ES)\n" \
"	lowp int type;\n" \
"#else\n" \
"	int type;\n" \
"#endif\n" \
"	vec4 position;\n" \
"	vec4 direction;\n" \
"	vec3 attenuation;\n" \
"	vec4 color;\n" \
"	float ambientIntensity;	//it is not used - we calculate it inside the shader\n" \
"	float intensity;\n" \
"	float beamWidth;	//it is not used - we calculate it inside the shader\n" \
"	float cutOffAngle;\n" \
"};\n" \
"\n" \
"//Generic (Scene) Uniforms\n" \
"#if defined(GL_ES)\n" \
"uniform lowp int gfNumLights;\n" \
"#else\n" \
"uniform int gfNumLights;\n" \
"#endif\n" \
"uniform bool gfLightTwoSide;\n" \
"uniform gfLight lights[LIGHTS_MAX];\n" \
"\n" \
"//Material and Lighting Properties\n" \
"uniform vec4 gfAmbientColor;\n" \
"uniform vec4 gfDiffuseColor;\n" \
"uniform vec4 gfSpecularColor;\n" \
"uniform float gfShininess;	//a.k.a. specular exponent\n" \
"uniform vec4 gfLightDiffuse;\n" \
"uniform vec4 gfLightAmbient;\n" \
"uniform vec4 gfLightSpecular;\n" \
"\n" \
"//Fog\n" \
"uniform bool gfFogEnabled;\n" \
"uniform vec3 gfFogColor;\n" \
"\n" \
"#endif\n" \
"\n" \
"uniform bool hasMaterial2D;\n" \
"uniform vec4 gfEmissionColor;\n" \
"\n" \
"#ifdef GF_GL_HAS_COLOR\n" \
"uniform bool hasMeshColor;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_CLIP\n" \
"#if defined(GL_ES)\n" \
"uniform lowp int gfNumClippers;\n" \
"#else\n" \
"uniform int gfNumClippers;\n" \
"#endif\n" \
"#endif\n" \
"\n" \
"//Color Matrix\n" \
"uniform mat4 gfColorMatrix;\n" \
"uniform bool hasColorMatrix;\n" \
"uniform vec4 gfTranslationVector;\n" \
"	\n" \
"//Color Key\n" \
"uniform vec3 gfKeyColor;\n" \
"uniform float gfKeyAlpha;\n" \
"uniform float gfKeyLow;\n" \
"uniform float gfKeyHigh;\n" \
"uniform bool hasColorKey;\n" \
"\n" \
"//Varyings\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"varying vec3 m_normal;\n" \
"varying vec4 gfEye;\n" \
"varying vec3 lightVector[LIGHTS_MAX];\n" \
"varying float gfFogFactor;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_TEXTURE\n" \
"varying vec2 TexCoord;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_COLOR\n" \
"varying vec4 m_color;\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_CLIP\n" \
"varying float clipDistance[CLIPS_MAX];\n" \
"#endif\n" \
"\n" \
"//constants\n" \
"const float zero_float = 0.0;\n" \
"const float one_float = 1.0;\n" \
"\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"vec4 doLighting(int i){\n" \
"\n" \
"	vec4 lightColor = vec4(zero_float, zero_float, zero_float, zero_float);\n" \
"	float att = zero_float;\n" \
"	gfLight tempLight;	//we use a temp gfLight, because array of straucts in fragment are not supported in android\n" \
"	vec3 lightVnorm; //normalized lightVector\n" \
"	vec3 lightV; //temp lightVector\n" \
"	vec3 halfVnorm; //temp lightVector\n" \
"\n" \
"	//FIXME - couldn't we use a light[i]. something here ?\n" \
"	if(i==0) {	//ES2 does not support switch() statements\n" \
"		tempLight = lights[0];\n" \
"		lightV = lightVector[0];\n" \
"		halfVnorm = normalize( lightVector[0] + gfEye.xyz );\n" \
"	} else if (i==1) {\n" \
"		tempLight = lights[1];\n" \
"		lightV = lightVector[1];\n" \
"		halfVnorm = normalize( lightVector[1] + gfEye.xyz );\n" \
"	} else if(i==2) {\n" \
"		tempLight = lights[2];\n" \
"		lightV = lightVector[2];\n" \
"		halfVnorm = normalize( lightVector[2] + gfEye.xyz );\n" \
"	}\n" \
"	\n" \
"	lightVnorm = normalize(lightV);\n" \
"	vec3 normal = normalize(m_normal);\n" \
"\n" \
"	if (gfLightTwoSide && (!gl_FrontFacing)){//light back face\n" \
"		//originally: normal *=-1; -> Not compliant with Shading Language v1.0\n" \
"		normal *= vec3(-1.0, -1.0, -1.0);\n" \
"	}\n" \
"	\n" \
"	float light_cos = max(zero_float, dot(normal, lightVnorm));	//ndotl\n" \
"	float half_cos = dot(normal, halfVnorm);\n" \
"\n" \
"	if (tempLight.type == L_POINT) {	//we have a point\n" \
"		float distance = length(lightV);	\n" \
"		att = 1.0 / (tempLight.attenuation.x + tempLight.attenuation.y * distance + tempLight.attenuation.z * distance * distance);\n" \
"\n" \
"		if (att <= 0.0)\n" \
"			return lightColor;\n" \
"			\n" \
"		lightColor += light_cos * tempLight.color * gfDiffuseColor;\n" \
"		\n" \
"		if (light_cos > 0.0) {\n" \
"			float dotNormHalf = max(dot(normal, halfVnorm),0.0);	//ndoth\n" \
"			lightColor += (pow(dotNormHalf, gfShininess) * gfSpecularColor * tempLight.color);\n" \
"			lightColor *= att;\n" \
"		}\n" \
"		lightColor.a = gfDiffuseColor.a;\n" \
"		return lightColor;\n" \
"		\n" \
"	} else if (tempLight.type == L_SPOT) {	//we have a spot\n" \
"		if (light_cos > 0.0) {\n" \
"			float spot = dot(normalize(tempLight.direction.xyz), lightVnorm);	//it should be -direction, but we invert it before parsing\n" \
"			if (spot > tempLight.cutOffAngle) {\n" \
"				float distance = length(lightV);	\n" \
"				float dotNormHalf = max(dot(normal, halfVnorm),0.0);	//ndoth\n" \
"				spot = pow(spot, tempLight.intensity);\n" \
"				att = spot / (tempLight.attenuation.x + tempLight.attenuation.y * distance + tempLight.attenuation.z * distance * distance);\n" \
"				lightColor += att * (light_cos * tempLight.color * gfDiffuseColor);\n" \
"				lightColor += att * (pow(dotNormHalf, gfShininess) * gfSpecularColor * tempLight.color);\n" \
"			}\n" \
"		}\n" \
"		return lightColor;\n" \
"\n" \
"	} else if(tempLight.position.w == zero_float || tempLight.type == L_DIRECTIONAL) { //we have a direction\n" \
"		vec3 lightDirection = vec3(tempLight.position);\n" \
"		lightColor = (gfDiffuseColor * gfLightDiffuse) * light_cos; \n" \
"		if (half_cos > zero_float) {\n" \
"			lightColor += (gfSpecularColor * gfLightSpecular) * pow(half_cos, gfShininess);\n" \
"		}\n" \
"		lightColor.a = gfDiffuseColor.a;\n" \
"		return lightColor;\n" \
"	}\n" \
"\n" \
"	return vec4(zero_float);\n" \
"}\n" \
"#endif //GF_GL_HAS_LIGHT\n" \
"\n" \
"void main()\n" \
"{\n" \
"#if defined(GF_GL_HAS_CLIP) && defined(GL_ES)\n" \
"	bool do_clip=false;\n" \
"#endif\n" \
"	int i;\n" \
"	vec2 texc;\n" \
"	vec3 yuv, rgb;\n" \
"	vec4 rgba, fragColor;\n" \
"\n" \
"#ifdef GF_GL_HAS_CLIP\n" \
"	//clipping\n" \
"	for (int i=0; i<CLIPS_MAX; i++) {\n" \
"		if (i==gfNumClippers) break;\n" \
"		if (clipDistance[i]<0.0) {\n" \
"			//do not discard on GLES too slow on most devices\n" \
"#if defined(GL_ES)\n" \
"			do_clip=true;\n" \
"			break;\n" \
"#else\n" \
"			discard;\n" \
"#endif\n" \
"		}\n" \
"	}\n" \
"#endif\n" \
"	\n" \
"#if defined(GF_GL_HAS_CLIP) && defined(GL_ES)\n" \
"	if (do_clip) {\n" \
"		gl_FragColor = vec4(zero_float);\n" \
"	} else {\n" \
"#endif\n" \
"		\n" \
"		\n" \
"#ifdef GF_GL_HAS_COLOR\n" \
"	fragColor = m_color;\n" \
"#else\n" \
"	fragColor = vec4(zero_float);\n" \
"#endif\n" \
"\n" \
"	if (hasMaterial2D) {\n" \
"		fragColor = gfEmissionColor;\n" \
"	}\n" \
"\n" \
"#if defined (GF_GL_HAS_LIGHT)\n" \
"	if (gfNumLights>0) {\n" \
"#ifdef GF_GL_HAS_COLOR\n" \
"		fragColor += (gfAmbientColor * gfLightAmbient);\n" \
"#else\n" \
"		fragColor = gfEmissionColor + (gfAmbientColor * gfLightAmbient);\n" \
"#endif\n" \
"	}\n" \
"#endif\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"	if (gfNumLights > 0) {\n" \
"		for (int i=0; i<LIGHTS_MAX; i++) {\n" \
"			if (i==gfNumLights) break;\n" \
"			fragColor += doLighting(i);\n" \
"		}\n" \
"		fragColor.a = gfDiffuseColor.a;\n" \
"	}\n" \
"#endif\n" \
"	\n" \
"	fragColor = clamp(fragColor, zero_float, one_float);\n" \
"\n" \
"#ifdef GF_GL_HAS_TEXTURE\n" \
"	\n" \
"	//currently supporting 1 texture\n" \
"	rgba = maintx_sample(TexCoord);\n" \
"\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"	if (gfNumLights>0) {	//RGB texture\n" \
"		fragColor *= rgba;\n" \
"	}\n" \
"	//RGB texture with material 2D [TODO: check]\n" \
"	else if(gfNumLights==0)\n" \
"#endif\n" \
"	{\n" \
"		fragColor = rgba;\n" \
"	}\n" \
"\n" \
"		//we have mat 2D + texture\n" \
"#ifndef GF_GL_IS_ExternalOES\n" \
"	if (hasMaterial2D) {\n" \
"		if (gfEmissionColor.a > 0.0) {\n" \
"			fragColor *= gfEmissionColor;\n" \
"		}\n" \
"		//hack - if full transparency on texture with material2D, use material color\n" \
"		else if (fragColor.rgb == vec3(0.0, 0.0, 0.0)){\n" \
"			fragColor.rgb = gfEmissionColor.rgb;\n" \
"		}\n" \
"	}\n" \
"#endif // GF_GL_IS_ExternalOES\n" \
"\n" \
"	\n" \
"#endif // GF_GL_HAS_TEXTURE\n" \
"	\n" \
"	\n" \
"	if (hasColorMatrix) {\n" \
"		fragColor = gfColorMatrix * fragColor;\n" \
"		fragColor += gfTranslationVector;\n" \
"		fragColor = clamp(fragColor, zero_float, one_float);\n" \
"	}\n" \
"	\n" \
"	if (hasColorKey) {\n" \
"		vec3 tempColour = vec3(0.0, 0.0, 0.0);\n" \
"		float mean = 0.0;\n" \
"		\n" \
"		tempColour.r = abs(gfKeyColor.r-fragColor.r);\n" \
"		tempColour.g = abs(gfKeyColor.g-fragColor.g);\n" \
"		tempColour.b = abs(gfKeyColor.b-fragColor.b);\n" \
"		mean = (tempColour.r + tempColour.g + tempColour.b)/3.0;\n" \
"		\n" \
"		if (mean<gfKeyLow) {\n" \
"			fragColor.a =0.0;\n" \
"		} else if(mean<=gfKeyHigh) {\n" \
"			fragColor.a = (mean-gfKeyLow) * gfKeyAlpha / (gfKeyHigh - gfKeyLow);\n" \
"		}\n" \
"	}\n" \
"	\n" \
"#ifdef GF_GL_HAS_LIGHT\n" \
"	if (gfFogEnabled)\n" \
"		fragColor = fragColor * gfFogFactor + vec4(gfFogColor, zero_float) * (one_float - gfFogFactor);\n" \
"#endif\n" \
"	gl_FragColor = fragColor;\n" \
"\n" \
"#if defined(GF_GL_HAS_CLIP) && defined(GL_ES)\n" \
"	}\n" \
"#endif\n" \
"}\n"

#endif

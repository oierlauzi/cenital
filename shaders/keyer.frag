#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "color_utils.glsl"
#include "frame.glsl"
#include "bezier.glsl"

struct LumaKeyParameters {
	float minThreshHold;
	float maxThreshHold;
};

struct ChromaKeyParameters {
	float hue;
	float deltaHueThreshold;
	float deltaHueSmoothness;
	float saturationThreshold;
	float saturationSmoothness;
	float valueThreshold;
	float valueSmoothness;
};

//Specialization constants and normal constants
const int LINEAR_KEY_DISABLED 	= 0x00;
const int LINEAR_KEY_KEY_R 		= 0x01;
const int LINEAR_KEY_KEY_G 		= 0x02;
const int LINEAR_KEY_KEY_B 		= 0x03;
const int LINEAR_KEY_KEY_A 		= 0x04;
const int LINEAR_KEY_KEY_Y 		= 0x05;
const int LINEAR_KEY_FILL_R 	= 0x06;
const int LINEAR_KEY_FILL_G 	= 0x07;
const int LINEAR_KEY_FILL_B 	= 0x08;
const int LINEAR_KEY_FILL_A 	= 0x09;
const int LINEAR_KEY_FILL_Y 	= 0x0A;

layout(constant_id = 0) const int sampleMode = frame_SAMPLE_MODE_PASSTHOUGH;
layout(constant_id = 1) const bool sameKeyFill = false;
layout(constant_id = 2) const bool lumaKeyEnabled = false;
layout(constant_id = 3) const bool chromaKeyEnabled = false;
layout(constant_id = 4) const int linearKeyType = 0;

//Vertex I/O
layout(location = 0) in vec2 in_texCoord;
layout(location = 1) in vec3 in_klm;

layout(location = 0) out vec4 out_color;

//Uniform buffers
layout(set = 1, binding = 1) uniform LayerDataBlock {
	LumaKeyParameters 		lumaKeyparameters;
	ChromaKeyParameters 	chromaKeyparameters;
	float 					opacity;
};

//Frame descriptor sets
frame_descriptor_set(2)
frame_descriptor_set(3)



float lumaKeyAlpha(in LumaKeyParameters parameters, in vec3 keyColor) {
	//Calculate the luminance according to the YUV color system
	const float luminance = getLuminance(keyColor);

	//Determine if the values are inverted
	const bool inverted = parameters.minThreshHold < 0.0f;

	const float minThreshHold = abs(parameters.minThreshHold);
	const float maxThreshHold = abs(parameters.maxThreshHold);

	//Only use smoothstep if its behaviour is defined
	const float alpha = parameters.minThreshHold < parameters.maxThreshHold ?
		smoothstep(minThreshHold, maxThreshHold, luminance) :
		step(minThreshHold, luminance) ;

	//Invert the result if necessary
	return inverted ? 1.0 - alpha : alpha;
}

//Chroma keying is based on:
//Software Chroma Keying in an Imersive Virtual Environment, F. van den Bergh & V. Lalioti
//https://github.com/CasparCG/server/blob/master/src/accelerator/ogl/image/shader.frag

//Returns the absolute distance between two angles.
//Half specifies the units of the inputs, so that half
//equals a half turn (pi rad, 180 deg, 0.5 turns...).
//Inputs must be less or equal than one turn appart.
//At most, the distance between to angles will be "half"
float angleDistance(float a, float b, float halfTurn) {
	return halfTurn - abs(abs(b - a) - halfTurn);
}

float chromaKeyAlpha(in ChromaKeyParameters parameters, in vec3 keyColor) {
	//Convert the color into hsv
	const vec3 hsvColor = rgb2hsv(keyColor);

	//Calculate the deltas and the lower and upper thresholds
	const vec3 deltas = vec3(
		angleDistance(hsvColor.x, parameters.hue, 180.0), //rgb2hsv retunrs hue in [0, 360]
		hsvColor.y,
		hsvColor.z
	);
	const vec3 threshlold0 = vec3(
		parameters.deltaHueThreshold,
		parameters.saturationThreshold - parameters.saturationSmoothness,
		parameters.valueThreshold - parameters.valueSmoothness
	);
	const vec3 threshlold1 = vec3(
		parameters.deltaHueThreshold + parameters.deltaHueSmoothness,
		parameters.saturationThreshold,
		parameters.valueThreshold
	);

	//Calculate the alphas related to each of the parameters
	//Only use smoothstep if its behaviour is defined (edge0 < edge1)
	const vec3 alphas = mix(
		step(threshlold0, deltas),
		smoothstep(threshlold0, threshlold1, deltas),
		lessThan(threshlold0, threshlold1)
	);

	//The result will be the least limiting one
	return max(alphas.x, max(alphas.y, alphas.z));
}

float linearKeyAlpha(in int type, in vec4 keyColor, in vec4 fillColor) {
	float result;

	switch(abs(type)) {
	case LINEAR_KEY_KEY_R:	result = keyColor.r;					break;
	case LINEAR_KEY_KEY_G:	result = keyColor.g;					break;
	case LINEAR_KEY_KEY_B:	result = keyColor.b;					break;
	case LINEAR_KEY_KEY_A:	result = keyColor.a;					break;
	case LINEAR_KEY_KEY_Y:	result = getLuminance(keyColor.rgb);	break;

	case LINEAR_KEY_FILL_R:	result = fillColor.r;					break;
	case LINEAR_KEY_FILL_G:	result = fillColor.g;					break;
	case LINEAR_KEY_FILL_B:	result = fillColor.b;					break;
	case LINEAR_KEY_FILL_A:	result = fillColor.a;					break;
	case LINEAR_KEY_FILL_Y:	result = getLuminance(fillColor.rgb);	break;

	default:				result = 1.0f;							break;
	}

	return (type < 0) ? (1.0f - result) : result;
}



void main() {
	//Obtain th signed distance to the curve
	const float sDist = bezier3_signed_distance(in_klm);
	const float sDistOpacity = clamp(0.5f - sDist, 0.0f, 1.0f);
	if(sDistOpacity <= 0.0f) {
		//Discard everything which is outside
		discard;
	}

	//Sample the key frame
	const vec4 keyColor = frame_texture(sampleMode, frame_sampler(2), in_texCoord);

	//Sample the fill frame
	vec4 fillColor; 
	if(sameKeyFill) {
		fillColor = keyColor;
	} else {
		fillColor = frame_texture(sampleMode, frame_sampler(3), in_texCoord);
	}

	//Apply all alpha-s
	float alpha = opacity * sDistOpacity;
	if(lumaKeyEnabled) {
		alpha *= lumaKeyAlpha(lumaKeyparameters, keyColor.rgb);
	}
	if(chromaKeyEnabled) {
		alpha *= chromaKeyAlpha(chromaKeyparameters, keyColor.rgb);
	}
	if(linearKeyType != LINEAR_KEY_DISABLED) {
		alpha *= linearKeyAlpha(linearKeyType, keyColor, fillColor);
	} 

	//Compute the final color
	out_color = vec4(fillColor.rgb, alpha);
	out_color = frame_premultiply_alpha(out_color);
}
 

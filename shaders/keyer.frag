#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "color_transfer.glsl"
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
	float luminosityThreshold;
	float luminositySmoothness;
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

layout(constant_id = 0)  const bool sameKeyFill = false;
layout(constant_id = 1)  const bool lumaKeyEnabled = false;
layout(constant_id = 2)  const bool chromaKeyEnabled = false;
layout(constant_id = 3)  const int linearKeyType = 0;

//Vertex I/O
layout(location = 0) in vec2 in_texCoord;
layout(location = 1) in vec3 in_klm;

layout(location = 0) out vec4 out_color;

//Uniform buffers
layout(set = 0, binding = 1) uniform OutputColorTransferBlock{
	ct_write_data 			outColorTransfer;
};

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

	float alpha;
	if(parameters.minThreshHold < 0.0f) {
		//Negative threshold signals inversion
		if(parameters.minThreshHold > parameters.maxThreshHold) {
			//Ensures that he parameter provided to smoothstep is valid.
			//Note that min and max are negative, so max comes first
			alpha = smoothstep(parameters.maxThreshHold, parameters.minThreshHold, -luminance);
		} else {
			//Simple threshlold
			alpha = luminance > -parameters.minThreshHold ? 0.0f : 1.0f;
		}
	} else {
		//Positive threshold, normal operation
		if(parameters.minThreshHold < parameters.maxThreshHold) {
			//Ensures that he parameter provided to smoothstep is valid
			alpha = smoothstep(parameters.minThreshHold, parameters.maxThreshHold, luminance);
		} else {
			//Simple threshlold
			alpha = luminance < parameters.minThreshHold ? 0.0f : 1.0f;
		}
	}

	return alpha;
}

float chromaKeyAlpha(in ChromaKeyParameters parameters, in vec3 keyColor) {
	//Convert the color into hsl
	const vec3 hslColor = rgb2hsl(keyColor);

	//Calculate the attenuation regarding the hue
	float alphaHue;
	const float deltaHue = mod(abs(hslColor.x - parameters.hue), 360.0f);
	const float maxDeltaHue0 = parameters.deltaHueThreshold;
	if(parameters.deltaHueSmoothness > 0.0f) {
		//Ensures that he parameter provided to smoothstep is valid
		const float maxDeltaHue1 = maxDeltaHue0 + parameters.deltaHueSmoothness;
		alphaHue = smoothstep(maxDeltaHue0, maxDeltaHue1, deltaHue);
	} else {
		//Simple threshlold
		alphaHue = (deltaHue > maxDeltaHue0) ? 1.0f : 0.0f;
	}

	//Calculate the attenuation regarding the saturation
	float alphaSat;
	const float minSat1 = parameters.saturationThreshold;
	if(parameters.saturationSmoothness > 0.0f) {
		//Ensures that he parameter provided to smoothstep is valid
		const float minSat0 = minSat1 - parameters.saturationSmoothness;
		alphaSat = 1.0f - smoothstep(minSat0, minSat1, hslColor.y);
	} else {
		//Simple threshlold
		alphaSat = (hslColor.y < minSat1) ? 1.0f : 0.0f;
	}

	//Calculate the attenuation regarding the luminosity.
	//For HSL colorspace, the maximum saturation is at 0.5 luminosity
	float alphaLum;
	const float deltaLuminosity = abs(0.5f - hslColor.z);
	const float minLum0 = parameters.luminosityThreshold;
	if(parameters.luminositySmoothness > 0.0f) {
		//Ensures that he parameter provided to smoothstep is valid
		const float minLum1 = minLum0 + parameters.luminositySmoothness;
		alphaLum = smoothstep(minLum0, minLum1, deltaLuminosity);
	} else {
		//Simple threshlold
		alphaLum = (deltaLuminosity > minLum0) ? 1.0f : 0.0f;
	}

	//The result will be the least limiting one
	return max(alphaHue, max(alphaSat, alphaLum));
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
	const vec4 keyColor = frame_texture(2, in_texCoord);

	//Sample the fill frame
	vec4 fillColor; 
	if(sameKeyFill) {
		fillColor = keyColor;
	} else {
		fillColor = frame_texture(3, in_texCoord);
	}
	fillColor = ct_transferColor(frame_color_transfer(3), outColorTransfer, fillColor);

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
	out_color = ct_premultiply_alpha(out_color);
}
 

/* 
 * This file is part of Cycles for Max. (c) Jeffrey Witthuhn
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
#pragma once

/**
 * @file
 * @brief Defines commonly-used enums.
 */

enum class RenderPassType {
	COMBINED,
	DIFFUSE_COLOR,
	DIFFUSE_DIRECT,
	DIFFUSE_INDIRECT,
	GLOSSY_COLOR,
	GLOSSY_DIRECT,
	GLOSSY_INDIRECT,
	TRANSMISSION_COLOR,
	TRANSMISSION_DIRECT,
	TRANSMISSION_INDIRECT,
	SUBSURFACE_COLOR,
	SUBSURFACE_DIRECT,
	SUBSURFACE_INDIRECT,
	EMISSION,
	ENVIRONMENT,
	NORMAL,
	UV,
	MOTION,
	DEPTH,
	MIST,
	CRYPTOMATTE_OBJ,
	CRYPTOMATTE_MTL,
	CRYPTOMATTE_ASSET,
	ADAPTIVE_AUX,
	SAMPLE_COUNT,
	INVALID,
};

enum class CameraPanoramaType {
	EQUIRECTANGULAR,
	FISHEYE_EQUIDISTANT,
	FISHEYE_EQUISOLID,
};

enum class AccumulationBufferType {
	NONE,
	SINGLE,
	ANAGLYPH_RC,
	ANAGLYPH_GM,
	ANAGLYPH_YB,
	LEFT_RIGHT,
	TOP_BOTTOM,
};

enum class StereoscopyType {
	NONE,
	LEFT_EYE,
	RIGHT_EYE,
	ANAGLYPH,
	SPLIT_LEFT_RIGHT,
	SPLIT_TOP_BOTTOM,
	TYPE_COUNT,
};

enum class AnaglyphType {
	RED_CYAN,
	GREEN_MAGENTA,
	YELLOW_BLUE,
	TYPE_COUNT,
};

enum class CyclesLightType {
	INVALID,
	POINT,
	SPHERE,
	DIRECT,
	SPOT,
};

enum class RenderDevice {
	CPU,
	CUDA,
	OPTIX,
	DEVICE_COUNT,
};

// Material description enums

enum class DisplacementMethod {
	BUMP_MAP,
	DISPLACEMENT,
	BOTH,
};

enum class VolumeSamplingMethod {
	MULTIPLE_IMPORTANCE,
	EQUIANGULAR,
	DISTANCE,
};

enum class VolumeInterpolationMethod {
	LINEAR,
	CUBIC,
};

enum class AnisotropicDistribution {
	ASHIKHMIN_SHIRLEY,
	MULTISCATTER_GGX,
	GGX,
	BECKMANN,
	COUNT,
};

enum class GlassDistribution {
	MULTISCATTER_GGX,
	GGX,
	BECKMANN,
	COUNT,
};

enum class GlossyDistribution {
	ASHIKHMIN_SHIRLEY,
	MULTISCATTER_GGX,
	GGX,
	BECKMANN,
	COUNT,
};

enum class HairComponent {
	TRANSMISSION,
	REFLECTION,
	COUNT,
};

enum class PrincipledBsdfDistribution {
	MULTISCATTER_GGX,
	GGX,
	COUNT,
};

enum class PrincipledBsdfSSSMethod {
	BURLEY,
	RANDOM_WALK,
	COUNT,
};

enum class RefractionDistribution {
	GGX,
	BECKMANN,
	COUNT,
};

enum class SubsurfaceFalloff {
	CUBIC,
	GAUSSIAN,
	BURLEY,
	RANDOM_WALK,
	COUNT,
};

enum class ToonComponent {
	GLOSSY,
	DIFFUSE,
	COUNT,
};

enum class NormalMapTangentSpace {
	TANGENT,
	OBJECT,
	WORLD,
	COUNT,
};

// Texmap enums

enum class EnvironmentProjection {
	MIRROR_BALL,
	EQUIRECTANGULAR,
	BACKPLATE,
};

enum class SkyType {
	HOSEK_WILKIE,
	PREETHAM
};

// Log enums

enum class LogLevel {
	ALWAYS,
	ERR,
	WARN,
	INFO,
	DEBUG
};

enum class LogCtl {
	SEPARATOR,
	WRITE_LINE
};

enum class LogNumberFormat {
	OCTAL,
	DECIMAL,
	HEX
};

// Misc

enum class FilmFilterType {
	BOX,
	GAUSSIAN,
	BLACKMAN_HARRIS,
	COUNT,
};

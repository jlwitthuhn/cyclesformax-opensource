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
 
#include "util_pass.h"

#include <array>
#include <cstdio>

#include "plugin_re_simple.h"

static ccl::PassType get_ccl_pass_type(const RenderPassType type)
{
	switch (type) {
	case RenderPassType::COMBINED:
		return ccl::PassType::PASS_COMBINED;
	case RenderPassType::DIFFUSE_COLOR:
		return ccl::PassType::PASS_DIFFUSE_COLOR;
	case RenderPassType::DIFFUSE_DIRECT:
		return ccl::PassType::PASS_DIFFUSE_DIRECT;
	case RenderPassType::DIFFUSE_INDIRECT:
		return ccl::PassType::PASS_DIFFUSE_INDIRECT;
	case RenderPassType::GLOSSY_COLOR:
		return ccl::PassType::PASS_GLOSSY_COLOR;
	case RenderPassType::GLOSSY_DIRECT:
		return ccl::PassType::PASS_GLOSSY_DIRECT;
	case RenderPassType::GLOSSY_INDIRECT:
		return ccl::PassType::PASS_GLOSSY_INDIRECT;
	case RenderPassType::TRANSMISSION_COLOR:
		return ccl::PassType::PASS_TRANSMISSION_COLOR;
	case RenderPassType::TRANSMISSION_DIRECT:
		return ccl::PassType::PASS_TRANSMISSION_DIRECT;
	case RenderPassType::TRANSMISSION_INDIRECT:
		return ccl::PassType::PASS_TRANSMISSION_INDIRECT;
	case RenderPassType::SUBSURFACE_COLOR:
		return ccl::PassType::PASS_DIFFUSE_COLOR;
	case RenderPassType::SUBSURFACE_DIRECT:
		return ccl::PassType::PASS_DIFFUSE_DIRECT;
	case RenderPassType::SUBSURFACE_INDIRECT:
		return ccl::PassType::PASS_DIFFUSE_INDIRECT;
	case RenderPassType::EMISSION:
		return ccl::PassType::PASS_EMISSION;
	case RenderPassType::ENVIRONMENT:
		return ccl::PassType::PASS_BACKGROUND;
	case RenderPassType::NORMAL:
		return ccl::PassType::PASS_NORMAL;
	case RenderPassType::UV:
		return ccl::PassType::PASS_UV;
	case RenderPassType::MOTION:
		return ccl::PassType::PASS_MOTION;
	case RenderPassType::DEPTH:
		return ccl::PassType::PASS_DEPTH;
	case RenderPassType::MIST:
		return ccl::PassType::PASS_MIST;
	case RenderPassType::CRYPTOMATTE_OBJ:
	case RenderPassType::CRYPTOMATTE_MTL:
	case RenderPassType::CRYPTOMATTE_ASSET:
		return ccl::PassType::PASS_CRYPTOMATTE;
	}
	return ccl::PassType::PASS_NONE;
}

static int get_channel_count(const RenderPassType type)
{
	switch (type) {
	case RenderPassType::COMBINED:
	case RenderPassType::DIFFUSE_COLOR:
	case RenderPassType::DIFFUSE_DIRECT:
	case RenderPassType::DIFFUSE_INDIRECT:
	case RenderPassType::GLOSSY_COLOR:
	case RenderPassType::GLOSSY_DIRECT:
	case RenderPassType::GLOSSY_INDIRECT:
	case RenderPassType::TRANSMISSION_COLOR:
	case RenderPassType::TRANSMISSION_DIRECT:
	case RenderPassType::TRANSMISSION_INDIRECT:
	case RenderPassType::SUBSURFACE_COLOR:
	case RenderPassType::SUBSURFACE_DIRECT:
	case RenderPassType::SUBSURFACE_INDIRECT:
	case RenderPassType::EMISSION:
	case RenderPassType::ENVIRONMENT:
	case RenderPassType::NORMAL:
	case RenderPassType::UV:
	case RenderPassType::MOTION:
	case RenderPassType::CRYPTOMATTE_OBJ:
	case RenderPassType::CRYPTOMATTE_MTL:
	case RenderPassType::CRYPTOMATTE_ASSET:
		return 4;
	case RenderPassType::DEPTH:
	case RenderPassType::MIST:
		return 1;
	}
	return 0;
}

static std::string get_pass_name(const RenderPassType type)
{
	switch (type) {
	case RenderPassType::COMBINED:
		return "Combined";
	case RenderPassType::DIFFUSE_COLOR:
		return "__diffuse_color";
	case RenderPassType::DIFFUSE_DIRECT:
		return "__diffuse_direct";
	case RenderPassType::DIFFUSE_INDIRECT:
		return "__diffuse_indirect";
	case RenderPassType::GLOSSY_COLOR:
		return "__glossy_color";
	case RenderPassType::GLOSSY_DIRECT:
		return "__glossy_direct";
	case RenderPassType::GLOSSY_INDIRECT:
		return "__glossy_indirect";
	case RenderPassType::TRANSMISSION_COLOR:
		return "__transmission_color";
	case RenderPassType::TRANSMISSION_DIRECT:
		return "__transmission_direct";
	case RenderPassType::TRANSMISSION_INDIRECT:
		return "__transmission_indirect";
	case RenderPassType::SUBSURFACE_COLOR:
		return "__subsurface_color";
	case RenderPassType::SUBSURFACE_DIRECT:
		return "__subsurface_direct";
	case RenderPassType::SUBSURFACE_INDIRECT:
		return "__subsurface_indirect";
	case RenderPassType::EMISSION:
		return "__emission";
	case RenderPassType::ENVIRONMENT:
		return "__environment";
	case RenderPassType::NORMAL:
		return "__normal";
	case RenderPassType::UV:
		return "__uv";
	case RenderPassType::MOTION:
		return "__motion";
	case RenderPassType::DEPTH:
		return "__depth";
	case RenderPassType::MIST:
		return "__mist";
	case RenderPassType::CRYPTOMATTE_OBJ:
		return "CryptoObject";
	case RenderPassType::CRYPTOMATTE_MTL:
		return "CryptoMaterial";
	case RenderPassType::CRYPTOMATTE_ASSET:
		return "CryptoAsset";
	}
	return "__error";
}

RenderPassInfo::RenderPassInfo() :
	type{ RenderPassType::COMBINED },
	ccl_type{ get_ccl_pass_type(RenderPassType::COMBINED) },
	channels{ get_channel_count(RenderPassType::COMBINED) },
	name{ get_pass_name(RenderPassType::COMBINED) },
	render_element{ nullptr }
{

}

RenderPassInfo::RenderPassInfo(CyclesRenderElement* const render_element) :
	type{ render_element->GetRenderPassType() },
	ccl_type{ get_ccl_pass_type(render_element->GetRenderPassType()) },
	channels{ get_channel_count(render_element->GetRenderPassType()) },
	name{ get_pass_name(render_element->GetRenderPassType()) },
	render_element{ render_element }
{

}

RenderPassInfo::RenderPassInfo(CyclesRenderElement* const render_element, const size_t suffix) :
	RenderPassInfo(render_element)
{
	std::array<char, 48> buffer;
	buffer.fill('\0');
	snprintf(buffer.data(), buffer.size(), "%s%02d", name.data(), static_cast<int>(suffix));
	name = std::string{ buffer.data() };
}

RenderPassInfo::RenderPassInfo(RenderPassType type, ccl::PassType ccl_type, int channels, std::string name, size_t suffix) :
	type{ type },
	ccl_type{ ccl_type },
	channels{ channels },
	name{ name },
	render_element{ nullptr }
{
	std::array<char, 48> buffer;
	buffer.fill('\0');
	snprintf(buffer.data(), buffer.size(), "%s%02d", name.data(), static_cast<int>(suffix));
	this->name = std::string{ buffer.data() };
}

ccl::vector<ccl::Pass> get_ccl_pass_vector(const std::vector<RenderPassInfo>& pass_info_vector)
{
	ccl::vector<ccl::Pass> result;

	for (const RenderPassInfo& pass_info : pass_info_vector) {
		ccl::Pass::add(pass_info.ccl_type, result, pass_info.name.c_str());
	}

	return result;
}

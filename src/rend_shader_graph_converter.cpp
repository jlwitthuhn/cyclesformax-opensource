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
 
#include "rend_shader_graph_converter.h"

#include <cassert>

#include <boost/optional.hpp>

#include <render/nodes.h>

#include <kernel/kernel_types.h>

#undef ABSOLUTE
#undef DIFFERENCE

#include <shader_core/vector.h>
#include <shader_graph/graph.h>
#include <shader_graph/node.h>

#include "cache_baked_texmap.h"

static ccl::float3 float3_to_ccl_float3(const csc::Float3 in_vec)
{
	ccl::float3 result;

	result.x = in_vec.x;
	result.y = in_vec.y;
	result.z = in_vec.z;

	return result;
}

ShaderGraphConverter::ShaderGraphConverter(BakedTexmapCache& texmap_cache) :
	texmap_cache(texmap_cache),
	logger(global_log_manager.new_logger(L"ShaderGraphConverter"))
{
	*logger << LogCtl::SEPARATOR;
}

void ShaderGraphConverter::set_texmap_slot(Texmap* const texmap, const size_t slot)
{
	if (texmap == nullptr) {
		return;
	}

	texmap_slots[slot] = texmap;
}

static void apply_shader_param(ccl::Node& ccl_node, const csg::Node& csg_node, const char* name)
{
	for (const ccl::SocketType& socket_type : ccl_node.type->inputs) {
		if (socket_type.name.string() == name) {
			// We have found the matching input on the ccl::Node
			const boost::optional<size_t> opt_index{ csg_node.slot_index(csg::SlotDirection::INPUT, name) };
			if (opt_index) {
				const boost::optional<csg::Slot> opt_slot{ csg_node.slot(*opt_index) };
				if (opt_slot && opt_slot->value) {
					if (const auto bool_val{ opt_slot->value->as<csg::BoolSlotValue>() }) {
						ccl_node.set(socket_type, bool_val->get());
					}
					else if (const auto float3_val{ opt_slot->value->as<csg::ColorSlotValue>() }) {
						ccl_node.set(socket_type, float3_to_ccl_float3(float3_val->get()));
					}
					else if (const auto float_val{ opt_slot->value->as<csg::FloatSlotValue>() }) {
						ccl_node.set(socket_type, float_val->get());
					}
					else if (const auto float3_val{ opt_slot->value->as<csg::VectorSlotValue>() }) {
						ccl_node.set(socket_type, float3_to_ccl_float3(float3_val->get()));
					}
				}
			}
		}
	}
}

ccl::ShaderGraph* ShaderGraphConverter::get_shader_graph(const std::string encoded_graph, ccl::Scene* const scene) const
{
	*logger << "get_shader_graph called..." << LogCtl::WRITE_LINE;
	*logger << "encoded graph: " << encoded_graph.c_str() << LogCtl::WRITE_LINE;

	ccl::ShaderGraph* const cycles_graph = new ccl::ShaderGraph();

	const boost::optional<csg::Graph> opt_graph{ csg::Graph::from(encoded_graph) };
	if (opt_graph.has_value() == false) {
		// Serialized graph is not valid, return empty graph
		return cycles_graph;
	}

	std::map<csg::NodeId, ccl::ShaderNode*> nodes_by_id;

	const std::list<std::shared_ptr<csg::Node>> node_list{ opt_graph->nodes() };
	for (const std::shared_ptr<csg::Node> this_node : node_list) {
		if (this_node.use_count() == 0) {
			continue;
		}
		ccl::ShaderNode* new_node{ nullptr };
		switch (this_node->type()) {
		//////
		// Output
		//////
		case csg::NodeType::MATERIAL_OUTPUT:
		{
			nodes_by_id[this_node->id()] = cycles_graph->output();
			break;
		}
		//////
		// Color
		//////
		case csg::NodeType::BRIGHTNESS_CONTRAST:
		{
			ccl::BrightContrastNode* const ccl_node{ new ccl::BrightContrastNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "bright");
			apply_shader_param(*ccl_node, *this_node, "contrast");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::GAMMA:
		{
			ccl::GammaNode* const ccl_node{ new ccl::GammaNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "gamma");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::HSV:
		{
			ccl::HSVNode* const ccl_node{ new ccl::HSVNode() };
			apply_shader_param(*ccl_node, *this_node, "hue");
			apply_shader_param(*ccl_node, *this_node, "saturation");
			apply_shader_param(*ccl_node, *this_node, "value");
			apply_shader_param(*ccl_node, *this_node, "fac");
			apply_shader_param(*ccl_node, *this_node, "color");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::INVERT:
		{
			ccl::InvertNode* const ccl_node{ new ccl::InvertNode() };
			apply_shader_param(*ccl_node, *this_node, "fac");
			apply_shader_param(*ccl_node, *this_node, "color");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::LIGHT_FALLOFF:
		{
			ccl::LightFalloffNode* const ccl_node{ new ccl::LightFalloffNode() };
			apply_shader_param(*ccl_node, *this_node, "strength");
			apply_shader_param(*ccl_node, *this_node, "smooth");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::MIX_RGB:
		{
			ccl::MixNode* const ccl_node{ new ccl::MixNode() };
			apply_shader_param(*ccl_node, *this_node, "use_clamp");
			apply_shader_param(*ccl_node, *this_node, "fac");
			apply_shader_param(*ccl_node, *this_node, "color1");
			apply_shader_param(*ccl_node, *this_node, "color2");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("mix_type") }) {
				const csg::MixRGBType dist{ static_cast<csg::MixRGBType>(opt_enum->get()) };
				switch (dist) {
				case csg::MixRGBType::DARKEN:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_DARK);
					break;
				case csg::MixRGBType::MULTIPLY:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_MUL);
					break;
				case csg::MixRGBType::BURN:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_BURN);
					break;
				case csg::MixRGBType::LIGHTEN:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_LIGHT);
					break;
				case csg::MixRGBType::SCREEN:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_SCREEN);
					break;
				case csg::MixRGBType::DODGE:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_DODGE);
					break;
				case csg::MixRGBType::ADD:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_ADD);
					break;
				case csg::MixRGBType::OVERLAY:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_OVERLAY);
					break;
				case csg::MixRGBType::SOFT_LIGHT:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_SOFT);
					break;
				case csg::MixRGBType::LINEAR_LIGHT:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_LINEAR);
					break;
				case csg::MixRGBType::DIFFERENCE:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_DIFF);
					break;
				case csg::MixRGBType::SUBTRACT:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_SUB);
					break;
				case csg::MixRGBType::DIVIDE:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_DIV);
					break;
				case csg::MixRGBType::HUE:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_HUE);
					break;
				case csg::MixRGBType::SATURATION:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_SAT);
					break;
				case csg::MixRGBType::COLOR:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_COLOR);
					break;
				case csg::MixRGBType::VALUE:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_VAL);
					break;
				case csg::MixRGBType::MIX:
				default:
					ccl_node->set_mix_type(ccl::NodeMix::NODE_MIX_BLEND);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::RGB_CURVES:
		{
			ccl::RGBCurvesNode* const ccl_node{ new ccl::RGBCurvesNode() };
			apply_shader_param(*ccl_node, *this_node, "fac");
			apply_shader_param(*ccl_node, *this_node, "color");
			if (const auto opt_curves{ this_node->slot_value_as<csg::RGBCurveSlotValue>("curves") }) {
				const csg::Curve curve_all{ opt_curves->get_all() };
				const csg::Curve curve_r{ opt_curves->get_r() };
				const csg::Curve curve_g{ opt_curves->get_g() };
				const csg::Curve curve_b{ opt_curves->get_b() };
				ccl::array<ccl::float3> ramp_array;
				ramp_array.resize(RAMP_TABLE_SIZE);
				for (size_t i = 0; i < RAMP_TABLE_SIZE; i++) {
					const float t{ static_cast<float>(i) / static_cast<float>(RAMP_TABLE_SIZE - 1) };
					const float r{ curve_all.eval_point(curve_r.eval_point(t)) };
					const float g{ curve_all.eval_point(curve_g.eval_point(t)) };
					const float b{ curve_all.eval_point(curve_b.eval_point(t)) };
					ramp_array[i] = ccl::make_float3(r, g, b);
				}
				ccl_node->set_curves(ramp_array);
			}
			new_node = ccl_node;
			break;
		}
		//////
		// Converter
		//////
		case csg::NodeType::BLACKBODY:
		{
			ccl::BlackbodyNode* const ccl_node{ new ccl::BlackbodyNode() };
			apply_shader_param(*ccl_node, *this_node, "temperature");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::CLAMP:
		{
			ccl::ClampNode* const ccl_node{ new ccl::ClampNode() };
			apply_shader_param(*ccl_node, *this_node, "value");
			apply_shader_param(*ccl_node, *this_node, "min");
			apply_shader_param(*ccl_node, *this_node, "max");
			new_node = ccl_node;
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("clamp_type") }) {
				const csg::ClampType type{ static_cast<csg::ClampType>(opt_enum->get()) };
				switch (type) {
				case csg::ClampType::MINMAX:
					ccl_node->set_clamp_type(ccl::NodeClampType::NODE_CLAMP_MINMAX);
					break;
				case csg::ClampType::RANGE:
				default:
					ccl_node->set_clamp_type(ccl::NodeClampType::NODE_CLAMP_RANGE);
					break;
				}
			}
			break;
		}
		case csg::NodeType::COLOR_RAMP:
		{
			ccl::RGBRampNode* const ccl_node{ new ccl::RGBRampNode() };
			apply_shader_param(*ccl_node, *this_node, "fac");
			if (const auto opt_ramp{ this_node->slot_value_as<csg::ColorRampSlotValue>("ramp") }) {
				const csg::ColorRamp ramp = opt_ramp->get();
				ccl::array<ccl::float3> ramp_array;
				ramp_array.resize(RAMP_TABLE_SIZE);
				ccl::array<float> ramp_alpha_array;
				ramp_alpha_array.resize(RAMP_TABLE_SIZE);
				for (size_t i = 0; i < RAMP_TABLE_SIZE; i++) {
					const float t{ static_cast<float>(i) / static_cast<float>(RAMP_TABLE_SIZE - 1) };
					const csc::Float4 val{ ramp.eval(t) };
					ramp_array[i] = ccl::make_float3(val.x, val.y, val.z);
					ramp_alpha_array[i] = val.w;
				}
				ccl_node->set_ramp(ramp_array);
				ccl_node->set_ramp_alpha(ramp_alpha_array);
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::COMBINE_HSV:
		{
			ccl::CombineHSVNode* const ccl_node{ new ccl::CombineHSVNode() };
			apply_shader_param(*ccl_node, *this_node, "h");
			apply_shader_param(*ccl_node, *this_node, "s");
			apply_shader_param(*ccl_node, *this_node, "v");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::COMBINE_RGB:
		{
			ccl::CombineRGBNode* const ccl_node{ new ccl::CombineRGBNode() };
			apply_shader_param(*ccl_node, *this_node, "r");
			apply_shader_param(*ccl_node, *this_node, "g");
			apply_shader_param(*ccl_node, *this_node, "b");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::COMBINE_XYZ:
		{
			ccl::CombineXYZNode* const ccl_node{ new ccl::CombineXYZNode() };
			apply_shader_param(*ccl_node, *this_node, "x");
			apply_shader_param(*ccl_node, *this_node, "y");
			apply_shader_param(*ccl_node, *this_node, "z");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::MAP_RANGE:
		{
			ccl::MapRangeNode* const ccl_node{ new ccl::MapRangeNode() };
			apply_shader_param(*ccl_node, *this_node, "clamp");
			apply_shader_param(*ccl_node, *this_node, "from_min");
			apply_shader_param(*ccl_node, *this_node, "from_max");
			apply_shader_param(*ccl_node, *this_node, "to_min");
			apply_shader_param(*ccl_node, *this_node, "to_max");
			apply_shader_param(*ccl_node, *this_node, "steps");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("range_type") }) {
				const csg::MapRangeType type{ static_cast<csg::MapRangeType>(opt_enum->get()) };
				switch (type) {
				case csg::MapRangeType::STEPPED:
					ccl_node->set_range_type(ccl::NodeMapRangeType::NODE_MAP_RANGE_STEPPED);
					break;
				case csg::MapRangeType::SMOOTH_STEP:
					ccl_node->set_range_type(ccl::NodeMapRangeType::NODE_MAP_RANGE_SMOOTHSTEP);
					break;
				case csg::MapRangeType::SMOOTHER_STEP:
					ccl_node->set_range_type(ccl::NodeMapRangeType::NODE_MAP_RANGE_SMOOTHERSTEP);
					break;
				case csg::MapRangeType::LINEAR:
				default:
					ccl_node->set_range_type(ccl::NodeMapRangeType::NODE_MAP_RANGE_LINEAR);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::MATH:
		{
			ccl::MathNode* const ccl_node{ new ccl::MathNode() };
			apply_shader_param(*ccl_node, *this_node, "use_clamp");
			apply_shader_param(*ccl_node, *this_node, "value1");
			apply_shader_param(*ccl_node, *this_node, "value2");
			apply_shader_param(*ccl_node, *this_node, "value3");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("math_type") }) {
				const csg::MathType type{ static_cast<csg::MathType>(opt_enum->get()) };
				switch (type) {
				case csg::MathType::SUBTRACT:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SUBTRACT);
					break;
				case csg::MathType::MULTIPLY:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_MULTIPLY);
					break;
				case csg::MathType::DIVIDE:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_DIVIDE);
					break;
				case csg::MathType::MULTIPLY_ADD:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_MULTIPLY_ADD);
					break;
				case csg::MathType::SINE:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SINE);
					break;
				case csg::MathType::COSINE:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_COSINE);
					break;
				case csg::MathType::TANGENT:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_TANGENT);
					break;
				case csg::MathType::ARCSINE:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_ARCSINE);
					break;
				case csg::MathType::ARCCOSINE:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_ARCCOSINE);
					break;
				case csg::MathType::ARCTANGENT:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_ARCTANGENT);
					break;
				case csg::MathType::ARCTAN2:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_ARCTAN2);
					break;
				case csg::MathType::SINH:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SINH);
					break;
				case csg::MathType::COSH:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_COSH);
					break;
				case csg::MathType::TANH:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_TANH);
					break;
				case csg::MathType::POWER:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_POWER);
					break;
				case csg::MathType::LOGARITHM:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_LOGARITHM);
					break;
				case csg::MathType::MINIMUM:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_MINIMUM);
					break;
				case csg::MathType::MAXIMUM:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_MAXIMUM);
					break;
				case csg::MathType::LESS_THAN:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_LESS_THAN);
					break;
				case csg::MathType::GREATER_THAN:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_GREATER_THAN);
					break;
				case csg::MathType::MODULO:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_MODULO);
					break;
				case csg::MathType::ABSOLUTE:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_ABSOLUTE);
					break;
				case csg::MathType::ROUND:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_ROUND);
					break;
				case csg::MathType::FLOOR:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_FLOOR);
					break;
				case csg::MathType::CEIL:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_CEIL);
					break;
				case csg::MathType::FRACTION:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_FRACTION);
					break;
				case csg::MathType::SQRT:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SQRT);
					break;
				case csg::MathType::INV_SQRT:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_INV_SQRT);
					break;
				case csg::MathType::SIGN:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SIGN);
					break;
				case csg::MathType::EXPONENT:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_EXPONENT);
					break;
				case csg::MathType::RADIANS:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_RADIANS);
					break;
				case csg::MathType::DEGREES:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_DEGREES);
					break;
				case csg::MathType::TRUNC:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_TRUNC);
					break;
				case csg::MathType::SNAP:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SNAP);
					break;
				case csg::MathType::WRAP:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_WRAP);
					break;
				case csg::MathType::COMPARE:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_COMPARE);
					break;
				case csg::MathType::PINGPONG:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_PINGPONG);
					break;
				case csg::MathType::SMOOTH_MIN:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SMOOTH_MIN);
					break;
				case csg::MathType::SMOOTH_MAX:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_SMOOTH_MAX);
					break;
				case csg::MathType::ADD:
				default:
					ccl_node->set_math_type(ccl::NodeMathType::NODE_MATH_ADD);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::RGB_TO_BW:
		{
			ccl::RGBToBWNode* const ccl_node{ new ccl::RGBToBWNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::SEPARATE_HSV:
		{
			ccl::SeparateHSVNode* const ccl_node{ new ccl::SeparateHSVNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::SEPARATE_RGB:
		{
			ccl::SeparateRGBNode* const ccl_node{ new ccl::SeparateRGBNode() };
			apply_shader_param(*ccl_node, *this_node, "image");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::SEPARATE_XYZ:
		{
			ccl::SeparateXYZNode* const ccl_node{ new ccl::SeparateXYZNode() };
			apply_shader_param(*ccl_node, *this_node, "vector");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VECTOR_MATH:
		{
			ccl::VectorMathNode* const ccl_node{ new ccl::VectorMathNode() };
			apply_shader_param(*ccl_node, *this_node, "vector1");
			apply_shader_param(*ccl_node, *this_node, "vector2");
			apply_shader_param(*ccl_node, *this_node, "vector3");
			apply_shader_param(*ccl_node, *this_node, "scale");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("math_type") }) {
				const csg::VectorMathType type{ static_cast<csg::VectorMathType>(opt_enum->get()) };
				switch (type) {
				case csg::VectorMathType::SUBTRACT:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_SUBTRACT);
					break;
				case csg::VectorMathType::MULTIPLY:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_MULTIPLY);
					break;
				case csg::VectorMathType::DIVIDE:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_DIVIDE);
					break;
				case csg::VectorMathType::CROSS_PRODUCT:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_CROSS_PRODUCT);
					break;
				case csg::VectorMathType::PROJECT:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_PROJECT);
					break;
				case csg::VectorMathType::REFLECT:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_REFLECT);
					break;
				case csg::VectorMathType::DOT_PRODUCT:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_DOT_PRODUCT);
					break;
				case csg::VectorMathType::DISTANCE:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_DISTANCE);
					break;
				case csg::VectorMathType::LENGTH:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_LENGTH);
					break;
				case csg::VectorMathType::SCALE:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_SCALE);
					break;
				case csg::VectorMathType::NORMALIZE:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_NORMALIZE);
					break;
				case csg::VectorMathType::SNAP:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_SNAP);
					break;
				case csg::VectorMathType::FLOOR:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_FLOOR);
					break;
				case csg::VectorMathType::CEIL:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_CEIL);
					break;
				case csg::VectorMathType::MODULO:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_MODULO);
					break;
				case csg::VectorMathType::FRACTION:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_FRACTION);
					break;
				case csg::VectorMathType::ABSOLUTE:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_ABSOLUTE);
					break;
				case csg::VectorMathType::MINIMUM:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_MINIMUM);
					break;
				case csg::VectorMathType::MAXIMUM:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_MAXIMUM);
					break;
				case csg::VectorMathType::WRAP:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_WRAP);
					break;
				case csg::VectorMathType::SINE:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_SINE);
					break;
				case csg::VectorMathType::COSINE:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_COSINE);
					break;
				case csg::VectorMathType::TANGENT:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_TANGENT);
					break;
				case csg::VectorMathType::ADD:
				default:
					ccl_node->set_math_type(ccl::NodeVectorMathType::NODE_VECTOR_MATH_ADD);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::WAVELENGTH:
		{
			ccl::WavelengthNode* const ccl_node{ new ccl::WavelengthNode() };
			apply_shader_param(*ccl_node, *this_node, "wavelength");
			new_node = ccl_node;
			break;
		}
		//////
		// Input
		//////
		case csg::NodeType::AMBIENT_OCCLUSION:
		{
			ccl::AmbientOcclusionNode* const ccl_node{ new ccl::AmbientOcclusionNode() };
			apply_shader_param(*ccl_node, *this_node, "samples");
			apply_shader_param(*ccl_node, *this_node, "inside");
			apply_shader_param(*ccl_node, *this_node, "only_local");
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "distance");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::BEVEL:
		{
			ccl::BevelNode* const ccl_node{ new ccl::BevelNode() };
			apply_shader_param(*ccl_node, *this_node, "samples");
			apply_shader_param(*ccl_node, *this_node, "radius");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::CAMERA_DATA:
		{
			ccl::CameraNode* const ccl_node{ new ccl::CameraNode() };
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::FRESNEL:
		{
			ccl::FresnelNode* const ccl_node{ new ccl::FresnelNode() };
			apply_shader_param(*ccl_node, *this_node, "IOR");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::GEOMETRY:
		{
			ccl::GeometryNode* const ccl_node{ new ccl::GeometryNode() };
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::LAYER_WEIGHT:
		{
			ccl::LayerWeightNode* const ccl_node{ new ccl::LayerWeightNode() };
			apply_shader_param(*ccl_node, *this_node, "blend");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::LIGHT_PATH:
		{
			ccl::LightPathNode* const ccl_node{ new ccl::LightPathNode() };
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::OBJECT_INFO:
		{
			ccl::ObjectInfoNode* const ccl_node{ new ccl::ObjectInfoNode() };
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::RGB:
		{
			ccl::ColorNode* const ccl_node{ new ccl::ColorNode() };
			apply_shader_param(*ccl_node, *this_node, "value");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::TANGENT:
		{
			ccl::TangentNode* const ccl_node{ new ccl::TangentNode() };
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("direction") }) {
				const csg::TangentDirection type{ static_cast<csg::TangentDirection>(opt_enum->get()) };
				switch (type) {
				case csg::TangentDirection::UV_MAP:
					ccl_node->set_direction_type(ccl::NodeTangentDirectionType::NODE_TANGENT_UVMAP);
					break;
				case csg::TangentDirection::RADIAL:
				default:
					ccl_node->set_direction_type(ccl::NodeTangentDirectionType::NODE_TANGENT_RADIAL);
					break;
				}
			}
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("axis") }) {
				const csg::TangentAxis type{ static_cast<csg::TangentAxis>(opt_enum->get()) };
				switch (type) {
				case csg::TangentAxis::X:
					ccl_node->set_axis(ccl::NodeTangentAxis::NODE_TANGENT_AXIS_X);
					break;
				case csg::TangentAxis::Y:
					ccl_node->set_axis(ccl::NodeTangentAxis::NODE_TANGENT_AXIS_Y);
					break;
				case csg::TangentAxis::Z:
				default:
					ccl_node->set_axis(ccl::NodeTangentAxis::NODE_TANGENT_AXIS_Z);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::TEXTURE_COORDINATE:
		{
			ccl::TextureCoordinateNode* const ccl_node{ new ccl::TextureCoordinateNode() };
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VALUE:
		{
			ccl::ValueNode* const ccl_node{ new ccl::ValueNode() };
			apply_shader_param(*ccl_node, *this_node, "value");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::WIREFRAME:
		{
			ccl::WireframeNode* const ccl_node{ new ccl::WireframeNode() };
			apply_shader_param(*ccl_node, *this_node, "use_pixel_size");
			apply_shader_param(*ccl_node, *this_node, "value");
			new_node = ccl_node;
			break;
		}
		//////
		// Shader
		//////
		case csg::NodeType::ADD_SHADER:
		{
			ccl::AddClosureNode* const ccl_node{ new ccl::AddClosureNode() };
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::ANISOTROPIC_BSDF:
		{
			ccl::AnisotropicBsdfNode* const ccl_node{ new ccl::AnisotropicBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			apply_shader_param(*ccl_node, *this_node, "anisotropy");
			apply_shader_param(*ccl_node, *this_node, "rotation");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("distribution") }) {
				const csg::AnisotropicDistribution dist{ static_cast<csg::AnisotropicDistribution>(opt_enum->get()) };
				switch (dist) {
				case csg::AnisotropicDistribution::ASHIKHMIN_SHIRLEY:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_ASHIKHMIN_SHIRLEY_ID);
					break;
				case csg::AnisotropicDistribution::BECKMANN:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_ID);
					break;
				case csg::AnisotropicDistribution::MULTISCATTER_GGX:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_MULTI_GGX_ID);
					break;
				case csg::AnisotropicDistribution::GGX:
				default:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::DIFFUSE_BSDF:
		{
			ccl::DiffuseBsdfNode* const ccl_node { new ccl::DiffuseBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::EMISSION:
		{
			ccl::EmissionNode* const ccl_node{ new ccl::EmissionNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "strength");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::GLASS_BSDF:
		{
			ccl::GlassBsdfNode* const ccl_node{ new ccl::GlassBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			apply_shader_param(*ccl_node, *this_node, "IOR");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("distribution") }) {
				const csg::GlassDistribution dist{ static_cast<csg::GlassDistribution>(opt_enum->get()) };
				switch (dist) {
				case csg::GlassDistribution::BECKMANN:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_GLASS_ID);
					break;
				case csg::GlassDistribution::MULTISCATTER_GGX:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_MULTI_GGX_GLASS_ID);
					break;
				case csg::GlassDistribution::SHARP:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_SHARP_GLASS_ID);
					break;
				case csg::GlassDistribution::GGX:
				default:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_GLASS_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::GLOSSY_BSDF:
		{
			ccl::GlossyBsdfNode* const ccl_node{ new ccl::GlossyBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("distribution") }) {
				const csg::GlossyDistribution dist{ static_cast<csg::GlossyDistribution>(opt_enum->get()) };
				switch (dist) {
				case csg::GlossyDistribution::ASHIKHMIN_SHIRLEY:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_ASHIKHMIN_SHIRLEY_ID);
					break;
				case csg::GlossyDistribution::BECKMANN:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_ID);
					break;
				case csg::GlossyDistribution::MULTISCATTER_GGX:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_MULTI_GGX_ID);
					break;
				case csg::GlossyDistribution::SHARP:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_REFLECTION_ID);
					break;
				case csg::GlossyDistribution::GGX:
				default:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::HAIR_BSDF:
		{
			ccl::HairBsdfNode* const ccl_node{ new ccl::HairBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "offset");
			apply_shader_param(*ccl_node, *this_node, "roughness_u");
			apply_shader_param(*ccl_node, *this_node, "roughness_v");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("component") }) {
				const csg::HairComponent dist{ static_cast<csg::HairComponent>(opt_enum->get()) };
				switch (dist) {
				case csg::HairComponent::TRANSMISSION:
					ccl_node->set_component(ccl::ClosureType::CLOSURE_BSDF_HAIR_TRANSMISSION_ID);
					break;
				case csg::HairComponent::REFLECTION:
				default:
					ccl_node->set_component(ccl::ClosureType::CLOSURE_BSDF_HAIR_REFLECTION_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::HOLDOUT:
		{
			ccl::HoldoutNode* const ccl_node{ new ccl::HoldoutNode() };
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::MIX_SHADER:
		{
			ccl::MixClosureNode* const ccl_node{ new ccl::MixClosureNode() };
			apply_shader_param(*ccl_node, *this_node, "fac");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::PRINCIPLED_BSDF:
		{
			ccl::PrincipledBsdfNode* const ccl_node{ new ccl::PrincipledBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "base_color");
			apply_shader_param(*ccl_node, *this_node, "subsurface");
			apply_shader_param(*ccl_node, *this_node, "subsurface_radius");
			apply_shader_param(*ccl_node, *this_node, "subsurface_color");
			apply_shader_param(*ccl_node, *this_node, "metallic");
			apply_shader_param(*ccl_node, *this_node, "specular");
			apply_shader_param(*ccl_node, *this_node, "specular_tint");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			apply_shader_param(*ccl_node, *this_node, "anisotropic");
			apply_shader_param(*ccl_node, *this_node, "anisotropic_rotation");
			apply_shader_param(*ccl_node, *this_node, "sheen");
			apply_shader_param(*ccl_node, *this_node, "sheen_tint");
			apply_shader_param(*ccl_node, *this_node, "clearcoat");
			apply_shader_param(*ccl_node, *this_node, "clearcoat_roughness");
			apply_shader_param(*ccl_node, *this_node, "ior");
			apply_shader_param(*ccl_node, *this_node, "transmission");
			apply_shader_param(*ccl_node, *this_node, "transmission_roughness");
			apply_shader_param(*ccl_node, *this_node, "emission");
			apply_shader_param(*ccl_node, *this_node, "emission_strength");
			apply_shader_param(*ccl_node, *this_node, "alpha");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("distribution") }) {
				const csg::PrincipledBSDFDistribution dist{ static_cast<csg::PrincipledBSDFDistribution>(opt_enum->get()) };
				switch (dist) {
				case csg::PrincipledBSDFDistribution::MULTISCATTER_GGX:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_MULTI_GGX_GLASS_ID);
					break;
				case csg::PrincipledBSDFDistribution::GGX:
				default:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_GLASS_ID);
					break;
				}
			}
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("subsurface_method") }) {
				const csg::PrincipledBSDFSubsurfaceMethod dist{ static_cast<csg::PrincipledBSDFSubsurfaceMethod>(opt_enum->get()) };
				switch (dist) {
				case csg::PrincipledBSDFSubsurfaceMethod::RANDOM_WALK:
					ccl_node->set_subsurface_method(ccl::ClosureType::CLOSURE_BSSRDF_PRINCIPLED_RANDOM_WALK_ID);
					break;
				case csg::PrincipledBSDFSubsurfaceMethod::BURLEY:
				default:
					ccl_node->set_subsurface_method(ccl::ClosureType::CLOSURE_BSSRDF_PRINCIPLED_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::PRINCIPLED_HAIR:
		{
			ccl::PrincipledHairBsdfNode* const ccl_node{ new ccl::PrincipledHairBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "melanin");
			apply_shader_param(*ccl_node, *this_node, "melanin_redness");
			apply_shader_param(*ccl_node, *this_node, "tint");
			apply_shader_param(*ccl_node, *this_node, "absorption_coefficient");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			apply_shader_param(*ccl_node, *this_node, "radial_roughness");
			apply_shader_param(*ccl_node, *this_node, "coat");
			apply_shader_param(*ccl_node, *this_node, "ior");
			apply_shader_param(*ccl_node, *this_node, "offset");
			apply_shader_param(*ccl_node, *this_node, "random_roughness");
			apply_shader_param(*ccl_node, *this_node, "random_color");
			apply_shader_param(*ccl_node, *this_node, "offset");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("coloring") }) {
				const csg::PrincipledHairColoring dist{ static_cast<csg::PrincipledHairColoring>(opt_enum->get()) };
				switch (dist) {
				case csg::PrincipledHairColoring::ABSORPTION_COEFFICIENT:
					ccl_node->set_parametrization(ccl::NodePrincipledHairParametrization::NODE_PRINCIPLED_HAIR_DIRECT_ABSORPTION);
					break;
				case csg::PrincipledHairColoring::MELANIN_CONCENTRATION:
					ccl_node->set_parametrization(ccl::NodePrincipledHairParametrization::NODE_PRINCIPLED_HAIR_PIGMENT_CONCENTRATION);
					break;
				case csg::PrincipledHairColoring::DIRECT_COLORING:
				default:
					ccl_node->set_parametrization(ccl::NodePrincipledHairParametrization::NODE_PRINCIPLED_HAIR_REFLECTANCE);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::PRINCIPLED_VOLUME:
		{
			ccl::PrincipledVolumeNode* const ccl_node{ new ccl::PrincipledVolumeNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "density");
			apply_shader_param(*ccl_node, *this_node, "anisotropy");
			apply_shader_param(*ccl_node, *this_node, "absorption_color");
			apply_shader_param(*ccl_node, *this_node, "emission_strength");
			apply_shader_param(*ccl_node, *this_node, "emission_color");
			apply_shader_param(*ccl_node, *this_node, "blackbody_intensity");
			apply_shader_param(*ccl_node, *this_node, "blackbody_tint");
			apply_shader_param(*ccl_node, *this_node, "temperature");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::REFRACTION_BSDF:
		{
			ccl::RefractionBsdfNode* const ccl_node{ new ccl::RefractionBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			apply_shader_param(*ccl_node, *this_node, "IOR");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("distribution") }) {
				const csg::RefractionDistribution dist{ static_cast<csg::RefractionDistribution>(opt_enum->get()) };
				switch (dist) {
				case csg::RefractionDistribution::BECKMANN:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_BECKMANN_REFRACTION_ID);
					break;
				case csg::RefractionDistribution::SHARP:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_REFRACTION_ID);
					break;
				case csg::RefractionDistribution::GGX:
				default:
					ccl_node->set_distribution(ccl::ClosureType::CLOSURE_BSDF_MICROFACET_GGX_REFRACTION_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::SUBSURFACE_SCATTER:
		{
			ccl::SubsurfaceScatteringNode* const ccl_node{ new ccl::SubsurfaceScatteringNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "scale");
			apply_shader_param(*ccl_node, *this_node, "radius");
			apply_shader_param(*ccl_node, *this_node, "sharpness");
			apply_shader_param(*ccl_node, *this_node, "texture_blur");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("falloff") }) {
				const csg::SubsurfaceScatterFalloff dist{ static_cast<csg::SubsurfaceScatterFalloff>(opt_enum->get()) };
				switch (dist) {
				case csg::SubsurfaceScatterFalloff::RANDOM_WALK:
					ccl_node->set_falloff(ccl::ClosureType::CLOSURE_BSSRDF_PRINCIPLED_RANDOM_WALK_ID);
					break;
				case csg::SubsurfaceScatterFalloff::BURLEY:
				default:
					ccl_node->set_falloff(ccl::ClosureType::CLOSURE_BSSRDF_PRINCIPLED_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::TOON_BSDF:
		{
			ccl::ToonBsdfNode* const ccl_node{ new ccl::ToonBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "size");
			apply_shader_param(*ccl_node, *this_node, "smooth");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("component") }) {
				const csg::ToonComponent dist{ static_cast<csg::ToonComponent>(opt_enum->get()) };
				switch (dist) {
				case csg::ToonComponent::GLOSSY:
					ccl_node->set_component(ccl::ClosureType::CLOSURE_BSDF_GLOSSY_TOON_ID);
					break;
				case csg::ToonComponent::DIFFUSE:
				default:
					ccl_node->set_component(ccl::ClosureType::CLOSURE_BSDF_DIFFUSE_TOON_ID);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::TRANSLUCENT_BSDF:
		{
			ccl::TranslucentBsdfNode* const ccl_node{ new ccl::TranslucentBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::TRANSPARENT_BSDF:
		{
			ccl::TransparentBsdfNode* const ccl_node{ new ccl::TransparentBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VELVET_BSDF:
		{
			ccl::VelvetBsdfNode* const ccl_node{ new ccl::VelvetBsdfNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "sigma");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VOL_ABSORPTION:
		{
			ccl::AbsorptionVolumeNode* const ccl_node{ new ccl::AbsorptionVolumeNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "density");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VOL_SCATTER:
		{
			ccl::ScatterVolumeNode* const ccl_node{ new ccl::ScatterVolumeNode() };
			apply_shader_param(*ccl_node, *this_node, "color");
			apply_shader_param(*ccl_node, *this_node, "density");
			apply_shader_param(*ccl_node, *this_node, "anisotropy");
			new_node = ccl_node;
			break;
		}
		//////
		//  Texture
		//////
		case csg::NodeType::MAX_TEXMAP:
		{
			const boost::optional<csg::BoolSlotValue> opt_autosize{ this_node->slot_value_as<csg::BoolSlotValue>("autosize") };

			const boost::optional<csg::IntSlotValue> opt_slot{ this_node->slot_value_as<csg::IntSlotValue>("slot") };
			size_t slot = 1;
			if (opt_slot) {
				slot = static_cast<size_t>(opt_slot->get());
			}

			const boost::optional<csg::EnumSlotValue> opt_precision{ this_node->slot_value_as<csg::EnumSlotValue>("precision") };
			bool use_float = false;
			if (opt_precision) {
				assert(opt_precision->get_meta() == csg::NodeMetaEnum::MAX_TEXMAP_PRECISION);
				use_float = (opt_precision->get() == static_cast<size_t>(csg::MaxTexmapPrecision::FLOAT));
			}

			if (texmap_slots.count(slot) == 1) {
				if (opt_autosize.has_value() == false || opt_autosize->get()) {
					// Default to using autosize
					new_node = texmap_cache.get_node_from_texmap(texmap_slots.at(slot), scene);
				}
				else {
					// Autosize is off, use manual dimensions
					const boost::optional<csg::IntSlotValue> opt_width{ this_node->slot_value_as<csg::IntSlotValue>("width") };
					const boost::optional<csg::IntSlotValue> opt_height{ this_node->slot_value_as<csg::IntSlotValue>("height") };
					if (opt_width && opt_height) {
						new_node = texmap_cache.get_node_from_texmap(texmap_slots.at(slot), scene, opt_width->get(), opt_height->get(), use_float);
					}
				}
			}
			break;
		}
		case csg::NodeType::BRICK_TEX:
		{
			ccl::BrickTextureNode* const ccl_node{ new ccl::BrickTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "offset");
			apply_shader_param(*ccl_node, *this_node, "offset_frequency");
			apply_shader_param(*ccl_node, *this_node, "squash");
			apply_shader_param(*ccl_node, *this_node, "squash_frequency");
			apply_shader_param(*ccl_node, *this_node, "color1");
			apply_shader_param(*ccl_node, *this_node, "color2");
			apply_shader_param(*ccl_node, *this_node, "mortar");
			apply_shader_param(*ccl_node, *this_node, "scale");
			apply_shader_param(*ccl_node, *this_node, "mortar_size");
			apply_shader_param(*ccl_node, *this_node, "mortar_smooth");
			apply_shader_param(*ccl_node, *this_node, "bias");
			apply_shader_param(*ccl_node, *this_node, "brick_width");
			apply_shader_param(*ccl_node, *this_node, "row_height");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::CHECKER_TEX:
		{
			ccl::CheckerTextureNode* const ccl_node{ new ccl::CheckerTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "color1");
			apply_shader_param(*ccl_node, *this_node, "color2");
			apply_shader_param(*ccl_node, *this_node, "scale");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::GRADIENT_TEX:
		{
			ccl::GradientTextureNode* const ccl_node{ new ccl::GradientTextureNode() };
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("gradient_type") }) {
				const csg::GradientTexType type{ static_cast<csg::GradientTexType>(opt_enum->get()) };
				switch (type) {
				case csg::GradientTexType::QUADRATIC:
					ccl_node->set_gradient_type(ccl::NodeGradientType::NODE_BLEND_QUADRATIC);
					break;
				case csg::GradientTexType::EASING:
					ccl_node->set_gradient_type(ccl::NodeGradientType::NODE_BLEND_EASING);
					break;
				case csg::GradientTexType::DIAGONAL:
					ccl_node->set_gradient_type(ccl::NodeGradientType::NODE_BLEND_DIAGONAL);
					break;
				case csg::GradientTexType::RADIAL:
					ccl_node->set_gradient_type(ccl::NodeGradientType::NODE_BLEND_RADIAL);
					break;
				case csg::GradientTexType::QUADRATIC_SPHERE:
					ccl_node->set_gradient_type(ccl::NodeGradientType::NODE_BLEND_QUADRATIC_SPHERE);
					break;
				case csg::GradientTexType::SPHERICAL:
					ccl_node->set_gradient_type(ccl::NodeGradientType::NODE_BLEND_SPHERICAL);
					break;
				case csg::GradientTexType::LINEAR:
				default:
					ccl_node->set_gradient_type(ccl::NodeGradientType::NODE_BLEND_LINEAR);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::MAGIC_TEX:
		{
			ccl::MagicTextureNode* const ccl_node{ new ccl::MagicTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "depth");
			apply_shader_param(*ccl_node, *this_node, "scale");
			apply_shader_param(*ccl_node, *this_node, "distortion");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::MUSGRAVE_TEX:
		{
			ccl::MusgraveTextureNode* const ccl_node{ new ccl::MusgraveTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "w");
			apply_shader_param(*ccl_node, *this_node, "scale");
			apply_shader_param(*ccl_node, *this_node, "detail");
			apply_shader_param(*ccl_node, *this_node, "dimension");
			apply_shader_param(*ccl_node, *this_node, "lacunarity");
			apply_shader_param(*ccl_node, *this_node, "offset");
			apply_shader_param(*ccl_node, *this_node, "gain");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("dimensions") }) {
				const csg::MusgraveTexDimensions dim{ static_cast<csg::MusgraveTexDimensions>(opt_enum->get()) };
				switch (dim) {
				case csg::MusgraveTexDimensions::ONE:
					ccl_node->set_dimensions(1);
					break;
				case csg::MusgraveTexDimensions::TWO:
					ccl_node->set_dimensions(2);
					break;
				case csg::MusgraveTexDimensions::FOUR	:
					ccl_node->set_dimensions(4);
					break;
				case csg::MusgraveTexDimensions::THREE:
				default:
					ccl_node->set_dimensions(3);
					break;
				}
			}
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("musgrave_type") }) {
				const csg::MusgraveTexType type{ static_cast<csg::MusgraveTexType>(opt_enum->get()) };
				switch (type) {
				case csg::MusgraveTexType::MULTIFRACTAL:
					ccl_node->set_musgrave_type(ccl::NodeMusgraveType::NODE_MUSGRAVE_MULTIFRACTAL);
					break;
				case csg::MusgraveTexType::RIDGED_MULTIFRACTAL:
					ccl_node->set_musgrave_type(ccl::NodeMusgraveType::NODE_MUSGRAVE_RIDGED_MULTIFRACTAL);
					break;
				case csg::MusgraveTexType::HYBRID_MULTIFRACTAL:
					ccl_node->set_musgrave_type(ccl::NodeMusgraveType::NODE_MUSGRAVE_HYBRID_MULTIFRACTAL);
					break;
				case csg::MusgraveTexType::HETERO_TERRAIN:
					ccl_node->set_musgrave_type(ccl::NodeMusgraveType::NODE_MUSGRAVE_HETERO_TERRAIN);
					break;
				case csg::MusgraveTexType::FBM:
				default:
					ccl_node->set_musgrave_type(ccl::NodeMusgraveType::NODE_MUSGRAVE_FBM);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::NOISE_TEX:
		{
			ccl::NoiseTextureNode* const ccl_node{ new ccl::NoiseTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "w");
			apply_shader_param(*ccl_node, *this_node, "scale");
			apply_shader_param(*ccl_node, *this_node, "detail");
			apply_shader_param(*ccl_node, *this_node, "roughness");
			apply_shader_param(*ccl_node, *this_node, "distortion");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("dimensions") }) {
				const csg::NoiseTexDimensions dim{ static_cast<csg::NoiseTexDimensions>(opt_enum->get()) };
				switch (dim) {
				case csg::NoiseTexDimensions::ONE:
					ccl_node->set_dimensions(1);
					break;
				case csg::NoiseTexDimensions::TWO:
					ccl_node->set_dimensions(2);
					break;
				case csg::NoiseTexDimensions::FOUR:
					ccl_node->set_dimensions(4);
					break;
				case csg::NoiseTexDimensions::THREE:
				default:
					ccl_node->set_dimensions(3);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VORONOI_TEX:
		{
			ccl::VoronoiTextureNode* const ccl_node{ new ccl::VoronoiTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "w");
			apply_shader_param(*ccl_node, *this_node, "scale");
			apply_shader_param(*ccl_node, *this_node, "smoothness");
			apply_shader_param(*ccl_node, *this_node, "exponent");
			apply_shader_param(*ccl_node, *this_node, "randomness");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("dimensions") }) {
				const csg::VoronoiTexDimensions dim{ static_cast<csg::VoronoiTexDimensions>(opt_enum->get()) };
				switch (dim) {
				case csg::VoronoiTexDimensions::ONE:
					ccl_node->set_dimensions(1);
					break;
				case csg::VoronoiTexDimensions::TWO:
					ccl_node->set_dimensions(2);
					break;
				case csg::VoronoiTexDimensions::FOUR:
					ccl_node->set_dimensions(4);
					break;
				case csg::VoronoiTexDimensions::THREE:
				default:
					ccl_node->set_dimensions(3);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::WAVE_TEX:
		{
			ccl::WaveTextureNode* const ccl_node{ new ccl::WaveTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "scale");
			apply_shader_param(*ccl_node, *this_node, "distortion");
			apply_shader_param(*ccl_node, *this_node, "detail");
			apply_shader_param(*ccl_node, *this_node, "detail_scale");
			apply_shader_param(*ccl_node, *this_node, "detail_roughness");
			apply_shader_param(*ccl_node, *this_node, "phase");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("wave_type") }) {
				const csg::WaveTexType type{ static_cast<csg::WaveTexType>(opt_enum->get()) };
				switch (type) {
				case csg::WaveTexType::RINGS:
					ccl_node->set_wave_type(ccl::NodeWaveType::NODE_WAVE_RINGS);
					break;
				case csg::WaveTexType::BANDS:
				default:
					ccl_node->set_wave_type(ccl::NodeWaveType::NODE_WAVE_BANDS);
					break;
				}
			}
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("direction") }) {
				const csg::WaveTexDirection direction{ static_cast<csg::WaveTexDirection>(opt_enum->get()) };
				switch (direction) {
				case csg::WaveTexDirection::DIAGONAL:
					ccl_node->set_bands_direction(ccl::NodeWaveBandsDirection::NODE_WAVE_BANDS_DIRECTION_DIAGONAL);
					ccl_node->set_rings_direction(ccl::NodeWaveRingsDirection::NODE_WAVE_RINGS_DIRECTION_SPHERICAL);
					break;
				case csg::WaveTexDirection::Y:
					ccl_node->set_bands_direction(ccl::NodeWaveBandsDirection::NODE_WAVE_BANDS_DIRECTION_Y);
					ccl_node->set_rings_direction(ccl::NodeWaveRingsDirection::NODE_WAVE_RINGS_DIRECTION_Y);
					break;
				case csg::WaveTexDirection::Z:
					ccl_node->set_bands_direction(ccl::NodeWaveBandsDirection::NODE_WAVE_BANDS_DIRECTION_Z);
					ccl_node->set_rings_direction(ccl::NodeWaveRingsDirection::NODE_WAVE_RINGS_DIRECTION_Z);
					break;
				case csg::WaveTexDirection::X:
				default:
					ccl_node->set_bands_direction(ccl::NodeWaveBandsDirection::NODE_WAVE_BANDS_DIRECTION_X);
					ccl_node->set_rings_direction(ccl::NodeWaveRingsDirection::NODE_WAVE_RINGS_DIRECTION_X);
					break;
				}
			}
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("profile") }) {
				const csg::WaveTexProfile profile{ static_cast<csg::WaveTexProfile>(opt_enum->get()) };
				switch (profile) {
				case csg::WaveTexProfile::SAW:
					ccl_node->set_profile(ccl::NodeWaveProfile::NODE_WAVE_PROFILE_SAW);
					break;
				case csg::WaveTexProfile::TRIANGLE:
					ccl_node->set_profile(ccl::NodeWaveProfile::NODE_WAVE_PROFILE_TRI);
					break;
				case csg::WaveTexProfile::SINE:
				default:
					ccl_node->set_profile(ccl::NodeWaveProfile::NODE_WAVE_PROFILE_SIN);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::WHITE_NOISE_TEX:
		{
			ccl::WhiteNoiseTextureNode* const ccl_node{ new ccl::WhiteNoiseTextureNode() };
			apply_shader_param(*ccl_node, *this_node, "w");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("dimensions") }) {
				const csg::WhiteNoiseTexDimensions dim{ static_cast<csg::WhiteNoiseTexDimensions>(opt_enum->get()) };
				switch (dim) {
				case csg::WhiteNoiseTexDimensions::ONE:
					ccl_node->set_dimensions(1);
					break;
				case csg::WhiteNoiseTexDimensions::TWO:
					ccl_node->set_dimensions(2);
					break;
				case csg::WhiteNoiseTexDimensions::FOUR:
					ccl_node->set_dimensions(4);
					break;
				case csg::WhiteNoiseTexDimensions::THREE:
				default:
					ccl_node->set_dimensions(3);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		//////
		// Vector
		//////
		case csg::NodeType::BUMP:
		{
			ccl::BumpNode* const ccl_node{ new ccl::BumpNode() };
			apply_shader_param(*ccl_node, *this_node, "invert");
			apply_shader_param(*ccl_node, *this_node, "strength");
			apply_shader_param(*ccl_node, *this_node, "distance");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::DISPLACEMENT:
		{
			ccl::DisplacementNode* const ccl_node{ new ccl::DisplacementNode() };
			apply_shader_param(*ccl_node, *this_node, "height");
			apply_shader_param(*ccl_node, *this_node, "midlevel");
			apply_shader_param(*ccl_node, *this_node, "scale");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("dimensions") }) {
				const csg::DisplacementSpace space{ static_cast<csg::DisplacementSpace>(opt_enum->get()) };
				switch (space) {
				case csg::DisplacementSpace::WORLD:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_WORLD);
					break;
				case csg::DisplacementSpace::OBJECT:
				default:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_OBJECT);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::MAPPING:
		{
			ccl::MappingNode* const ccl_node{ new ccl::MappingNode() };
			apply_shader_param(*ccl_node, *this_node, "vector");
			apply_shader_param(*ccl_node, *this_node, "location");
			apply_shader_param(*ccl_node, *this_node, "rotation");
			apply_shader_param(*ccl_node, *this_node, "scale");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("mapping_type") }) {
				const csg::VectorMappingType type{ static_cast<csg::VectorMappingType>(opt_enum->get()) };
				switch (type) {
				case csg::VectorMappingType::TEXTURE:
					ccl_node->set_mapping_type(ccl::NodeMappingType::NODE_MAPPING_TYPE_TEXTURE);
					break;
				case csg::VectorMappingType::VECTOR:
					ccl_node->set_mapping_type(ccl::NodeMappingType::NODE_MAPPING_TYPE_VECTOR);
					break;
				case csg::VectorMappingType::NORMAL:
					ccl_node->set_mapping_type(ccl::NodeMappingType::NODE_MAPPING_TYPE_NORMAL);
					break;
				case csg::VectorMappingType::POINT:
				default:
					ccl_node->set_mapping_type(ccl::NodeMappingType::NODE_MAPPING_TYPE_POINT);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::NORMAL:
		{
			ccl::NormalNode* const ccl_node{ new ccl::NormalNode() };
			apply_shader_param(*ccl_node, *this_node, "direction");
			apply_shader_param(*ccl_node, *this_node, "normal");
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::NORMAL_MAP:
		{
			ccl::NormalMapNode* const ccl_node{ new ccl::NormalMapNode() };
			apply_shader_param(*ccl_node, *this_node, "strength");
			apply_shader_param(*ccl_node, *this_node, "color");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("dimensions") }) {
				const csg::NormalMapSpace space{ static_cast<csg::NormalMapSpace>(opt_enum->get()) };
				switch (space) {
				case csg::NormalMapSpace::OBJECT:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_OBJECT);
					break;
				case csg::NormalMapSpace::WORLD:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_WORLD);
					break;
				case csg::NormalMapSpace::TANGENT:
				default:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_TANGENT);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VECTOR_CURVES:
		{
			ccl::VectorCurvesNode* const ccl_node{ new ccl::VectorCurvesNode() };
			apply_shader_param(*ccl_node, *this_node, "fac");
			apply_shader_param(*ccl_node, *this_node, "vector");
			if (const auto opt_curves{ this_node->slot_value_as<csg::VectorCurveSlotValue>("curves") }) {
				const csg::Curve curve_x{ opt_curves->get_x() };
				const csg::Curve curve_y{ opt_curves->get_y() };
				const csg::Curve curve_z{ opt_curves->get_z() };
				ccl::array<ccl::float3> curves_array;
				curves_array.resize(RAMP_TABLE_SIZE);
				for (size_t i = 0; i < RAMP_TABLE_SIZE; i++) {
					const float t{ static_cast<float>(i) / static_cast<float>(RAMP_TABLE_SIZE - 1) };
					const float x{ curve_x.eval_point(t) };
					const float y{ curve_y.eval_point(t) };
					const float z{ curve_z.eval_point(t) };
					curves_array[i] = ccl::make_float3(x, y, z);
				}
				ccl_node->set_curves(curves_array);
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VECTOR_DISPLACEMENT:
		{
			ccl::VectorDisplacementNode* const ccl_node{ new ccl::VectorDisplacementNode() };
			apply_shader_param(*ccl_node, *this_node, "midlevel");
			apply_shader_param(*ccl_node, *this_node, "scale");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("dimensions") }) {
				const csg::VectorDisplacementSpace space{ static_cast<csg::VectorDisplacementSpace>(opt_enum->get()) };
				switch (space) {
				case csg::VectorDisplacementSpace::OBJECT:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_OBJECT);
					break;
				case csg::VectorDisplacementSpace::WORLD:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_WORLD);
					break;
				case csg::VectorDisplacementSpace::TANGENT:
				default:
					ccl_node->set_space(ccl::NodeNormalMapSpace::NODE_NORMAL_MAP_TANGENT);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		case csg::NodeType::VECTOR_TRANSFORM:
		{
			ccl::VectorTransformNode* const ccl_node{ new ccl::VectorTransformNode() };
			apply_shader_param(*ccl_node, *this_node, "vector");
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("transform_type") }) {
				const csg::VectorTransformType type{ static_cast<csg::VectorTransformType>(opt_enum->get()) };
				switch (type) {
				case csg::VectorTransformType::VECTOR:
					ccl_node->set_transform_type(ccl::NodeVectorTransformType::NODE_VECTOR_TRANSFORM_TYPE_VECTOR);
					break;
				case csg::VectorTransformType::NORMAL:
					ccl_node->set_transform_type(ccl::NodeVectorTransformType::NODE_VECTOR_TRANSFORM_TYPE_NORMAL);
					break;
				case csg::VectorTransformType::POINT:
				default:
					ccl_node->set_transform_type(ccl::NodeVectorTransformType::NODE_VECTOR_TRANSFORM_TYPE_POINT);
					break;
				}
			}
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("convert_from") }) {
				const csg::VectorTransformSpace space{ static_cast<csg::VectorTransformSpace>(opt_enum->get()) };
				switch (space) {
				case csg::VectorTransformSpace::CAMERA:
					ccl_node->set_convert_from(ccl::NodeVectorTransformConvertSpace::NODE_VECTOR_TRANSFORM_CONVERT_SPACE_CAMERA);
					break;
				case csg::VectorTransformSpace::OBJECT:
					ccl_node->set_convert_from(ccl::NodeVectorTransformConvertSpace::NODE_VECTOR_TRANSFORM_CONVERT_SPACE_OBJECT);
					break;
				case csg::VectorTransformSpace::WORLD:
				default:
					ccl_node->set_convert_from(ccl::NodeVectorTransformConvertSpace::NODE_VECTOR_TRANSFORM_CONVERT_SPACE_WORLD);
					break;
				}
			}
			if (const auto opt_enum{ this_node->slot_value_as<csg::EnumSlotValue>("convert_to") }) {
				const csg::VectorTransformSpace space{ static_cast<csg::VectorTransformSpace>(opt_enum->get()) };
				switch (space) {
				case csg::VectorTransformSpace::CAMERA:
					ccl_node->set_convert_to(ccl::NodeVectorTransformConvertSpace::NODE_VECTOR_TRANSFORM_CONVERT_SPACE_CAMERA);
					break;
				case csg::VectorTransformSpace::WORLD:
					ccl_node->set_convert_to(ccl::NodeVectorTransformConvertSpace::NODE_VECTOR_TRANSFORM_CONVERT_SPACE_WORLD);
					break;
				case csg::VectorTransformSpace::OBJECT:
				default:
					ccl_node->set_convert_to(ccl::NodeVectorTransformConvertSpace::NODE_VECTOR_TRANSFORM_CONVERT_SPACE_OBJECT);
					break;
				}
			}
			new_node = ccl_node;
			break;
		}
		default:
			; // Do nothing
		}
		if (new_node) {
			cycles_graph->add(new_node);
			nodes_by_id[this_node->id()] = new_node;
		}
	}



	for (const csg::Connection this_connection : opt_graph->connections()) {

		*logger << "Adding connection..." << LogCtl::WRITE_LINE;

		if (nodes_by_id.count(this_connection.source().node_id()) == 0 || nodes_by_id.count(this_connection.dest().node_id()) == 0) {
			// One of the two nodes in this connection does not exist, go next
			continue;
		}

		ccl::ShaderNode* const ccl_node_src{ nodes_by_id[this_connection.source().node_id()] };
		ccl::ShaderNode* const ccl_node_dest{ nodes_by_id[this_connection.dest().node_id()] };

		std::shared_ptr<const csg::Node> node_src{ opt_graph->get(this_connection.source().node_id()) };
		assert(node_src.use_count() != 0);
		std::shared_ptr<const csg::Node> node_dest{ opt_graph->get(this_connection.dest().node_id()) };
		assert(node_dest.use_count() != 0);

		const boost::optional<csg::Slot> opt_slot_src{ node_src->slot(this_connection.source().index()) };
		if (opt_slot_src.has_value() == false) {
			*logger << "Can't find source slot, skipping connection" << LogCtl::WRITE_LINE;
			continue;
		}
		const boost::optional<csg::Slot> opt_slot_dest{ node_dest->slot(this_connection.dest().index()) };
		if (opt_slot_dest.has_value() == false) {
			*logger << "Can't find dest slot, skipping connection" << LogCtl::WRITE_LINE;
			continue;
		}

		const char* const src_name{ opt_slot_src->disp_name() };
		const char* const dest_name{ opt_slot_dest->disp_name() };

		ccl::ShaderOutput* source{ ccl_node_src->output(src_name) };
		ccl::ShaderInput* dest{ ccl_node_dest->input(dest_name) };

		if (source == nullptr || dest == nullptr) {
			*logger << "Can't find ccl input/output, skipping connection" << LogCtl::WRITE_LINE;
			continue;
		}

		cycles_graph->connect(source, dest);
		*logger << "Connection complete" << LogCtl::WRITE_LINE;
	}

	// TODO: Add special default connections

	return cycles_graph;
}


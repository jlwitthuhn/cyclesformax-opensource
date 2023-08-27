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
 
#include "util_translate_light.h"

#include <cmath>

#include <render/light.h>
#include <render/scene.h>

#include <genlight.h>
#include <lslights.h>
#include <object.h>

#include "const_classid.h"
#include "rend_logger.h"
#include "rend_logger_ext.h"
#include "rend_shader_manager.h"
#include "util_matrix_max.h"
#include "util_pblock_dump.h"

constexpr float PI = 3.14159f;

static CyclesLightType from_max_light_type(const int type)
{
	switch (type) {
	case OMNI_LIGHT:
		return CyclesLightType::POINT;
	case DIR_LIGHT:
		return CyclesLightType::DIRECT;
		return CyclesLightType::DIRECT;
	case FSPOT_LIGHT:
	case TSPOT_LIGHT:
		return CyclesLightType::SPOT;
	case LightscapeLight::TARGET_POINT_TYPE:
		return CyclesLightType::POINT;
	case LightscapeLight::POINT_TYPE:
		return CyclesLightType::POINT;
	case LightscapeLight::TARGET_SPHERE_TYPE:
		return CyclesLightType::SPHERE;
	case LightscapeLight::SPHERE_TYPE:
		return CyclesLightType::SPHERE;
	}
	return CyclesLightType::INVALID;
}

static bool is_fstorm_light_class(const Class_ID& class_id)
{
	if (class_id == FSTORM_LIGHT_CLASS) {
		return true;
	}
	else if (class_id == FSTORM_SUN_CLASS) {
		return true;
	}
	return false;
}

CyclesLightParams get_light_params(INode* const node, Object* const object, const TimeValue t)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilLightGet", false, true);
	*logger << LogCtl::SEPARATOR;
	*logger << "Translating light node: " << node->GetName() << LogCtl::WRITE_LINE;
	*logger << "with class id: " << object->ClassID() << LogCtl::WRITE_LINE;

	CyclesLightParams result;
	if (object->SuperClassID() != LIGHT_CLASS_ID) {
		return result;
	}

	const Matrix3 light_tfm = node->GetObjTMAfterWSM(t);
	const Point3 node_translation = light_tfm.GetTrans();

	const ccl::Transform ccl_tfm = cycles_transform_from_max_matrix(light_tfm);
	result.tfm = ccl_tfm;

	*logger << "Getting light state..." << LogCtl::WRITE_LINE;
	
	LightState light_state;
	if (LightObject* const light_obj = dynamic_cast<LightObject*>(object)) {
		Interval light_state_valid = FOREVER;
		RefResult ref_result = light_obj->EvalLightState(t, light_state_valid, &light_state);
		if (ref_result != REF_SUCCEED) {
			return result;
		}
	}
	else {
		return result;
	}

	if (GenLight* const gen_light = dynamic_cast<GenLight*>(object)) {
		*logger << "gen_light->Type(): " << gen_light->Type() << LogCtl::WRITE_LINE;

		result.type = from_max_light_type(gen_light->Type());
		// Sometimes the above function isn't enough to determine type
		if (gen_light->IsSpot()) {
			result.type = CyclesLightType::SPOT;
		}
		if (gen_light->IsDir()) {
			result.type = CyclesLightType::DIRECT;
		}
		if (result.type == CyclesLightType::SPOT) {
			*logger << "hotspot: " << gen_light->GetHotspot(t) << LogCtl::WRITE_LINE;
			*logger << "fallsize: " << gen_light->GetFallsize(t) << LogCtl::WRITE_LINE;
			result.spot_angle = gen_light->GetFallsize(t) * PI / 180.0f;
			const float smooth_tmp = 1.0f - gen_light->GetHotspot(t) / gen_light->GetFallsize(t);
			result.spot_smooth = pow(smooth_tmp, 0.6f);
			*logger << "smooth: " << result.spot_smooth << LogCtl::WRITE_LINE;
		}
		if (result.type == CyclesLightType::INVALID) {
			return result;
		}
	}
	else {
		return result;
	}

	if (LightscapeLight* const ls_light = dynamic_cast<LightscapeLight*>(object)) {
		if (result.type == CyclesLightType::SPHERE) {
			result.size = ls_light->GetRadius(t);
		}
	}

	*logger << "building result..." << LogCtl::WRITE_LINE;

	result.intensity = light_state.intens * 61000.0f;
	result.color = ccl::make_float3(light_state.color.r, light_state.color.g, light_state.color.b);
	result.shadows_enabled = (light_state.shadow == TRUE);
	result.active = (light_state.on != 0);

	*logger << "intensity: " << light_state.intens << LogCtl::WRITE_LINE;
	*logger << "color: " << result.color << LogCtl::WRITE_LINE;
	*logger << "active: " << result.active << LogCtl::WRITE_LINE;

	if (is_fstorm_light_class(object->ClassID())) {
		*logger << "this is an unsupported fstorm light, setting as errored" << LogCtl::WRITE_LINE;
		result.errored = true;
		result.error_string = std::wstring(L"Ignoring unsupported light object: ") + node->GetName();
	}

	*logger << "complete" << LogCtl::WRITE_LINE;

	return result;
}

static ccl::Light* new_light()
{
	ccl::Light* const result = new ccl::Light();

	result->set_tfm(ccl::transform_identity());

	result->set_light_type(ccl::LightType::LIGHT_POINT);
	result->set_co(ccl::make_float3(0.0f, 0.0f, 0.0f));

	result->set_dir(ccl::make_float3(0.0f, 0.0f, 0.0f));
	result->set_size(0.0f);

	result->set_axisu(ccl::make_float3(0.0f, 0.0f, 0.0f));
	result->set_sizeu(0.0f);
	result->set_axisv(ccl::make_float3(0.0f, 0.0f, 0.0f));
	result->set_sizev(0.0f);

	result->set_map_resolution(512);

	result->set_spot_angle(3.14159f / 4.0f);
	result->set_spot_smooth(0.0f);

	result->set_cast_shadow(true);
	result->set_use_mis(false);
	result->set_use_diffuse(true);
	result->set_use_glossy(true);
	result->set_use_transmission(true);
	result->set_use_scatter(true);

	result->set_samples(1);
	result->set_max_bounces(1024);

	result->set_is_portal(false);
	result->set_is_enabled(true);

	result->set_shader(nullptr);

	return result;
}

void add_light_to_scene(ccl::Scene* const scene, const std::unique_ptr<MaxShaderManager>& shader_manager, const CyclesLightParams light_params, const float point_light_size)
{
	const std::unique_ptr<LoggerInterface> logger = global_log_manager.new_logger(L"UtilLightAdd", false, true);
	*logger << LogCtl::SEPARATOR;

	if (light_params.active == false || light_params.type == CyclesLightType::INVALID) {
		// Nothing to add
		return;
	}

	*logger << "adding light..." << LogCtl::WRITE_LINE;

	float light_intensity = light_params.intensity;
	if (light_params.type == CyclesLightType::DIRECT) {
		light_intensity /= 61000.0f;
	}

	const int light_shader_index = shader_manager->get_light_shader(light_params.color, light_intensity);
	ccl::Light* const light = new_light();
	light->set_shader(scene->shaders[light_shader_index]);

	light->set_tfm(light_params.tfm);
	light->set_co(ccl::transform_get_column(&(light_params.tfm), 3));
	light->set_dir(-1.0f * ccl::transform_get_column(&(light_params.tfm), 2));

	if (light_params.type == CyclesLightType::POINT) {
		light->set_light_type(ccl::LightType::LIGHT_POINT);
		light->set_size(point_light_size);
	}
	else if (light_params.type == CyclesLightType::SPHERE) {
		light->set_light_type(ccl::LightType::LIGHT_POINT);
		light->set_size(light_params.size);
	}
	else if (light_params.type == CyclesLightType::DIRECT) {
		light->set_light_type(ccl::LightType::LIGHT_DISTANT);
		light->set_size(0.0f);
	}
	else if (light_params.type == CyclesLightType::SPOT) {
		light->set_light_type(ccl::LightType::LIGHT_SPOT);
		light->set_spot_angle(light_params.spot_angle);
		light->set_spot_smooth(light_params.spot_smooth);
		light->set_size(point_light_size);
	}
	else {
		delete light;
		return;
	}

	light->set_cast_shadow(light_params.shadows_enabled);

	light->tag_update(scene);

	scene->lights.push_back(light);

	*logger << "complete" << LogCtl::WRITE_LINE;
}

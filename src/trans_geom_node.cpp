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
 
#include "trans_geom_node.h"

#include <inode.h>
#include <modstack.h>
#include <NotificationAPI/NotificationAPI_Events.h>
#include <object.h>
#include <RenderingAPI/Renderer/IRenderSessionContext.h>

#include "const_classid.h"
#include "plugin_mod_properties.h"
#include "util_translate_geometry.h"

using MaxSDK::NotificationAPI::NotifierType;

using MaxSDK::RenderingAPI::IRenderSessionContext;
using MaxSDK::RenderingAPI::TranslationResult;
using MaxSDK::RenderingAPI::Translator;
using MaxSDK::RenderingAPI::TranslatorGraphNode;

static ccl::float3 get_float3_from_colorref(COLORREF input)
{
	ccl::float3 output;
	output.x = GetRValue(input) / 255.0f;
	output.y = GetGValue(input) / 255.0f;
	output.z = GetBValue(input) / 255.0f;

	return output;
}

CyclesGeomNodeTranslator::CyclesGeomNodeTranslator(const TranslatorKey& key, MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node) :
	BaseTranslator_INode(key, NotifierType::NotifierType_Node_Geom, translator_graph_node),
	Translator(translator_graph_node),
	logger(global_log_manager.new_logger(L"CyclesGeomNodeTranslator", false, true))
{
	*logger << LogCtl::SEPARATOR;
}

CyclesGeomNodeTranslator::~CyclesGeomNodeTranslator()
{

}

bool CyclesGeomNodeTranslator::GetGeomObject(CyclesGeomObject& geom_object) const
{
	if (GetNumOutputs() != 1) {
		return false;
	}

	geom_object = GetOutput_SimpleValue<CyclesGeomObject>(0, geom_object);
	return true;
}

#if PLUGIN_SDK_VERSION >= 2018
MaxSDK::RenderingAPI::TranslationResult CyclesGeomNodeTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/, KeyframeList& /*keyframes_needed*/)
#elif PLUGIN_SDK_VERSION == 2017
MaxSDK::RenderingAPI::TranslationResult CyclesGeomNodeTranslator::Translate(const TimeValue translation_time, Interval& new_validity, MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/)
#else
static_assert(false);
#endif
{
	*logger << "Translate called..." << LogCtl::WRITE_LINE;

	GetRenderSessionContext().CallRenderBegin(GetNode(), translation_time);

	ObjectState os = GetNode().EvalWorldState(translation_time);

	if (os.obj == nullptr || os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) {
		ResetAllOutputs();
		return TranslationResult::Success;
	}

	GeomObject* const geom_obj = dynamic_cast<GeomObject*>(os.obj);

	if (geom_obj->IsRenderable() == false) {
		*logger << "IsRenderable() is false..." << LogCtl::WRITE_LINE;
		ResetAllOutputs();
		return TranslationResult::Success;
	}

	Interval view_valid = FOREVER;
	View& mesh_view = const_cast<View&>(GetRenderSessionContext().GetCamera().GetView(translation_time, view_valid));

	std::vector<int> empty_motion_sample_vector;
	std::shared_ptr<MeshGeometryObj> mesh_geom = get_mesh_geometry(&(GetNode()), mesh_view, translation_time, empty_motion_sample_vector);

	INode* const node_ptr = &(GetNode());
	CyclesGeomObject result = get_geom_object(translation_time, node_ptr, std::vector<int>());

	result.mesh_geometry = mesh_geom;
	result.mtl = GetNode().GetMtl();
	result.wire_color = get_float3_from_colorref(GetNode().GetWireColor());

	SetOutput_SimpleValue<CyclesGeomObject>(0, result);

	new_validity = geom_obj->ObjectValidity(translation_time);

	*logger << "Translate complete" << LogCtl::WRITE_LINE;

	return TranslationResult::Success;
}

MaxSDK::RenderingAPI::TranslationResult CyclesGeomNodeTranslator::TranslateKeyframe(
	const TimeValue /*t*/,
	const TimeValue /*keyframe_t*/,
	MaxSDK::RenderingAPI::ITranslationProgress& /*translation_progress*/
	)
{
	return MaxSDK::RenderingAPI::TranslationResult::Success;
}

void CyclesGeomNodeTranslator::AccumulateStatistics(MaxSDK::RenderingAPI::TranslatorStatistics& /*stats*/) const
{
	// Do nothing
}

Interval CyclesGeomNodeTranslator::CheckValidity(const TimeValue /*t*/, const Interval& /*previous_validity*/) const
{
	return NEVER;
}

MSTR CyclesGeomNodeTranslator::GetTimingCategory() const
{
	return L"GeomNode";
}

bool CyclesGeomNodeTranslator::CareAboutMissingUVWChannels() const
{
	return true;
}

std::vector<unsigned int> CyclesGeomNodeTranslator::GetMeshUVWChannelIDs() const
{
	return uv_channels_present;
}

std::vector<MtlID> CyclesGeomNodeTranslator::GetMeshMaterialIDs() const
{
	return mtl_ids_present;
}

std::unique_ptr<MaxSDK::RenderingAPI::Translator> CyclesGeomNodeTranslator::Allocator::operator()(
	const TranslatorKey& key,
	MaxSDK::RenderingAPI::TranslatorGraphNode& translator_graph_node,
	const MaxSDK::RenderingAPI::IRenderSessionContext& /*render_session_context*/,
	const TimeValue /*initial_time*/
	) const
{
	return std::make_unique<CyclesGeomNodeTranslator>(key, translator_graph_node);
}

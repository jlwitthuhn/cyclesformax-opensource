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
* @brief Defines classes and macros used by (almost) all material plugins classes.
*/

#include <vector>

#include <IMaterialViewportShading.h>

#include "rend_logger.h"
#include "util_enums.h"

class ClassDesc2;

class CyclesPluginMatBase;
class NormalMapDescriptor;

/**
 * @brief Enum to specify the output type of a given material.
 */
enum class ClosureType {
	SURFACE,
	VOLUME,
	SPECIAL,
	ANY
};

/**
 * @brief Implementation of IMaterialViewportShading that is used to provide basic viewport shading for all plugin
 * materials.
 */
class CyclesMaterialViewportShading : public IMaterialViewportShading {
public:
	CyclesMaterialViewportShading(CyclesPluginMatBase* mat_in);
	virtual ~CyclesMaterialViewportShading();

	virtual bool IsShadingModelSupported(ShadingModel model) const;
	virtual ShadingModel GetCurrentShadingModel() const;
	virtual bool SetCurrentShadingModel(ShadingModel model);
	virtual int GetSupportedMapLevels() const;

private:
	CyclesPluginMatBase* mat_ptr;
};

/**
 * @brief Class used to track information about one of a material's paramblocks.
 */
class ParamBlockSlotDescriptor {
public:
	IParamBlock2* pblock = nullptr;
	int ref = -1;
	TSTR name = nullptr;
};

/**
 * @brief Description of a sub-texmap slot that is part of a plugin material.
 */
class SubtexSlotDescriptor {
public:
	MSTR display_name;
	int slot_type = MAPSLOT_TEXTURE;
	int param_id = -1;
	int enabled_param_id = -1;
	int subtex_id = -1;
	int pblock_ref = 0;
};

/**
* @brief Description of a sub-material slot that is part of a plugin material.
*/
class SubmatSlotDescriptor {
public:
	SubmatSlotDescriptor();

	MSTR display_name;
	int param_id;
	int submat_id;
	ClosureType accepted_closure_type;
};

/**
 * @brief The base class that all plugin materials derive from. Implements common plugin material functionality.
 */
class CyclesPluginMatBase : public Mtl, public CyclesMaterialViewportShading {
public:
	// Functions defined by this class
	virtual ClassDesc2* GetClassDesc() const = 0;
	virtual ParamBlockDesc2* GetParamBlockDesc() = 0;
	virtual ParamBlockDesc2* GetNormalParamBlockDesc();
	virtual void PopulateSubmatSlots() = 0;
	virtual void PopulateSubtexSlots() = 0;

	virtual void PopulateNormalMapDesc(NormalMapDescriptor* desc, TimeValue t);
	
	virtual ClosureType GetClosureType() const;

	virtual void UpdateVPTexmap(int subtex_id);

	// Mtl functions
	virtual ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);
	virtual Interval Validity(TimeValue t);
	virtual void Reset();

	// Parameters for interactive render
	virtual void SetAmbient(Color c, TimeValue t);
	virtual void SetDiffuse(Color c, TimeValue t);
	virtual void SetSpecular(Color c, TimeValue t);
	virtual void SetShininess(float v, TimeValue t);
	virtual Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
	virtual Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
	virtual Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
	virtual float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
	virtual float GetShininess(int mtlNum=0, BOOL backFace=FALSE);
	virtual float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
	virtual float WireSize(int mtlNum=0, BOOL backFace=FALSE);
	
	// Extended parameters for interactive render
	virtual float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE);
	virtual Color GetSelfIllumColor(int mtlNum=0, BOOL backFace=FALSE);

	// MtlBase
	virtual void Update(TimeValue t, Interval& valid) = 0;
	
	// ISubMap virtual functions
	virtual int NumSubTexmaps();
	virtual Texmap* GetSubTexmap(int i);
	virtual int MapSlotType(int i);
	virtual void SetSubTexmap(int i, Texmap* m);
	virtual int SubTexmapOn(int i);
#if PLUGIN_SDK_VERSION < 2022
	virtual MSTR GetSubTexmapSlotName(int i) override;
#else
	virtual MSTR GetSubTexmapSlotName(int i, bool localized) override;
#endif

	// Shading and displacement, these do nothing
	virtual void Shade(ShadeContext& sc);
	virtual float EvalDisplacement(ShadeContext& sc);
	virtual Interval DisplacementValidity(TimeValue t);

	// Submaterials
	virtual int NumSubMtls();
	virtual Mtl* GetSubMtl(int i);
	virtual void SetSubMtl(int i, Mtl* m);
	virtual int VPDisplaySubMtl();
#if PLUGIN_SDK_VERSION < 2022
	virtual MSTR GetSubMtlSlotName(int i) override;
#else
	virtual MSTR GetSubMtlSlotName(int i, bool localized) override;
#endif

	// VP Display
	virtual BOOL SupportTexDisplay();
	virtual BOOL SupportMultiMapsInViewport();
	
	// What
	virtual BOOL SetDlgThing(ParamDlg* dlg);
	
	// I/O, this must be re-implemented by a child class only if it needs to save something outside of the pblock
	virtual IOResult Load(ILoad *iload);
	virtual IOResult Save(ISave *isave);

	// From Animatable
	virtual SClass_ID SuperClassID();
	virtual Class_ID ClassID();
#if PLUGIN_SDK_VERSION < 2022
	virtual void GetClassName(MSTR& s) override;
#else
	virtual void GetClassName(MSTR& s, bool localized) const override;
#endif

	virtual RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);

	virtual int NumSubs();
	virtual Animatable* SubAnim(int i);
#if PLUGIN_SDK_VERSION < 2022
	virtual MSTR SubAnimName(int i);
#else
	virtual MSTR SubAnimName(int i, bool localized);
#endif

	virtual int NumRefs();
	virtual RefTargetHandle GetReference(int i);

	virtual int NumParamBlocks();
	virtual IParamBlock2* GetParamBlock(int i);
	virtual IParamBlock2* GetParamBlockByID(BlockID id);

	virtual RefTargetHandle Clone(RemapDir &remap) override;

	virtual void DeleteThis();

	// InterfaceServer
	virtual BaseInterface* GetInterface(Interface_ID id) override;

protected:
	CyclesPluginMatBase(bool supports_viewport_texmap, bool use_common_normal_map, int main_pblock_ref = 0, int normal_pblock_ref = 1);

	virtual void SetReference(int i, RefTargetHandle rtarg) override;

	virtual void Initialize();
	virtual void UpdateSubThings(TimeValue t, Interval& iv);

	virtual void SetupCommonNormalStuff();

	IParamBlock2* pblock = nullptr;
	Interval valid_interval;

	const int main_pblock_ref;
	const int normal_pblock_ref;

	std::vector<ParamBlockSlotDescriptor> pblock_vec;

	Color color_ambient;
	Color color_diffuse;
	Color color_specular;
	float transparency;
	float shininess;
	float shine_strength;
	float wire_size;
	float self_illum;
	Color self_illum_color;
	
	ClosureType node_type;

	bool supports_viewport_tex;
	int viewport_submat_primary;
	int viewport_submat_secondary;

	std::vector<SubmatSlotDescriptor> submat_slots;
	std::vector<SubtexSlotDescriptor> subtex_slots;

	// Normal map support
	const bool use_common_normal_map;
	int subtex_normal_strength = 0;
	int subtex_normal_color = 0;

	const std::unique_ptr<LoggerInterface> base_logger;
};

/**
 * @brief Macro to generate an enum for normal paramblock parameters.
 */
#define COMMON_NORMAL_ENUMS(PARAM_PREFIX) \
enum { PARAM_PREFIX ## _enable, PARAM_PREFIX ## _space, PARAM_PREFIX ## _strength, PARAM_PREFIX ## _strength_map, PARAM_PREFIX ## _strength_map_on, PARAM_PREFIX ## _color, PARAM_PREFIX ## _color_map, PARAM_PREFIX ## _color_map_on, PARAM_PREFIX ## _invert_green }; \

/**
 * @brief Macro to generate a normal ParamBlockDesc2.
 */
#define MAKE_NORMAL_PBLOCK(PBLOCK_INDEX,CLASSDESC,PBLOCK_NAME,PANEL_TITLE_IDS,VAR_NAME,PARAM_PREFIX) \
COMMON_NORMAL_ENUMS(PARAM_PREFIX) \
static ParamBlockDesc2 VAR_NAME( \
	/* Pblock data */ \
	PBLOCK_INDEX, \
	_T(PBLOCK_NAME), \
	0, \
	CLASSDESC, \
	P_AUTO_CONSTRUCT + P_AUTO_UI, \
	PBLOCK_INDEX, \
	/* Interface stuff */ \
	IDD_PANEL_COMMON_NORMAL, \
	PANEL_TITLE_IDS, \
	0, \
	0, \
	NULL, \
		PARAM_PREFIX ## _enable, \
		_T("enabled"), \
		TYPE_BOOL, \
		0, \
		IDS_ENABLE_NORMAL_MAP, \
		p_default, FALSE, \
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_ENABLE_NORMAL_MAP, \
		p_end, \
		/* Space */ \
		PARAM_PREFIX ## _space, \
		_T("space"), \
		TYPE_RADIOBTN_INDEX, \
		P_ANIMATABLE, \
		IDS_SPACE, \
		p_default, NormalMapTangentSpace::TANGENT, \
		p_range, NormalMapTangentSpace::TANGENT, NormalMapTangentSpace::WORLD, \
		p_ui, TYPE_RADIO, 3, IDC_RADIO_SPACE_TANGENT, IDC_RADIO_SPACE_OBJECT, IDC_RADIO_SPACE_WORLD, \
		p_end, \
		/* Strength */ \
		PARAM_PREFIX ## _strength, \
		_T("strength"), \
		TYPE_FLOAT, \
		P_ANIMATABLE, \
		IDS_STRENGTH, \
		p_default, 1.0f, \
		p_range, 0.0f, 100.0f, \
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_EDIT_STRENGTH, IDC_SPIN_STRENGTH, 0.1f, \
		p_end, \
		/* Strength texmap */ \
		PARAM_PREFIX ## _strength_map, \
		_T("strength_map"), \
		TYPE_TEXMAP, \
		0, \
		IDS_STRENGTH_TEXMAP, \
		p_subtexno, subtex_ ## PARAM_PREFIX ## _strength, \
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_STRENGTH, \
		p_end, \
		/* Strength texmap on */ \
		PARAM_PREFIX ## _strength_map_on, \
		_T("strength_map_enabled"), \
		TYPE_BOOL, \
		0, \
		IDS_STRENGTH_TEXMAP_ENABLED, \
		p_default, TRUE, \
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_STRENGTH, \
		p_end, \
		/* Color */ \
		PARAM_PREFIX ## _color, \
		_T("color"), \
		TYPE_RGBA, \
		P_ANIMATABLE, \
		IDS_COLOR, \
		p_default, Color(0.5f, 0.5f, 1.0f), \
		p_ui, TYPE_COLORSWATCH, IDC_SWATCH, \
		p_end, \
		/* Color texmap */ \
		PARAM_PREFIX ## _color_map, \
		_T("color_map"), \
		TYPE_TEXMAP, \
		0, \
		IDS_COLOR_TEXMAP, \
		p_subtexno, subtex_ ## PARAM_PREFIX ## _color, \
		p_ui, TYPE_TEXMAPBUTTON, IDC_BUTTON_MAP_COLOR, \
		p_end, \
		/* Color texmap on */ \
		PARAM_PREFIX ## _color_map_on, \
		_T("color_map_enabled"), \
		TYPE_BOOL, \
		0, \
		IDS_COLOR_TEXMAP_ENABLED, \
		p_default, TRUE, \
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_MAP_COLOR, \
		p_end, \
		/* Green channel inverted */ \
		PARAM_PREFIX ## _invert_green, \
		_T("invert_green"), \
		TYPE_BOOL, \
		0, \
		IDS_INVERT_GREEN, \
		p_default, TRUE, \
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_ENABLE_INVERT_GREEN, \
		p_end, \
	p_end \
	);

/**
 * @brief Macro to create a generic normal ParamBlockDesc2 that is usable by any material.
 */
#define MAKE_COMMON_NORMAL_PBLOCK(PBLOCK_INDEX,CLASSDESC) MAKE_NORMAL_PBLOCK(PBLOCK_INDEX,CLASSDESC,"normal_pblock",IDS_NORMAL_PARAMS,common_normal_pblock_desc,nparam)

/**
 * @brief Similar to MAKE_COMMON_NORMAL_PBLOCK, but used for a clearcoat normal ParamBlockDesc2.
 */
#define MAKE_CC_NORMAL_PBLOCK(PBLOCK_INDEX,CLASSDESC) MAKE_NORMAL_PBLOCK(PBLOCK_INDEX,CLASSDESC,"cc_normal_pblock",IDS_CC_NORMAL_PARAMS,cc_normal_pblock_desc,cc_nparam)

/**
 * @brief Macro to generate an enum for shader-level paramblock parameters.
 */
#define SHADER_PARAMS_ENUMS enum { param_shader_surface_use_mis, param_shader_vol_sampling, param_shader_vol_interp, param_shader_vol_homogeneous, param_shader_surface_displacement };

/**
 * @brief Macro to generate a ParamBlockDesc2 for shader-level parameters.
 */
#define SHADER_PARAMS_PBLOCK_DESC(CLASSDESC) \
static ParamBlockDesc2 mat_shader_pblock_params_desc( \
	pblock_shader_params, \
	_T("pblock_params"), \
	0, \
	CLASSDESC, \
	P_AUTO_CONSTRUCT + P_AUTO_UI, \
	pblock_shader_params, \
	IDD_PANEL_MAT_SHADER_PARAMS, \
	IDS_SHADER_PARAMS, \
	0, \
	0, \
	NULL, \
		/* Surface - MIS */ \
		param_shader_surface_use_mis, \
		_T("surface_mis"), \
		TYPE_BOOL, \
		0, \
		IDS_SURFACE_MIS, \
		p_default, TRUE, \
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_USE_MIS, \
		p_end, \
		/* Volume - Sampling*/ \
		param_shader_vol_sampling, \
		_T("volume_sampling"), \
		TYPE_RADIOBTN_INDEX, \
		0, \
		IDS_VOLUME_SAMPLING, \
		p_range, VolumeSamplingMethod::MULTIPLE_IMPORTANCE, VolumeSamplingMethod::DISTANCE, \
		p_default, VolumeSamplingMethod::MULTIPLE_IMPORTANCE, \
		p_ui, TYPE_RADIO, 3, IDC_RADIO_VOL_SAMPLE_MI, IDC_RADIO_VOL_SAMPLE_EQUIANGULAR, IDC_RADIO_VOL_SAMPLE_DISTANCE, \
		p_end, \
		/* Volume - Interp */ \
		param_shader_vol_interp, \
		_T("volume_interp"), \
		TYPE_RADIOBTN_INDEX, \
		0, \
		IDS_VOLUME_INTERP, \
		p_range, VolumeInterpolationMethod::LINEAR, VolumeInterpolationMethod::CUBIC, \
		p_default, VolumeInterpolationMethod::LINEAR, \
		p_ui, TYPE_RADIO, 2, IDC_RADIO_VOL_INTERP_LINEAR, IDC_RADIO_VOL_INTERP_CUBIC, \
		p_end, \
		/* Volume - Homogeneous */ \
		param_shader_vol_homogeneous, \
		_T("volume_homogeneous"), \
		TYPE_BOOL, \
		0, \
		IDS_VOLUME_HOMOGENEOUS, \
		p_default, FALSE, \
		p_ui, TYPE_SINGLECHEKBOX, IDC_BOOL_VOL_HOMOGENEOUS, \
		p_end, \
		/* Surface - Displacement Mehod */ \
		param_shader_surface_displacement, \
		_T("surface_displacement"), \
		TYPE_RADIOBTN_INDEX, \
		0, \
		IDS_DISPLACEMENT, \
		p_range, DisplacementMethod::BUMP_MAP, DisplacementMethod::BOTH, \
		p_default, DisplacementMethod::BUMP_MAP, \
		p_ui, TYPE_RADIO, 3, IDC_RADIO_DISPLACE_BUMP, IDC_RADIO_DISPLACE_DISPLACE, IDC_RADIO_DISPLACE_BOTH, \
		p_end, \
	p_end \
);

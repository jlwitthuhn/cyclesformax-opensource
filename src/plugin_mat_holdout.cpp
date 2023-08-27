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
 
#include "plugin_mat_holdout.h"

#include <iparamm2.h>

#include "max_classdesc_mtl.h"
#include "win_resource.h"

#define PBLOCK_REF 0

// Pblock enum
enum { holdout_pblock };

static ParamBlockDesc2 mat_holdout_pblock_desc(
	// Pblock data
	holdout_pblock,
	_T("pblock"),
	0,
	GetCyclesMaterialHoldoutClassDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// Interface stuff
	IDD_PANEL_MAT_HOLDOUT,
	IDS_PARAMS,
	0,
	0,
	NULL,
	p_end
	);

CyclesHoldoutMat::CyclesHoldoutMat() : CyclesPluginMatBase(false, false)
{
	pblock = nullptr;

	this->Reset();

	Initialize();

	node_type = ClosureType::SURFACE;
}

ClassDesc2* CyclesHoldoutMat::GetClassDesc() const
{
	return GetCyclesMaterialHoldoutClassDesc();
}

ParamBlockDesc2* CyclesHoldoutMat::GetParamBlockDesc()
{
	return &mat_holdout_pblock_desc;
}

void CyclesHoldoutMat::PopulateSubmatSlots()
{
	// No sub-materials, do nothing
}

void CyclesHoldoutMat::PopulateSubtexSlots()
{
	// No sub-textures, do nothing
}

void CyclesHoldoutMat::Update(const TimeValue /*t*/, Interval& /*valid*/)
{
	// Do nothing at all, ever
}
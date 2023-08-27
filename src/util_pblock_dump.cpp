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
 
#include "util_pblock_dump.h"

#include <fstream>

#include <iparamb2.h>

void dump_pblock_info(IParamBlock2* const pblock, const std::string id)
{
	std::wofstream out_stream;
	out_stream.open(std::string("D:\\Log\\pblock_dump_") + id + ".txt", std::wofstream::app);
	out_stream << L"-- New PBlock --" << std::endl;

	ParamBlockDesc2* desc = pblock->GetDesc();
	out_stream << "Param count: " << desc->Count() << std::endl;

	for (int i = 0; i < desc->Count(); i++) {
		const ParamDef* def = desc->GetParamDefByIndex(i);
		ParamType2 type = def->type;
		out_stream << L"--------" << std::endl;
		out_stream << L"name: " << def->int_name << std::endl;
		out_stream << L"id: " << def->ID << std::endl;
		switch (static_cast<int>(type)) {
			case TYPE_FLOAT:
				out_stream << L"TYPE_FLOAT" << std::endl;
				break;
			case TYPE_INT:
				out_stream << L"TYPE_INT" << std::endl;
				break;
			case TYPE_RGBA:
				out_stream << L"TYPE_RGBA" << std::endl;
				break;
			case TYPE_BOOL:
				out_stream << L"TYPE_BOOL" << std::endl;
				break;
			case TYPE_TEXMAP:
				out_stream << L"TYPE_TEXMAP" << std::endl;
				break;
			case TYPE_REFTARG:
				out_stream << L"TYPE_REFTARG" << std::endl;
				break;
			case TYPE_FRGBA:
				out_stream << L"TYPE_FRGBA" << std::endl;
				break;
			default:
				out_stream << type << L" (unknown type)" << std::endl;
				break;
		}
	}
}

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
 
#include "util_windows.h"

#include <UserEnv.h>
#include <Windows.h>

bool create_directory(const std::wstring dir)
{
	if (CreateDirectory(dir.c_str(), nullptr)) {
		return true;
	}
	else {
		return false;
	}
}

std::wstring get_user_dir()
{
	DWORD BUFFER_LENGTH = MAX_PATH;

	TCHAR path_buffer[MAX_PATH];
	HANDLE user_handle;

	OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &user_handle);
	GetUserProfileDirectory(user_handle, path_buffer, &BUFFER_LENGTH);
	CloseHandle(user_handle);

	return std::wstring(path_buffer);
}

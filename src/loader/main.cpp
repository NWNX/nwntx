/***************************************************************************
	NWNCX Loader - Preloads the main DLL and starts the nwmain executable
	Copyright (C) 2008 virusman (virusman@virusman.ru)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "detours.h"

int WINAPI WinMain(         
				   HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow
				   )
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	int size = strlen(lpCmdLine) + strlen("nwtoolset.exe") + 2;
	char *cmdline = new char[size];
	sprintf_s(cmdline, size, "%s %s", "nwtoolset.exe", lpCmdLine);

	//MessageBox(NULL, lpCmdLine, "Cmd Line", MB_OK);

	DetourCreateProcessWithDll(NULL, cmdline, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi, "NWNTX.dll", NULL);

	delete cmdline;
	return 0;
}
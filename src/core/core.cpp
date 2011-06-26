/***************************************************************************
	NWNTX Core - Loads the plugins
	Copyright (C) 2011 virusman (virusman@virusman.ru)

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

#include "core.h"
#include "commonheaders.h"

CHashTable Libraries;
int debuglevel = 0;
FILE *logFile;
char logFileName[18] = "logs/nwntx.txt";

// helper function prototype
static void Log(int priority, const char *pcMsg, ...);

static DWORD g_nwntxVersion = PLUGIN_MAKE_VERSION(0,1,0,0);
PLUGINLINK pluginCoreLink;
extern "C" int InitialiseModularEngine(void);
extern "C" void DestroyModularEngine(void);
//Hooks
HANDLE hPluginsLoadedEvent;
//Plugins
typedef int (* Plugin_Init) ( PLUGINLINK * );
typedef PLUGININFO * (* Plugin_Info) ( DWORD nwntxVersion );



/***************************************************************************
Fake export function for detours
***************************************************************************/

extern "C" __declspec(dllexport) 
void dummy() 
{	
	return ;
}

void LoadLibraries()
{
	WIN32_FIND_DATA findData;
	memset(&findData, 0, sizeof(findData));
	char logDir[MAX_PATH];
	BOOL proceed = TRUE;
	char moduleName[MAX_PATH];

	strcpy (logDir, "logs");

	// create hash table with default size
	if (!Libraries.Create())
	{
		fprintf (logFile, "* Critical error: creation of hash table failed!");
		return;
	}

	// search for all plugin DLL's
	HANDLE hSearch = FindFirstFile("nwntx_*.dll", &findData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		fprintf(logFile, "* Error: No plugins found\n");
		return;
	}

	do
	{
		memset(moduleName, 0, MAX_PATH);
		char* pos = strchr(findData.cFileName, '_') + 1;
		char* end = strchr(pos, '.');
		strncpy (moduleName, pos, end - pos);

		fprintf(logFile, "* Loading %s...\n", findData.cFileName);
		fflush(logFile);
		// try to load the library
		HINSTANCE hDLL = LoadLibrary(findData.cFileName);
		if (hDLL == NULL)
		{
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError(); 
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM| FORMAT_MESSAGE_MAX_WIDTH_MASK ,
				NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,	0, NULL);

			fprintf(logFile, "* An error occured while loading extension %s (%d: %s)\n", moduleName, dw, lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
		else
		{
			Plugin_Info getPluginInfo = (Plugin_Info) GetProcAddress(hDLL, "GetPluginInfo");
			Plugin_Init initPlugin = (Plugin_Init) GetProcAddress(hDLL, "InitPlugin");
			if(getPluginInfo && initPlugin)	{
				Log(1, "%s: Implements the  plugin interface\n" ,findData.cFileName);
				PLUGININFO * pi = 0;
				int initResult;
				pi = getPluginInfo(g_nwntxVersion);
				if(!pi) {
					Log(0, "%s: The plugin returned NULL on GetPluginInfo: skipping\n" ,findData.cFileName);
					continue;
				}
				if((initResult = initPlugin(&pluginCoreLink)) != 0) {
					Log(0, "%s: The plugin returned %d on InitPlugin: skipping\n" ,findData.cFileName, initResult);
					continue;
				}
			}
			else{
				Log(0, "%s: Doesn't implement the plugin interface\n" ,findData.cFileName);
			}
			// create an instance of module
			/*GetClassObject = (GETCLASSOBJECT)GetProcAddress(hDLL, "GetClassObject");
			if (GetClassObject != NULL)
			{
				CNWNXBase* pBase = GetClassObject();
				if (pBase)
				{
					if (!pBase->OnCreate(logDir))
						fprintf(logFile, "* An error occured during OnCreate of %s\n", moduleName);
					else
						fprintf(logFile, "* Plugin %s is loaded.\n", moduleName);
					Libraries.Insert(strupr(moduleName), pBase);
				}
				else
					fprintf(logFile, "* Could not create an instance of plugin %s", moduleName);
			}
			else
			{
				fprintf(logFile, "* Error: can not find creation function in %s\n", findData.cFileName);
			}*/
		}

		// find next module
		proceed = FindNextFile(hSearch, &findData);
	} while (proceed);

	FindClose(hSearch);
}

static void Log(int priority, const char *pcMsg, ...)
{
	va_list argList;
	char acBuffer[2048];

	if (logFile && priority<=debuglevel)
	{  
		// build up the string
		va_start(argList, pcMsg);
		vsnprintf(acBuffer, 2047, pcMsg, argList);
		acBuffer[2047]= 0;
		va_end(argList);

		// log string in file
		fprintf (logFile, acBuffer);
		fflush (logFile);
	}

	/*if(priority==0 && logFile != stdout)
	{
		printf(acBuffer);
	}*/
}

void LoadCoreModule()
{
	pluginCoreLink.CallService=CallService;
	pluginCoreLink.ServiceExists=ServiceExists;
	pluginCoreLink.CreateServiceFunction=CreateServiceFunction;
	pluginCoreLink.CreateTransientServiceFunction=CreateServiceFunction;
	pluginCoreLink.DestroyServiceFunction=DestroyServiceFunction;
	pluginCoreLink.CreateHookableEvent=CreateHookableEvent;
	pluginCoreLink.DestroyHookableEvent=DestroyHookableEvent;
	pluginCoreLink.HookEvent=HookEvent;
	pluginCoreLink.UnhookEvent=UnhookEvent;
	pluginCoreLink.NotifyEventHooks=NotifyEventHooks;
	pluginCoreLink.SetHookDefaultForHookableEvent=SetHookDefaultForHookableEvent;
	//pluginCoreLink.NotifyEventHooksDirect=CallHookSubscribers;

	hPluginsLoadedEvent = CreateHookableEvent(EV_CORE_PLUGINSLOADED);
}

int TestFunction(WPARAM wParam,LPARAM lParam)
{
	printf("TestFunction here: %d:%d\n", wParam, lParam);
	return 1;
}


DWORD WINAPI NWNTXInit(LPVOID lpParam) 
{
	CIniFile iniFile ("nwntx.ini");
	InitialiseModularEngine();
	LoadCoreModule();
	debuglevel = iniFile.ReadInteger("NWNTX", "debuglevel", 0);
	logFile = fopen(logFileName, "w");
	fprintf(logFile, "NWN Toolset Extender 1.0\n");
	fprintf(logFile, "(c) 2011 by virusman\n");
	fprintf(logFile, "* Loading plugins...\n");
	LoadLibraries();
	NotifyEventHooks(hPluginsLoadedEvent, 0, 0);
	fprintf(logFile, "* NWNTX activated.\n");
	fflush(logFile);
	return 0;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DWORD dwThreadId;
		HANDLE hThread; 
		hThread = CreateThread(NULL, 0, NWNTXInit, NULL, 0, &dwThreadId);                
		CloseHandle( hThread );
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//fprintf(logFile, "* NWNX2 shutting down...\n");
		Libraries.DeleteAll();
		DestroyModularEngine();
		////fprintf(logFile, "* NWNX2 shutdown successfull.\n");
		//fclose(logFile);
	}

	return TRUE;
}
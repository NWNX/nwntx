#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <pluginapi.h>
#include "detours.h"
#include "../../core/IniFile.h"
#include "Scene.h"
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

FILE *logFile;
char logFileName[] = "logs/nwntx_aabb.txt";
void InitPlugin();

//////////////////////////////////////////////////////////////////////////
PLUGINLINK *pluginLink = 0;
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"NWNTX AABB plugin",
	PLUGIN_MAKE_VERSION(1,0,0,0),
	"Render AABB nodes",
	"virusman",
	"virusman@virusman.ru",
	"© 2012 virusman",
	"http://www.virusman.ru/",
	0		//not transient
};

extern "C" __declspec(dllexport) PLUGININFO* GetPluginInfo(DWORD nwnxVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int InitPlugin(PLUGINLINK *link)
{
	pluginLink=link;
	InitPlugin();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
Scene *(__cdecl *NewCAurScene)(char *sName);

bool bRenderAABB = false;

Scene * __cdecl NewCAurScene_Hook(char *sName)
{
	Scene *pScene = NewCAurScene(sName);
	fprintf(logFile, "Scene %x", pScene);
	fflush(logFile);
	pScene->SetRenderAABB(bRenderAABB);
	fprintf(logFile, "SetAABB %x", pScene);
	fflush(logFile);
	return pScene;
}

void HookFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	*(DWORD*)&NewCAurScene = 0x0078C008;
	int success = DetourAttach(&(PVOID&)NewCAurScene, NewCAurScene_Hook)==0;

	fprintf(logFile, "Hooked: %d\n", success);
	DetourTransactionCommit();
}

void InitPlugin()
{
	logFile = fopen(logFileName, "w");
	fprintf(logFile, "NWNTX AABB plugin 1.0.0\n");
	fprintf(logFile, "(c) 2012 by virusman\n");
	fflush(logFile);

	CIniFile nwtoolsetIni ("nwtoolset.ini");
	bRenderAABB = nwtoolsetIni.ReadBool("Area", "RenderAABB", false);
	fprintf(logFile, "RenderAABB = %d\n", bRenderAABB);
	fflush(logFile);

	if(pluginLink){
	}
	HookFunctions();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		//delete plugin;
	}
	return TRUE;
}
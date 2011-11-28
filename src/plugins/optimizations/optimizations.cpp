#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <pluginapi.h>
#include "detours.h"
#include "../../core/IniFile.h"
#include "C2DA.h"
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

FILE *logFile;
char logFileName[] = "logs/nwntx_optimizations.txt";
typedef std::map<std::string, C2DA *> C2DAMap;
C2DAMap m2DAMap;
bool enable2DACache = false;
void InitPlugin();

//////////////////////////////////////////////////////////////////////////
PLUGINLINK *pluginLink = 0;
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"NWNTX Optimizations plugin",
	PLUGIN_MAKE_VERSION(1,0,1,0),
	"Toolset optimizations",
	"virusman",
	"virusman@virusman.ru",
	"© 2011 virusman",
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

template<class T>
struct map_data_compare : public std::binary_function<typename T::value_type, 
                                                      typename T::mapped_type, 
                                                      bool>
{
public:
    bool operator() (typename T::value_type &pair, 
                     typename T::mapped_type i) const
    {
        return pair.second == i;
    }
};

//////////////////////////////////////////////////////////////////////////

int Find2DA(C2DA *p2DA)
{
	C2DAMap::iterator it = std::find_if( m2DAMap.begin(), m2DAMap.end(), std::bind2nd(map_data_compare<C2DAMap>(), p2DA) );
    if ( it != m2DAMap.end() )
		return true;
	else
		return false;
}

void EnableWrite (unsigned long location)
{
	unsigned char *page;
	DWORD oldAlloc;
	page = (unsigned char *) location;
	VirtualProtect(page, 0xFF, PAGE_EXECUTE_READWRITE, &oldAlloc);
}

void PatchImage()
{
	//NOP the call to function that selects an item in the tree
	//when you click on an object
	char *pPatch = (char *) 0x00472CBF;
	EnableWrite((DWORD) pPatch);
	pPatch[0] = 0x90;
	pPatch[1] = 0x90;
	pPatch[2] = 0x90;
	pPatch[3] = 0x90;
	pPatch[4] = 0x90;

	//Patch ResMan to use 256MB of memory for resource cache
	pPatch = (char *) 0x007C7518;
	EnableWrite((DWORD) pPatch);
	pPatch[0] = 0x10;
	pPatch = (char *) 0x007C752D;
	EnableWrite((DWORD) pPatch);
	pPatch[0] = 0x10;
}


//////////////////////////////////////////////////////////////////////////
int (__cdecl *C2DA__Load2DArray)(C2DA *p2DA);
void (__cdecl *C2DA__Destructor)(C2DA *p2DA, char opt);
void (__cdecl *C2DA__Unload2DArray)(C2DA *p2DA);
void (*ReloadHaks)();

void *(*borland_new)(int size);
void (*borland_close)(void *ptr);


int __cdecl C2DA__Load2DArray_Hook(C2DA *p2DA)
{
	LARGE_INTEGER counterAtStart_;
	LARGE_INTEGER c;
	LARGE_INTEGER liFrequency;
	unsigned int tim;
	int ret;

	CResRef cResRef;
	strncpy(cResRef.m_resRef, p2DA->m_cResRef.m_resRef, 16);
	cResRef.m_resRef[15] = 0x0;
	
	fprintf(logFile, "2DA parse (%s)\n", cResRef.m_resRef);
	fflush(logFile);

	if(!enable2DACache)
	{
		fprintf(logFile, "Skipping\n");
		fflush(logFile);
		return C2DA__Load2DArray(p2DA);
	}

	if (m2DAMap.find(cResRef.m_resRef) != m2DAMap.end()) //object found
	{
		fprintf(logFile, "Cached 2DA: %s\n", cResRef.m_resRef);
		//ret = C2DA__Load2DArray(p2DA);
		C2DA *pCached2DA = m2DAMap.find(cResRef.m_resRef)->second;
		memcpy(p2DA, pCached2DA, sizeof(C2DA));
		ret = 1;
	}
	else
	{

	//QueryPerformanceFrequency(&liFrequency);
	//QueryPerformanceCounter(&counterAtStart_);
		fprintf(logFile, "Caching 2DA: %s\n", cResRef.m_resRef);
		ret = C2DA__Load2DArray(p2DA);

    //QueryPerformanceCounter(&c);
	//tim = (unsigned int) (c.QuadPart - counterAtStart_.QuadPart) * 1000000 / liFrequency.QuadPart ;
	//fprintf(logFile, " - took %d\n", tim);
	//fflush(logFile);
		m2DAMap[cResRef.m_resRef] = p2DA;
	}

	return ret;
}

void __cdecl C2DA__Unload2DArray_Hook(C2DA *p2DA)
{
	if(!enable2DACache)
	{
		C2DA__Unload2DArray(p2DA);
	}
	if(Find2DA(p2DA))
		return;
	/*else
		C2DA__Unload2DArray(p2DA);*/
}

void __cdecl C2DA__Destructor_Hook(C2DA *p2DA, char opt)
{
	if(!enable2DACache)
	{
		C2DA__Destructor(p2DA, opt);
	}
	if(Find2DA(p2DA))
	{
		fprintf(logFile, "Destructor called: %x\n", p2DA);
		fflush(logFile);
		return;
	}
	/*else
		C2DA__Destructor(p2DA, opt);*/
}

void Clear2DACache()
{
	/*fprintf(logFile, "Clearing 2DA cache\n");
	fflush(logFile);
	C2DAMap::iterator it;
	for(it = m2DAMap.begin(); it != m2DAMap.end(); ++it) {
		//C2DA__Unload2DArray(it->second);
		C2DA__Destructor(it->second, 3);
	}*/
	m2DAMap.clear();
}

int ReloadHaks_eax;
__declspec( naked ) void ReloadHaks_Hook()
{
	__asm{
		mov ReloadHaks_eax, eax
		pusha
	}
	Clear2DACache();
	enable2DACache = true;
	__asm{
		popa
		mov eax, ReloadHaks_eax
		jmp ReloadHaks
	}
}

void HookFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	*(DWORD*)&borland_new = 0x007F1A5C;
	*(DWORD*)&borland_close = 0x007F19A8;
	*(DWORD*)&C2DA__Load2DArray = 0x007C63F8;
	*(DWORD*)&C2DA__Unload2DArray = 0x007C6ACC;
	*(DWORD*)&C2DA__Destructor = 0x007C5DBC;
	*(DWORD*)&ReloadHaks = 0x0040B908;
	int success = DetourAttach(&(PVOID&)C2DA__Load2DArray, C2DA__Load2DArray_Hook)==0;
	success &= DetourAttach(&(PVOID&)C2DA__Unload2DArray, C2DA__Unload2DArray_Hook)==0;
	success &= DetourAttach(&(PVOID&)C2DA__Destructor, C2DA__Destructor_Hook)==0;
	success &= DetourAttach(&(PVOID&)ReloadHaks, ReloadHaks_Hook)==0;

	fprintf(logFile, "Hooked: %d\n", success);
	DetourTransactionCommit();
}

void InitPlugin()
{
	//DebugBreak();
	logFile = fopen(logFileName, "w");
	fprintf(logFile, "NWNTX Optimizations plugin 1.0.1\n");
	fprintf(logFile, "(c) 2011 by virusman\n");
	fflush(logFile);
	/*g_stDispatchTable.Size = sizeof(NSC_COMPILER_DISPATCH_TABLE_V2);
	g_stDispatchTable.Context = NULL;
	g_stDispatchTable.ResOpenFile = NULL;*/

	if(pluginLink){
	}
	HookFunctions();
	PatchImage();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		//MessageBox(NULL, GetCommandLine(), "NWNCX", MB_OK);
		//MessageBox(NULL, GetCurrentDirectory(), "NWNCX", MB_OK);
		//InitPlugin();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		//delete plugin;
	}
	return TRUE;
}
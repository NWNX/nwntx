#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <pluginapi.h>
#include "detours.h"
#include "../../core/IniFile.h"

namespace NWN { typedef int ResType; struct ResRef32 { char RefStr[32]; }; }
#define NSCEXT_NO_NWN2DATALIB_REFS
#include "NWNScriptCompilerDll.h"
using namespace NscExt;

#include "CScriptCompiler.h"
/*typedef struct  
{
	char		*Text;
	unsigned int Length;
} CExoString;*/

FILE *logFile;
char logFileName[] = "logs/nwntx_compiler.txt";
bool enableOptimizations = false;
bool enableExtensions = false;
bool generateDebugInfo = false;

void __stdcall NscCompilerDiagnosticOutput(const char * DiagnosticMessage, void * Context);
bool __stdcall ResLoadFile(const NWN::ResRef32 & ResRef, ResType Type, void **FileContents, size_t * FileSize, void *Context);
bool __stdcall ResUnloadFile(void * FileContents, void * Context);

NSC_COMPILER_HANDLE g_pCompiler;
NSC_COMPILER_DISPATCH_TABLE_V2 g_stDispatchTable={
	sizeof(NSC_COMPILER_DISPATCH_TABLE_V2),					// Size
	NULL,													// Context
	NULL,													// ResOpenFile
	NULL,													// ResOpenFileByIndex
	NULL,													// ResCloseFile
	NULL,													// ResReadEncapsulatedFile
	NULL,													// ResGetEncapsulatedFileSize
	NULL,													// ResGetEncapsulatedFileType
	NULL,													// ResGetEncapsulatedFileEntry
	NULL,													// ResGetEncapsulatedFileCount
	NscCompilerDiagnosticOutput,							// NscCompilerDiagnosticOutput
	ResLoadFile,											// ResLoadFile
	ResUnloadFile,											// ResUnloadFile
};

void InitPlugin();

//////////////////////////////////////////////////////////////////////////
PLUGINLINK *pluginLink = 0;
PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"NWNTX Compiler plugin",
	PLUGIN_MAKE_VERSION(1,0,1,0),
	"Custom compiler support for NWN Toolset",
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

//////////////////////////////////////////////////////////////////////////
int (__cdecl *CScriptCompiler__CompileFile)(void *pScriptCompiler, CExoString *sScriptName);
int (__cdecl *CScriptSourceFile__LoadScript)(CScriptSourceFile *pScript, CExoString const &,char **,unsigned long *);
void (__cdecl *CScriptSourceFile__UnloadScript)(CScriptSourceFile *pScript);
CExoString *(__cdecl *CExoAliasList__GetAliasPath)(void *pAliasList, CExoString const &sAlias, int flag);
void *(*borland_new)(int size);
void (*borland_close)(void *ptr);

void **g_pExoBase = NULL;

int CScriptCompiler__CompileFile_Hook(CScriptCompiler *pScriptCompiler, CExoString *sScriptName)
{
	/*int nRet = CScriptCompiler__CompileFile(pScriptCompiler, sScriptName);
	fprintf(logFile, "Compiling script: %s: %d\n", sScriptName->Text, nRet);
	fflush(logFile);
	return nRet;*/
	if(!g_pExoBase)
		g_pExoBase = *((void ***)0x00940B04);
	CExoString *sPath = CExoAliasList__GetAliasPath(*(g_pExoBase+3), pScriptCompiler->m_sOutputAlias, 0);


	g_stDispatchTable.Context = pScriptCompiler;
	fprintf(logFile, "Compiling script: %s\n", sScriptName->Text);
	fflush(logFile);
	int result = NscCompileScriptExternal(g_pCompiler, sScriptName->Text, sPath->Text, false, generateDebugInfo, enableOptimizations, true, 169, &g_stDispatchTable);
	fprintf(logFile, "Compiled script: %s: %d\n", sScriptName->Text, result);
	fflush(logFile);
	return !result;
	//pScriptCompiler->sErrorText = 
}

void __stdcall NscCompilerDiagnosticOutput(const char * DiagnosticMessage, void * Context)
{
	CScriptCompiler *pSC = (CScriptCompiler *)Context;
	pSC->sErrorText = DiagnosticMessage;
	fprintf(logFile, "%s\n", DiagnosticMessage);
	fflush(logFile);
}
bool __stdcall ResLoadFile(const NWN::ResRef32 & ResRef, ResType Type, void **FileContents, size_t * FileSize, void *Context)
{
	CExoString sScriptName = ResRef.RefStr;
	CScriptCompiler *pSC = (CScriptCompiler *)Context;
	if(CScriptSourceFile__LoadScript(&pSC->m_pcIncludeFileStack[0].m_cSourceScript, sScriptName, (char **) FileContents, (unsigned long *) FileSize) >= 0 )
		return true;
	else return false;
}
bool __stdcall ResUnloadFile(void * FileContents, void * Context)
{
	CScriptCompiler *pSC = (CScriptCompiler *)Context;
	CScriptSourceFile__UnloadScript(&pSC->m_pcIncludeFileStack[0].m_cSourceScript);
	return true;
}

void HookFunctions()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	*(DWORD*)&CScriptCompiler__CompileFile = 0x0074091C;
	*(DWORD*)&CScriptSourceFile__LoadScript = 0x0073E2AC;
	*(DWORD*)&CScriptSourceFile__UnloadScript = 0x0073E310;
	*(DWORD*)&CExoAliasList__GetAliasPath = 0x007C0C5C;
	*(DWORD*)&borland_new = 0x007F1A5C;
	*(DWORD*)&borland_close = 0x007F19A8;
	int success = DetourAttach(&(PVOID&)CScriptCompiler__CompileFile, CScriptCompiler__CompileFile_Hook)==0;

	fprintf(logFile, "Hooked: %d\n", success);
	DetourTransactionCommit();
}

void InitPlugin()
{
	CIniFile iniFile ("nwntx_compiler.ini");
	enableOptimizations = iniFile.ReadBool("Compiler", "EnableOptimizations", false);
	enableExtensions = iniFile.ReadBool("Compiler", "EnableExtensions", false);

	CIniFile nwtoolsetIni ("nwtoolset.ini");
	generateDebugInfo = iniFile.ReadBool("Script", "GenerateDebugInfo", false);

	//DebugBreak();
	logFile = fopen(logFileName, "w");
	fprintf(logFile, "NWN Toolset Extender 1.0.2 - Compiler plugin\n");
	fprintf(logFile, "(c) 2011 by virusman\n");
	fprintf(logFile, "Enable optimizations: %d\n", enableOptimizations);
	fprintf(logFile, "Enable extensions: %d\n", enableExtensions);
	fprintf(logFile, "Generate debug info: %d\n", generateDebugInfo);
	fflush(logFile);

	g_pCompiler = NscCreateCompiler(enableExtensions);
	/*g_stDispatchTable.Size = sizeof(NSC_COMPILER_DISPATCH_TABLE_V2);
	g_stDispatchTable.Context = NULL;
	g_stDispatchTable.ResOpenFile = NULL;*/

	if(pluginLink){
	}
	HookFunctions();

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
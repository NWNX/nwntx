/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_PLUGINAPI_H__
#define M_PLUGINAPI_H__

#define PLUGIN_MAKE_VERSION(a,b,c,d)   (((((DWORD)(a))&0xFF)<<24)|((((DWORD)(b))&0xFF)<<16)|((((DWORD)(c))&0xFF)<<8)|(((DWORD)(d))&0xFF))
#define MAXMODULELABELLENGTH 64

#if defined( _UNICODE )
	#define UNICODE_AWARE 1
#else
	#define UNICODE_AWARE 0
#endif

typedef struct {
	int cbSize;
	char *shortName;
	DWORD version;
	char *description; // [TRANSLATED-BY-CORE]
	char *author;
	char *authorEmail;
	char *copyright;
	char *homepage;
	BYTE flags;	   // right now the only flag, UNICODE_AWARE, is recognized here
} PLUGININFO;

#ifndef MODULES_H_
typedef int (*NWNTXHOOK)(WPARAM,LPARAM);
typedef int (*NWNTXSERVICE)(WPARAM,LPARAM);
typedef int (*NWNTXSERVICEPARAM)(WPARAM,LPARAM,LPARAM);
#define CALLSERVICE_NOTFOUND      ((int)0x80000000)
#endif

//see modules.h for what all this stuff is
typedef struct {
	HANDLE (*CreateHookableEvent)(const char *);
	int (*DestroyHookableEvent)(HANDLE);
	int (*NotifyEventHooks)(HANDLE,WPARAM,LPARAM);
	HANDLE (*HookEvent)(const char *,NWNTXHOOK);
	HANDLE (*HookEventMessage)(const char *,HWND,UINT);
	int (*UnhookEvent)(HANDLE);
	HANDLE (*CreateServiceFunction)(const char *,NWNTXSERVICE);
	HANDLE (*CreateTransientServiceFunction)(const char *,NWNTXSERVICE);
	int (*DestroyServiceFunction)(HANDLE);
	int (*CallService)(const char *,WPARAM,LPARAM);
	int (*ServiceExists)(const char *);		  //v0.1.0.1+
	int (*SetHookDefaultForHookableEvent) (HANDLE, NWNTXHOOK); // v0.3.4 (2004/09/15)
	int (*NotifyEventHooksDirect)(HANDLE,WPARAM,LPARAM); // v0.7+
} PLUGINLINK;

#ifndef MODULES_H_
#ifndef NODEFINEDLINKFUNCTIONS
//relies on a global variable 'pluginLink' in the plugins
extern PLUGINLINK *pluginLink;
#define CreateHookableEvent(a)               pluginLink->CreateHookableEvent(a)
#define DestroyHookableEvent(a)              pluginLink->DestroyHookableEvent(a)
#define NotifyEventHooks(a,b,c)              pluginLink->NotifyEventHooks(a,b,c)
#define HookEventMessage(a,b,c)              pluginLink->HookEventMessage(a,b,c)
#define HookEvent(a,b)                       pluginLink->HookEvent(a,b)
#define UnhookEvent(a)                       pluginLink->UnhookEvent(a)
#define CreateServiceFunction(a,b)           pluginLink->CreateServiceFunction(a,b)
#define CreateTransientServiceFunction(a,b)  pluginLink->CreateTransientServiceFunction(a,b)
#define DestroyServiceFunction(a)            pluginLink->DestroyServiceFunction(a)
#define CallService(a,b,c)                   pluginLink->CallService(a,b,c)
#define ServiceExists(a)                     pluginLink->ServiceExists(a)
#define SetHookDefaultForHookableEvent(a,b)  pluginLink->SetHookDefaultForHookableEvent(a,b)
#define NotifyEventHooksDirect(a,b,c)        pluginLink->NotifyEventHooksDirect(a,b,c)
#endif
#endif

#endif // M_PLUGINAPI_H__

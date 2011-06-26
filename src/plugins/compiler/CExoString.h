#ifndef _CExoString_h_
#define _CExoString_h_

#include <string.h>

extern void *(*borland_new)(int size);
extern void (*borland_close)(void *ptr);

class CExoString
{
	public:
		char *Text;
		unsigned int Length;

		CExoString();
		CExoString(const char *str);
		bool operator==(const char *str);
		CExoString *operator=(char const *str);
		~CExoString();
};

#endif


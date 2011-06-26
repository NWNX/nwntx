#ifndef CSCRIPTCOMPILER_H_
#define CSCRIPTCOMPILER_H_
#include "CExoString.h"

struct CResRef;
struct CScriptSourceFile;
struct CScriptCompilerIncludeFileStackEntry;
struct CScriptCompiler;
struct CResRef
{
  /* 0x0/0 */ char field_0[16];
};
struct CScriptSourceFile
{
  /* 0x0/0 */ unsigned long __VFN_table;
  /* 0x4/4 */ unsigned long m_bAutoRequest;
  /* 0x8/8 */ unsigned long m_pRes;
  /* 0xC/12 */ CResRef m_cResRef;
};
struct CScriptCompilerIncludeFileStackEntry
{
  /* 0x0/0 */ CExoString m_sCompiledScriptName;
  /* 0x8/8 */ CScriptSourceFile m_cSourceScript;
  /* 0x24/36 */ unsigned long m_nLine;
  /* 0x28/40 */ unsigned long m_nCharacterOnLine;
  /* 0x2C/44 */ unsigned long m_nTokenStatus;
  /* 0x30/48 */ unsigned long m_nTokenCharacters;
};
struct CScriptCompiler
{
  /* 0x0/0 */ unsigned long field_0;
  /* 0x4/4 */ unsigned long field_4;
  /* 0x8/8 */ unsigned long field_8[187];
  /* 0x2F4/756 */ unsigned long field_2F4;
  /* 0x2F8/760 */ char field_2F8;
  /* 0x30C/780 */ char rsvd1[19];
  /* 0x30C/780 */ CExoString m_sOutputAlias;
  /* 0x538/1336 */ char rsvd2[548];
  /* 0x538/1336 */ unsigned long field_538;
  /* 0x53C/1340 */ unsigned long field_53C[9];
  /* 0x560/1376 */ unsigned long m_nCompileFileLevel;
  /* 0x564/1380 */ CScriptCompilerIncludeFileStackEntry m_pcIncludeFileStack[32];
  /* 0x3174/12660 */ char rsvd3[9616];
  /* 0x3174/12660 */ unsigned long field_3174;
  /* 0x318C/12684 */ char rsvd4[20];
  /* 0x318C/12684 */ unsigned long field_318C;
  /* 0x3194/12692 */ char rsvd5[4];
  /* 0x3194/12692 */ unsigned long field_3194;
  /* 0x32D4/13012 */ char rsvd6[316];
  /* 0x32D4/13012 */ CExoString sErrorText;
  /* 0x32DC/13020 */ unsigned long field_32DC;
};

#endif
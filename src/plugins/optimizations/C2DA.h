#include "CExoString.h"

struct CResRef;
struct CRes;
struct CRes2DA;
struct CKeyTableEntry;
struct C2DA;
struct CResRef
{
  /* 0x0/0 */ char m_resRef[16];
};
struct CRes
{
  /* 0x0/0 */ unsigned long m_nDemands;
  /* 0x4/4 */ unsigned long m_nRequests;
  /* 0x8/8 */ unsigned long m_nID;
  /* 0xC/12 */ unsigned long field_C;
  /* 0x10/16 */ void *m_pResource;
  /* 0x14/20 */ CKeyTableEntry *m_pKeyEntry;
  /* 0x18/24 */ unsigned long m_nSize;
  /* 0x1C/28 */ unsigned long m_status;
  /* 0x20/32 */ CKeyTableEntry *m_pKeyEntry2;
  /* 0x24/36 */ unsigned long field_24;
  /* 0x28/40 */ unsigned long field_28;
};
struct CRes2DA
{
  /* 0x0/0 */ CRes Res;
  /* 0x2C/44 */ unsigned long m_bLoaded;
  /* 0x30/48 */ unsigned long m_n2DANormalSize;
  /* 0x34/52 */ unsigned long m_p2DAData;
  /* 0x38/56 */ unsigned long m_p2DAHeader;
};
struct CKeyTableEntry
{
  /* 0x0/0 */ CResRef m_cResRef;
  /* 0x10/16 */ unsigned long m_pRes;
  /* 0x14/20 */ unsigned long m_nID;
  /* 0x18/24 */ unsigned short m_nRefCount;
  /* 0x1A/26 */ unsigned short m_nType;
};
struct C2DA
{
  /* 0x0/0 */ unsigned long __VFN_table;
  /* 0x4/4 */ unsigned long m_bAutoRequest;
  /* 0x8/8 */ CRes2DA *m_pRes;
  /* 0xC/12 */ CResRef m_cResRef;
  /* 0x1C/28 */ unsigned long m_sDefaultValue;
  /* 0x20/32 */ CExoString *m_pRowLabel;
  /* 0x24/36 */ CExoString *m_pColumnLabel;
  /* 0x28/40 */ CExoString **m_pArrayData;
  /* 0x2C/44 */ unsigned long m_nNumRows;
  /* 0x30/48 */ unsigned long m_nNumColumns;
  /* 0x34/52 */ unsigned long m_bLoaded;
  /* 0x38/56 */ unsigned long field_38;
};


//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation, 1999 - 2001.  All rights reserved.
//

#ifndef __FILE_WMA_H__
#define __FILE_WMA_H__

#include <wmsdk.h>
#include "STEPlugin.h"

#define STREAM_NUM  0x0000

bool LoadFileWMA(FILE_INFO *pFileMP3);
bool WriteFileWMA(FILE_INFO *pFileMP3);

HRESULT LoadHeaderAttribute(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszName, FILE_INFO *pFileMP3, BOOL *pIsProtected);
bool WriteAttributeStr(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszAttrName, LPCTSTR sValue, BOOL bSeparate = FALSE);
bool WriteAttributeDWORD(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszAttrName, DWORD dwValue);
void DeleteAttribute(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszAttrName);
#ifndef UNICODE
HRESULT ConvertMBtoWC(LPCTSTR ptszInString, LPWSTR *ppwszOutString);
#endif
#endif  // __FILE_WMA_H__

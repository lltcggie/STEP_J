#pragma once

#ifndef _WIN64
//32bit版 STEP_ape.ste or 32bit版 runtak.exe で使用
void GetAudioFormatTak(const TCHAR *cszFileName, 
                       TCHAR *pszFormat, int nSize,
                       int *pTimeSec);
//32bit版 runtak.exe でのみ使用
void GetAudioFormatTakToFileMap(const TCHAR *cszFileName, 
                                const TCHAR *cszFileMapName);
#else
//64bit版 STEP_ape.ste でのみ使用
void GetAudioFormatTakFromFileMap(void *pFileMap, 
                                  TCHAR *pszFormat, int nSize,
                                  int *pTimeSec);
void* CreateTakFileMap(const TCHAR *cszFileMapName);
void  CloseTakFileMap(void *pFileMap);
#endif

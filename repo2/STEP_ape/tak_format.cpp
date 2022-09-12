#include "StdAfx.h"
#include "ofr_format.h"
#include "runtak/tak.h"

#ifndef _WIN64
////////////////////////////////////////////////////////
void SetAudioFormatTak(FILE_INFO *pFileMP3)
{
    TCHAR format[256];
    int nTime;
    GetAudioFormatTak(GetFullPath(pFileMP3), format, 256, &nTime);
    SetAudioFormat(pFileMP3, format);
    if(nTime != 0){
        SetPlayTime(pFileMP3, nTime);
    }
}
#else
extern TCHAR g_szPluginFolder[MAX_PATH];
void SetAudioFormatTak(FILE_INFO *pFileMP3)
{
    TCHAR szRunTak[MAX_PATH];  //runtak.exe のパス
    TCHAR szFileName[MAX_PATH];//tak ファイル名
    TCHAR szObjName[256];      //ファイルマップオブジェクト名
    TCHAR szCmdLine[1024];     //コマンドライン
    STARTUPINFO s = {0};
    PROCESS_INFORMATION pi = {0};
    LARGE_INTEGER li = {0};
    _tcsncpy_s(szRunTak, g_szPluginFolder, _TRUNCATE);
    _tcsncat_s(szRunTak, _T("runtak.exe"), _TRUNCATE);
    _tcsncpy_s(szFileName, GetFullPath(pFileMP3), _TRUNCATE);
    QueryPerformanceCounter(&li);
    _sntprintf_s(szObjName, _TRUNCATE,
                 _T("runtak_filemap_name_%llu"), li.QuadPart);
    _sntprintf_s(szCmdLine, _TRUNCATE,
                 _T("\"%s\" \"%s\" \"%s\""),
                 szRunTak, szFileName, szObjName);
    void *pFileMap = CreateTakFileMap(szObjName);
    if(!pFileMap){
        return;
    }
    if (!CreateProcess(szRunTak, szCmdLine, NULL, NULL, FALSE,
                        0, // dwCreationFlags
                        NULL, NULL, &s, &pi)) {
        CloseTakFileMap(pFileMap);
        return;
    }
    if(WaitForSingleObject(pi.hProcess, 10*1000) == WAIT_TIMEOUT){//10秒だけ待つ
        //何かエラーが起きてフリーズしている？
        TerminateProcess(pi.hProcess, 0);//強制終了
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        CloseTakFileMap(pFileMap);
        return;
    }
    TCHAR format[256];
    int nTime;
    GetAudioFormatTakFromFileMap(pFileMap, format, 256, &nTime);
    SetAudioFormat(pFileMP3, format);
    if(nTime != 0){
        SetPlayTime(pFileMP3, nTime);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseTakFileMap(pFileMap);
}
#endif

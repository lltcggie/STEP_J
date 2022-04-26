#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#ifndef _WIN64
#include "../tak/tak_deco_lib.h"
#endif

// {E5FB989D-B0A5-401D-BAD5-D9CA080C2B56}
static const GUID guid_TakFilemapObj =
{ 0xe5fb989d, 0xb0a5, 0x401d, { 0xba, 0xd5, 0xd9, 0xca, 0x8, 0xc, 0x2b, 0x56 } };
//↑TAK_FILEMAP_OBJ の仕様が変わったら変更すること

#pragma pack(push, 1)
struct TAK_FILEMAP_OBJ
{//32bit 版と 64bit 版の両方から参照されるので各変数のサイズに注意すること
    GUID  guid;  //=guid_TakFilemapObj
    DWORD cb;    //=sizeof(TAK_FILEMAP_OBJ)
    TCHAR szFormat[256];
    int   nTimeSec;
};
#pragma pack(pop)

#ifdef _WIN64
//runtak.exe を呼び出す側だけが使用
struct TAK_FORMAT
{
    HANDLE hFileMap;
    TAK_FILEMAP_OBJ *pObj;
};
#endif

#ifndef _WIN64
////////////////////////////////////////////////////////
static TtakBool tak_CanRead(void * AUser)
{
    return tak_True;
}
////////////////////////////////////////////////////////
static TtakBool tak_CanWrite(void * AUser)
{
    return tak_False;
}
////////////////////////////////////////////////////////
static TtakBool tak_CanSeek(void * AUser)
{
    return tak_True;
}
////////////////////////////////////////////////////////
static TtakBool tak_Read(void *      AUser,
                           void *      ABuf,
                           TtakInt32   ANum,
                           TtakInt32 * AReadNum )
{
    FILE *fp = (FILE*)AUser;
    *AReadNum = fread(ABuf, 1, ANum, fp);
    return tak_True;
}
////////////////////////////////////////////////////////
static TtakBool tak_Write(void *       AUser,
                          const void * ABuf,
                          TtakInt32    ANum)
{
    return tak_False;
}
////////////////////////////////////////////////////////
static TtakBool tak_Flush(void * AUser)
{
    return tak_False;
}
////////////////////////////////////////////////////////
static TtakBool tak_Truncate(void * AUser)
{
    return tak_False;
}
////////////////////////////////////////////////////////
static TtakBool tak_Seek(void *    AUser,
                         TtakInt64 APos)
{
    FILE *fp = (FILE*)AUser;
    return _fseeki64(fp, APos, SEEK_SET) == 0;
}
////////////////////////////////////////////////////////
static TtakBool tak_GetLength(void *      AUser,
                                TtakInt64 * ALength)
{
    FILE *fp = (FILE*)AUser;
    _int64 cur_pos = _fseeki64(fp, 0, SEEK_CUR);
    _fseeki64(fp, 0, SEEK_END);
    *ALength = _ftelli64(fp);
    _fseeki64(fp, cur_pos, SEEK_SET);
    return tak_True;
}
////////////////////////////////////////////////////////
//32bit版 STEP_ape.ste or 32bit版 runtak.exe で使用
////////////////////////////////////////////////////////
void GetAudioFormatTak(const TCHAR *cszFileName,
                       TCHAR *pszFormat, int nSize,
                       int *pTimeSec)
{
    FILE *fp = NULL;
    if(_tfopen_s(&fp, cszFileName, _T("rb")) != 0){
        return;
    }
    TtakStreamIoInterface si;
    si.CanRead = tak_CanRead;
    si.CanWrite = tak_CanWrite;
    si.CanSeek = tak_CanSeek;
    si.Read = tak_Read,
    si.Write = tak_Write;
    si.Flush = tak_Flush;
    si.Truncate = tak_Truncate;
    si.Seek = tak_Seek;
    si.GetLength = tak_GetLength;
    _int64 i64FileSize;
    tak_GetLength(fp, &i64FileSize);
    TtakSSDOptions opt = {tak_Cpu_Any, 0};
    TtakSeekableStreamDecoder Decoder = tak_SSD_Create_FromStream(&si,
                                                                  fp,
                                                                  &opt,
                                                                  NULL,
                                                                  NULL);
    if(!Decoder){
        fclose(fp);
        return;
    }
    TtakSSDResult result = {0};
    Ttak_str_StreamInfo stream_info = {0};

    tak_SSD_GetStateInfo(Decoder, &result);
    tak_SSD_GetStreamInfo(Decoder, &stream_info);
    double dSec = 0;
    int bps = 0;
    if(stream_info.Audio.SampleRate){
        dSec = (double)stream_info.Sizes.SampleNum / (double)stream_info.Audio.SampleRate;
        if(dSec > 0){
            bps = i64FileSize / dSec * 8;
        }
    }
    CHAR codec[256];
    TCHAR format[256];
    tak_GetCodecName(stream_info.Encoder.Codec, codec, sizeof(codec));
    if(bps){
        _sntprintf_s(format, _TRUNCATE,
#ifdef _UNICODE
                   _T("%S, %dkbps, %dHz, %dbit, %dch"),
#else
                   _T("%s, %dkbps, %dHz, %dbit, %dch"),
#endif
            codec, bps/1000, stream_info.Audio.SampleRate, stream_info.Audio.SampleBits,  stream_info.Audio.ChannelNum);
    }
    else{
        _sntprintf_s(format, _TRUNCATE,
#ifdef _UNICODE
                   _T("%S, %dHz, %dbit, %dch"),
#else
                   _T("%s, %dHz, %dbit, %dch"),
#endif
                   codec, stream_info.Audio.SampleRate, stream_info.Audio.SampleBits,  stream_info.Audio.ChannelNum);

    }
    _tcsncpy_s(pszFormat, nSize, format, _TRUNCATE);
    *pTimeSec = (int)dSec;
    tak_SSD_Destroy(Decoder);
    fclose(fp);
}
////////////////////////////////////////////////////////
//32bit版 runtak.exe でのみ使用
////////////////////////////////////////////////////////
void GetAudioFormatTakToFileMap(const TCHAR *cszFileName,
                                const TCHAR *cszFileMapName)
{
    HANDLE hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, cszFileMapName);
    if(!hFileMap){
        return;
    }
    TAK_FILEMAP_OBJ *pObj = (TAK_FILEMAP_OBJ*)MapViewOfFile(hFileMap,
                                                            FILE_MAP_ALL_ACCESS,
                                                            0, 0, sizeof(TAK_FILEMAP_OBJ));
    if(!pObj){
        CloseHandle(hFileMap);
        return;
    }
    if(pObj->guid != guid_TakFilemapObj ||  //互換性のないバージョンから呼ばれた
       pObj->cb != sizeof(TAK_FILEMAP_OBJ)){//Ansi版でビルドしたものをUnicode版で使った場合とか
        UnmapViewOfFile(pObj);
        CloseHandle(hFileMap);
        return;
    }
    GetAudioFormatTak(cszFileName, pObj->szFormat, _countof(pObj->szFormat), &pObj->nTimeSec);
    UnmapViewOfFile(pObj);
    CloseHandle(hFileMap);
}
#else
////////////////////////////////////////////////////////
//64bit版 STEP_ape.ste でのみ使用
////////////////////////////////////////////////////////
void GetAudioFormatTakFromFileMap(void *pFileMap,
                                  TCHAR *pszFormat, int nSize,
                                  int *pTimeSec)
{
    TAK_FORMAT *pFormat = (TAK_FORMAT*)pFileMap;
    _tcsncpy_s(pszFormat, nSize, pFormat->pObj->szFormat, _TRUNCATE);
    *pTimeSec = pFormat->pObj->nTimeSec;
}
////////////////////////////////////////////////////////
void* CreateTakFileMap(const TCHAR *cszFileMapName)
{
    HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
                                        PAGE_READWRITE, 0, sizeof(TAK_FILEMAP_OBJ), cszFileMapName);
    if(!hFileMap){
        return NULL;
    }
    TAK_FILEMAP_OBJ *pObj = (TAK_FILEMAP_OBJ*)MapViewOfFile(hFileMap,
                                                     FILE_MAP_ALL_ACCESS,
                                                     0, 0, sizeof(TAK_FILEMAP_OBJ));
    if(!pObj){
        CloseHandle(hFileMap);
        return NULL;
    }
    ZeroMemory(pObj, sizeof(TAK_FILEMAP_OBJ));
    pObj->guid = guid_TakFilemapObj;
    pObj->cb = sizeof(TAK_FILEMAP_OBJ);
    TAK_FORMAT *pFormat = (TAK_FORMAT*)malloc(sizeof(TAK_FORMAT));
    pFormat->pObj = pObj;
    pFormat->hFileMap = hFileMap;
    return pFormat;
}
////////////////////////////////////////////////////////
void CloseTakFileMap(void *pFileMap)
{
    TAK_FORMAT *pFormat = (TAK_FORMAT*)pFileMap;
    UnmapViewOfFile(pFormat->pObj);
    CloseHandle(pFormat->hFileMap);
    free(pFormat);
}
////////////////////////////////////////////////////////
#endif

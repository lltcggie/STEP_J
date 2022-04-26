#include "StdAfx.h"
#include "ofr_format.h"
#include "OptimFROG\OptimFROG.h"
#ifdef _WIN64
#pragma comment(lib, "OptimFROG\\Library64\\OptimFROG.lib")
#else
#pragma comment(lib, "OptimFROG\\Library32\\OptimFROG.lib")
#endif

///////////////////////////////////////////////////////////////////////////
static condition_t ofr_close(void* instance)
{
    FILE *fp = (FILE*)instance;
    fclose(fp);
    return C_TRUE;
}
///////////////////////////////////////////////////////////////////////////
static sInt32_t ofr_read(void* instance,void* destBuffer, uInt32_t count)
{
    FILE *fp = (FILE*)instance;
    return fread(destBuffer, 1, count, fp);
}
///////////////////////////////////////////////////////////////////////////
static condition_t ofr_eof(void* instance)
{
    FILE *fp = (FILE*)instance;
    return feof(fp);
}
///////////////////////////////////////////////////////////////////////////
static condition_t ofr_seekable(void* instance)
{
    return C_TRUE;
}
///////////////////////////////////////////////////////////////////////////
static sInt64_t ofr_length(void* instance)
{
    FILE *fp = (FILE*)instance;
    __int64 cur_pos = _ftelli64(fp);
    fseek(fp, 0, SEEK_END);
    __int64 size = _ftelli64(fp);
    _fseeki64(fp, cur_pos, SEEK_SET);
    return size;
}
///////////////////////////////////////////////////////////////////////////
static sInt64_t ofr_getPos(void* instance)
{
    FILE *fp = (FILE*)instance;
    return _ftelli64(fp);
}
///////////////////////////////////////////////////////////////////////////
static condition_t ofr_seek(void* instance, sInt64_t pos)
{
    FILE *fp = (FILE*)instance;
    return _fseeki64(fp, pos, SEEK_SET) == 0;
}
///////////////////////////////////////////////////////////////////////////
void SetAudioFormatOfr(FILE_INFO *pFileMP3)
{
    FILE *fp;
    if(_tfopen_s(&fp, GetFullPath(pFileMP3), _T("rb")) != 0){
        return;
    }
    void *pInstance = OptimFROG_createInstance();
    if(!pInstance){
        fclose(fp);
        return;
    }
    ReadInterface ri;
    ri.close = ofr_close;
    ri.read = ofr_read;
    ri.eof = ofr_eof;
    ri.seekable = ofr_seekable;
    ri.length = ofr_length;
    ri.getPos= ofr_getPos;
    ri.seek= ofr_seek;
    if(!OptimFROG_openExt(pInstance, &ri, fp, FALSE)){
        OptimFROG_destroyInstance(pInstance);
        fclose(fp);
        return;
    }
    OptimFROG_Info Info = {0};
    if(OptimFROG_getInfo(pInstance, &Info)){
        TCHAR format[256];
        _sntprintf_s(format, _TRUNCATE, 
                     _T("OptimFROG %dkbps, %dHz, %dbit, %dch"), 
                     Info.bitrate, Info.samplerate, Info.bitspersample, Info.channels);
        SetAudioFormat(pFileMP3, format);
        SetPlayTime(pFileMP3, Info.length_ms/1000);
    }
    OptimFROG_destroyInstance(pInstance);
    fclose(fp);
}

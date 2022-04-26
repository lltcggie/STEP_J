#include "StdAfx.h"
#include "wv_format.h"
#include "wavpack.h"

static int32_t wv_read_bytes(void *id, void *data, int32_t bcount)
{
    FILE *fp = (FILE*)id;
    return fread(data, 1, bcount, fp);
}
static int32_t wv_write_bytes(void *id, void *data, int32_t bcount)
{
    return 0;
}
static int64_t wv_get_pos (void *id)
{
    FILE *fp = (FILE*)id;
    return _ftelli64(fp);
}
static int wv_set_pos_abs (void *id, int64_t pos)
{
    FILE *fp = (FILE*)id;
    return _fseeki64(fp, 0, SEEK_SET) != 0;
}
static int wv_set_pos_rel (void *id, int64_t delta, int mode)
{
    FILE *fp = (FILE*)id;
    return _fseeki64(fp, delta, mode) != 0;
}
static int wv_push_back_byte (void *id, int c)
{
    FILE *fp = (FILE*)id;
    return fseek(fp, -1, SEEK_CUR) != 0;
}
static int64_t wv_get_length (void *id)
{
    FILE *fp = (FILE*)id;
    __int64 cur_pos = _ftelli64(fp);
    fseek(fp, 0, SEEK_END);
    __int64 size = _ftelli64(fp);
    _fseeki64(fp, cur_pos, SEEK_SET);
    return size;
}
static int wv_can_seek (void *id)
{
    return 1;
}
static int wv_truncate_here(void *id)
{
    return 0;
}
static int wv_close(void *id)
{
    return 0;
}

void SetAudioFormatWv(FILE_INFO *pFileMP3)
{
    FILE *fp;
    if(_tfopen_s(&fp, GetFullPath(pFileMP3), _T("rb")) != 0){
        return ;
    }
    WavpackStreamReader64 StreamReader = 
    {
        wv_read_bytes, 
        wv_write_bytes,
        wv_get_pos,
        wv_set_pos_abs, 
        wv_set_pos_rel, 
        wv_push_back_byte, 
        wv_get_length, 
        wv_can_seek,
        wv_truncate_here,
        wv_close
    };
    TCHAR drive[_MAX_DRIVE];
    TCHAR dir[_MAX_DIR];
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];
    TCHAR wvcpath[MAX_PATH];
    FILE *fpWvc = NULL;
    _tsplitpath_s(GetFullPath(pFileMP3), drive, dir, fname, ext);
    _tmakepath_s(wvcpath, drive, dir, fname, _T(".wvc"));
    _tfopen_s(&fpWvc, wvcpath, _T("rb"));
    WavpackContext *pContext = WavpackOpenFileInputEx64(&StreamReader, fp, fpWvc, NULL, OPEN_WVC|OPEN_2CH_MAX, 0);
    if(!pContext){
        if(fpWvc){
            fclose(fpWvc);
        }
        fclose(fp);
        return;
    }
    int sample_rate = WavpackGetSampleRate(pContext);
    int bps = WavpackGetBitsPerSample(pContext);
    int ch = WavpackGetNumChannels(pContext);
    TCHAR bitrate[128];
    double dBitrate = WavpackGetAverageBitrate(pContext, 0);
    double dBitrateWvc = WavpackGetAverageBitrate(pContext, 1);
    if(dBitrateWvc > dBitrate){
        dBitrateWvc /= 1000;
        dBitrateWvc += 0.5;
        dBitrate /= 1000;
        dBitrate += 0.5;
        _sntprintf_s(bitrate, _TRUNCATE, _T("%dkbps(%dkbps+%dkbps)"), 
                    (int)dBitrateWvc, (int)dBitrate, (int)dBitrateWvc-(int)dBitrate);
    }
    else if(dBitrate > 0){
        dBitrate /= 1000;
        dBitrate += 0.5;
        _sntprintf_s(bitrate, _TRUNCATE, _T("%dkbps"), (int)dBitrate);
    }
    else{
        bitrate[0] = 0;
    }

    TCHAR format[256];
    if(bitrate[0]){
        _sntprintf_s(format, _TRUNCATE,
                     _T("WavPack, %s, %dHz, %dbit, %dch"),
                     bitrate, sample_rate, bps, ch);
    }
    else{
        _sntprintf_s(format, _TRUNCATE,
                     _T("WavPack, %dHz, %dbit, %dch"),
                     sample_rate, bps, ch);
    }
    SetAudioFormat(pFileMP3, format);
    if(sample_rate){
        int nTime = WavpackGetNumSamples(pContext) / sample_rate;
        SetPlayTime(pFileMP3, nTime);
    }
    if(fpWvc){
        fclose(fpWvc);
    }
    fclose(fp);
    WavpackCloseFile(pContext);
}

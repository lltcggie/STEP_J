#include "StdAfx.h"
#include "FileAPE.h"

#include "Info_MonkeysAudio.h"

#include "ofr_format.h"
#include "tak_format.h"
#include "mpc_format.h"
#include "wv_format.h"

static void SetAudioFormatApe(FILE_INFO *pFileMP3);

///////////////////////////////////////////////////////////////////////////
void SetAudioFormatFromExt(FILE_INFO *pFileMP3, const TCHAR *cszExt)
{
    if(_tcsicmp(cszExt, _T("ape")) == 0){
        SetAudioFormatApe(pFileMP3);
    }
    else if(_tcsicmp(cszExt, _T("mpc")) == 0 ||
       _tcsicmp(cszExt, _T("mp+")) == 0){
        SetAudioFormatMpc(pFileMP3);
    }
    else if(_tcsicmp(cszExt, _T("ofr")) == 0 ||
            _tcsicmp(cszExt, _T("ofs")) == 0){
        SetAudioFormatOfr(pFileMP3);
    }
    else if(_tcsicmp(cszExt, _T("tak")) == 0){
        SetAudioFormatTak(pFileMP3);
    }
    else if(_tcsicmp(cszExt, _T("wv")) == 0){
        SetAudioFormatWv(pFileMP3);
    }
}
///////////////////////////////////////////////////////////////////////////
void SetAudioFormatApe(FILE_INFO *pFileMP3)
{
    CInfo_MonkeysAudio ma;
    if(ma.Load(GetFullPath(pFileMP3)) != 0){
        return;
    }

    CString strAudioFormat = ma.GetAudioFormatString();
    CString strTimeString = ma.GetTimeString();

    SetAudioFormat(pFileMP3, strAudioFormat);
    TCHAR *time = strTimeString.GetBuffer();
    //szTime = "xx:xx (xxxsec) ‚Ì‚æ‚¤‚É‚È‚Á‚Ä‚¢‚é
    TCHAR *pszSec = _tcschr(time, _T('('));
    if(pszSec){
        pszSec++;
        TCHAR *end;
        int sec = _tcstol(pszSec, &end, 10);
        SetPlayTime(pFileMP3, sec);
    }
    strTimeString.ReleaseBuffer();
}

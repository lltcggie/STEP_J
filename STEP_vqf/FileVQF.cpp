#include "stdafx.h"
#include "STEPlugin.h"
#include "Vqf.h"

bool LoadAttributeFileVQF(FILE_INFO *pFileMP3);
bool WriteAttributeFileVQF(FILE_INFO *pFileMP3);

bool LoadAttributeFileVQF(FILE_INFO *pFileMP3)
{
    CVqf vqf;
    if(vqf.Load(GetFullPath(pFileMP3)) != ERROR_SUCCESS){
        return false;
    }
	//タイトル
	CString data = vqf.GetField('N','A','M','E');
	if(data){
        SetTrackNameSI(pFileMP3, data);
    }
	//アーティスト
	data = vqf.GetField('A','U','T','H');
	if(data){
		SetArtistNameSI(pFileMP3, data);
	}
	//保存名
	//data = vqf.GetField('F','I','L','E');
	//if(data){
	//}
	//著作権
	data = vqf.GetField('(','c',')',' ');
	if(data){
		SetCopyrightSI(pFileMP3, data);
	}
    //コメント
	data = vqf.GetField('C','O','M','T');
	if(data){
		SetCommentSI(pFileMP3, data);
	}
    SetAudioFormat(pFileMP3, vqf.GetFormatString());
    CString strTimeString = vqf.GetTimeString();
    TCHAR *time = strTimeString.GetBuffer();
    //szTime = "xx:xx (xxxsec) のようになっている
    TCHAR *pszSec = _tcschr(time, _T('('));
    if(pszSec){
        pszSec++;
        TCHAR *end;
        int sec = _tcstol(pszSec, &end, 10);
        SetPlayTime(pFileMP3, sec);
    }
    strTimeString.ReleaseBuffer();

    return true;
}

bool WriteAttributeFileVQF(FILE_INFO *pFileMP3)
{
    CVqf vqf;
    if(vqf.Load(GetFullPath(pFileMP3)) != ERROR_SUCCESS){
        return false;
    }
    CString strTmp;
    //タイトル
	vqf.SetField('N','A','M','E', GetTrackNameSI(pFileMP3));
    //アーティスト
	vqf.SetField('A','U','T','H', GetArtistNameSI(pFileMP3));
    //保存名
	//vqf.SetField('F','I','L','E',
    //              ???,
    //              ???);
    //著作権
	vqf.SetField('(','c',')',' ', GetCopyrightSI(pFileMP3));
    //コメント
	vqf.SetField('C','O','M','T', GetCommentSI(pFileMP3));

    return vqf.Save(GetFullPath(pFileMP3)) == ERROR_SUCCESS;
}

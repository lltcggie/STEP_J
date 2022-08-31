//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation, 1999 - 2001.  All rights reserved.
//

#include "stdafx.h"
#include <TCHAR.h>
#include <stdio.h>
#include <string.h>
#include "FileWMA.h"

#ifndef SAFE_RELEASE

#define SAFE_RELEASE( x )  \
    if( NULL != x )        \
    {                      \
        x->Release( );     \
        x = NULL;          \
    }

#endif // SAFE_RELEASE

//
// List of all the ID3v2 Tag's Attribute names
//

LPCWSTR ID3TagDefs[] =
{
	g_wszWMAlbumTitle,//L"WM/AlbumTitle", // This is same as the global variable g_wszWMAlbumTitle
	L"WM/GenreID",    // This is same as the global variable g_wszWMGenreID
	L"Author",        // This is same as the global variable g_wszWMAuthor
	L"WM/Track",      // This is same as the global variable g_wszWMTrack
	L"WM/TrackNumber",// WMP9
	L"WM/PartOfSet",
	L"Title",         // This is same as the global variable g_wszWMTitle
	L"WM/Year",       // This is same as the global variable g_wszWMYear
	L"Description",   // This is same as the global variable g_wszWMDescription
	L"WM/GenreID",
	L"WM/Genre",
	L"Copyright",
	L"Is_Protected",
	L"Duration",
	L"WM/PromotionURL",
	L"WM/AlbumCoverURL",
	L"WM/OriginalArtist",
	L"WM/Composer",		// WMP9
	L"WM/Publisher",	// WMP9
	L"WM/AlbumArtist",	// WMP9
	L"WM/Writer",
    L"WM/ToolName",     //ソフトウェア
	NULL,
} ;

bool LoadFileWMA(FILE_INFO *pFileMP3)
{
	HRESULT hr = S_OK;

	LPWSTR	pwszInFile = NULL;
#ifndef _UNICODE
	hr = ConvertMBtoWC(GetFullPath(pFileMP3), &pwszInFile);
	if (FAILED(hr)) return(false);
#else
    pwszInFile = _wcsdup(GetFullPath(pFileMP3));
#endif
	//CoInitialize(NULL);//STEP 本体が一度だけ呼ぶように修正

    IWMMetadataEditor* pEditor      = NULL;
    IWMHeaderInfo3*    pHeaderInfo  = NULL;
	BOOL	bIsProtected = FALSE;
    WCHAR wszCodecName[256];
    WCHAR wszCodecDesc[256];
    wszCodecName[0] = 0;
    wszCodecDesc[0] = 0;
    UINT64 qwDuration = 0;
	do {
		// Lets create a Metadata Editor.
		hr = WMCreateEditor(&pEditor);
		if (FAILED(hr)) {
			TRACE( _T( "Could not create Metadata Editor (hr=0x%08x).\n" ), hr );
			break ;
		}

		hr = pEditor->Open(pwszInFile);
		if (FAILED(hr)) {
			TRACE( _T( "Could not open outfile %ws (hr=0x%08x).\n" ), pwszInFile, hr );
			break ;
		}

		hr = pEditor->QueryInterface( IID_IWMHeaderInfo3, ( void ** ) &pHeaderInfo );
		if (FAILED(hr)) {
			TRACE( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
			break ;
		}

		int i; 
        for (i = 0; ID3TagDefs[i] != NULL; i++) {
			hr = LoadHeaderAttribute(pHeaderInfo, ID3TagDefs[i], pFileMP3, &bIsProtected);
			if (FAILED(hr)) break;
		}
        WORD wStreamNum = 0;
        WORD cbDuration = sizeof(qwDuration);
        WMT_ATTR_DATATYPE data_type;
        hr = pHeaderInfo->GetAttributeByName( &wStreamNum, L"Duration", 
                                &data_type, (BYTE*)&qwDuration, &cbDuration );
        if(FAILED(hr)){
            qwDuration = 0;
        }
        WORD cchName = 0;
        WORD cchDesc = 0;
        WORD cchCodec = 0;
        WMT_CODEC_INFO_TYPE info_type;        
        pHeaderInfo->GetCodecInfo(0, &cchName, NULL, &cchDesc, NULL, &info_type, &cchCodec, NULL);
        BYTE *pCodec = (BYTE*)malloc(cchCodec);
        pHeaderInfo->GetCodecInfo(0, &cchName, wszCodecName, &cchDesc, wszCodecDesc, &info_type, &cchCodec, pCodec);
        free(pCodec);
		hr = pEditor->Close();
		if (FAILED(hr)) {
			TRACE( _T( "Could not close the file %ws (hr=0x%08x).\n" ), pwszInFile, hr) ;
			break;
		}
	}
	while(FALSE);

	SAFE_RELEASE( pHeaderInfo ) ;
	SAFE_RELEASE( pEditor ) ;

	//CoUninitialize();//STEP 本体が一度だけ呼ぶように修正
    free(pwszInFile);
	pwszInFile = NULL;

    TCHAR szFormat[256];
    szFormat[0] = 0;
    if(wszCodecName[0]){
#ifdef _UNICODE
        _sntprintf_s(szFormat, _TRUNCATE, _T("%s, %s"), wszCodecName, wszCodecDesc);
#else
        _sntprintf_s(szFormat, _TRUNCATE, _T("%S %S"), wszCodecName, wszCodecDesc);
#endif
        SetAudioFormat(pFileMP3, szFormat);
    }
    if(qwDuration > 0){
        SetPlayTime(pFileMP3, qwDuration / 1000 / 10000);
    }
	if (bIsProtected) {
		CString fmt = GetAudioFormat(pFileMP3);
		fmt += _T(", DRM");
		SetAudioFormat(pFileMP3, fmt);
	}
//	return(bIsProtected ? false : true);
	if (FAILED(hr)) {
		return false;
	} else {
		return true;
	}
}

bool WriteFileWMA(FILE_INFO *pFileMP3)
{
	HRESULT hr = S_OK;

	LPWSTR	pwszInFile = NULL;
#ifndef _UNICODE
	hr = ConvertMBtoWC((LPCSTR)GetFullPath(pFileMP3), &pwszInFile);
	if (FAILED(hr)) return(false);
#else
    pwszInFile = _wcsdup(GetFullPath(pFileMP3));
#endif
	//CoInitialize(NULL);//STEP 本体が一度だけ呼ぶように修正

	IWMMetadataEditor* pEditor      = NULL;
#ifndef USE_WMSDK9
    IWMHeaderInfo*     pHeaderInfo  = NULL;
#else
    IWMHeaderInfo3*    pHeaderInfo  = NULL;
#endif

	do {
		//
		// Lets create a Metadata Editor.
		//
		hr = WMCreateEditor( &pEditor );
		if (FAILED(hr)) {
			TRACE( _T( "Could not create Metadata Editor (hr=0x%08x).\n" ), hr );
			break ;
		}

		hr = pEditor->Open( pwszInFile ) ;
		if (FAILED(hr)) {
			TRACE( _T( "Could not open the file %ws (hr=0x%08x).\n" ), pwszInFile, hr );
			break ;
		}
		hr = pEditor->QueryInterface( IID_IWMHeaderInfo3, ( void ** ) &pHeaderInfo );
		if (FAILED(hr)) {
			TRACE( _T( "Could not QI for IWMHeaderInfo (hr=0x%08x).\n" ), hr );
			break ;
		}
		// タグ情報の出力
		if (_tcslen(GetTrackNumberSI(pFileMP3)) > 0 && _ttoi(GetTrackNumberSI(pFileMP3)) > 0) {
			WriteAttributeDWORD(pHeaderInfo, L"WM/Track", _ttoi(GetTrackNumberSI(pFileMP3)) - 1);
			WriteAttributeDWORD(pHeaderInfo, L"WM/TrackNumber", _ttoi(GetTrackNumberSI(pFileMP3)));
		}
		WriteAttributeStr(pHeaderInfo, L"WM/PartOfSet" , GetDiscNumberSI(pFileMP3));
		WriteAttributeStr(pHeaderInfo, L"WM/AlbumTitle", GetAlbumNameSI(pFileMP3));
		WriteAttributeStr(pHeaderInfo, L"Author"       , GetArtistNameSI(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"Copyright"    , GetCopyrightSI(pFileMP3));
		WriteAttributeStr(pHeaderInfo, L"Title"        , GetTrackNameSI(pFileMP3));
		WriteAttributeStr(pHeaderInfo, L"Description"  , GetCommentSI(pFileMP3));
		WriteAttributeStr(pHeaderInfo, L"WM/Year"      , GetYearSI(pFileMP3));
		WriteAttributeStr(pHeaderInfo, L"WM/Genre"     , GetGenreSI(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"WM/GenreID"   , GetGenreSI(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"WM/PromotionURL", GetURLSI(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"WM/AlbumCoverURL", GetOther(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"WM/OriginalArtist", GetOrigArtistSI(pFileMP3));
		WriteAttributeStr(pHeaderInfo, L"WM/Composer"  , GetComposerSI(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"WM/AlbumArtist"  , GetAlbumArtistSI(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"WM/Writer"		, GetWriterSI(pFileMP3), TRUE);
		WriteAttributeStr(pHeaderInfo, L"WM/Publisher"  , GetEngineerSI(pFileMP3), TRUE); // WMP9
		WriteAttributeStr(pHeaderInfo, L"WM/ToolName"		, GetSoftwareSI(pFileMP3), TRUE);

		hr = pEditor->Flush();
		if (FAILED(hr)) {
			TRACE( _T( "Flush failed (hr=0x%08x).\n" ), hr) ;
			break;
		}

		hr = pEditor->Close();
		if (FAILED(hr)) {
			TRACE( _T( "Could not close the file %ws (hr=0x%08x).\n" ), pwszInFile, hr) ;
			break;
		}
	}
	while( FALSE ) ;

	SAFE_RELEASE( pHeaderInfo ) ;
	SAFE_RELEASE( pEditor ) ;

	//CoUninitialize();//STEP 本体が一度だけ呼ぶように修正
    free(pwszInFile);

	return(FAILED(hr) ? false : true);
}

//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation, 1999 - 2001.  All rights reserved.
//

#include "stdafx.h"
#include <TCHAR.h>
#include <stdio.h>
#include <string.h>
#include <wmsdk.h>
#include "FileWMA.h"

#ifndef SAFE_RELEASE

#define SAFE_RELEASE( x )  \
    if( NULL != x )        \
    {                      \
        x->Release( );     \
        x = NULL;          \
    }

#endif // SAFE_RELEASE

void SetAttribute(LPCWSTR pwszName, WMT_ATTR_DATATYPE type, BYTE* pValue, DWORD cbLength, FILE_INFO *pFileMP3, BOOL *pIsProtected, BOOL bAppend)
{
    CString strName;
    CString strData;
    strName = pwszName;
    switch(type) {
    case WMT_TYPE_DWORD:
        strData.Format(_T("%u"), *((DWORD *)pValue));
        //TRACE( _T("%ws:  %u\n" ), pwszName, *((DWORD *)pValue) );
        break;
    case WMT_TYPE_STRING:{
#ifndef _UNICODE
        /* RockDance 131 *///wsprintf(sBuffer, "%ls", (WCHAR *)pValue);
        int ansi_len = WideCharToMultiByte(CP_ACP,0,(LPWSTR )pValue,-1, NULL, 0,NULL,NULL);
        char *str_ansi = (char*)malloc(ansi_len+1);
        WideCharToMultiByte(CP_ACP,0,(LPWSTR )pValue,-1, str_ansi, ansi_len,NULL,NULL);
        str_ansi[ansi_len] = 0;
        strData = str_ansi;
        free(str_ansi);
#else
        strData = (WCHAR*)pValue;
#endif
        //TRACE( _T("%ws:   %ws\n" ), pwszName, (WCHAR *)pValue );
        break;
    }
    case WMT_TYPE_BINARY:
        //TRACE( _T("%ws:   Type = Binary of Length %u\n" ), pwszName, cbLength );
        break;
    case WMT_TYPE_BOOL:
        strData.Format(_T("%d"), *((BOOL *)pValue) ? 1 : 0);
        //TRACE( _T("%ws:   %s\n" ), pwszName, ( * ( ( BOOL * ) pValue) ? _T( "true" ) : _T( "false" ) ) );
        break;
    case WMT_TYPE_WORD:
        strData.Format(_T("%hu"), *((WORD *)pValue));
        //TRACE( _T("%ws:  %hu\n" ), pwszName, *((WORD *) pValue) );
        break;
    case WMT_TYPE_QWORD:
        strData.Format(_T("%I64u"), *((QWORD *)pValue));
        //TRACE( _T("%ws:  %I64u\n" ), pwszName, *((QWORD *) pValue) );
        break;
    case WMT_TYPE_GUID:
        strData.Format(_T("%I64x%I64x"), *((QWORD *)pValue), *((QWORD *)pValue+1));
        //TRACE( _T("%ws:  %I64x%I64x\n" ), pwszName, *((QWORD *) pValue), *((QWORD *) pValue + 1) );
        break;
    default:
        //TRACE( _T("%ws:   Type = %d, Length %u\n" ), pwszName, type, cbLength );
        break;
    }
    if (strData.IsEmpty() == FALSE) {
        CString strPrev;
        if (false) { /* Conspiracy 198 */
//        if (strcmp(strName, "Duration") == 0) {
//            // 演奏時間
//            pFileMP3->nPlayTime = (int)((*((QWORD *)pValue) / (QWORD)10000000));
//        } else if (strcmp(strName, "Bitrate") == 0) {
//            // ビットレート
//            pFileMP3->nBitRate = atoi(strData) / 1000;
        } else if (_tcsicmp(strName, _T("WM/Track")) == 0) {
            // トラック番号
            //SetBTrackNumber(pFileMP3, (BYTE)atoi(strData) + 1);
            CString strTrack;
            strTrack.Format(_T("%d"), _ttoi(strData) + 1);
            SetTrackNumberSI(pFileMP3, strTrack);
        } else if (_tcsicmp(strName, _T("WM/TrackNumber")) == 0) {
            // トラック番号
            if (_ttoi(strData) == 0) {
                //SetBTrackNumber(pFileMP3, (BYTE)0xFF);
                SetTrackNumberSI(pFileMP3, _T(""));
            } else {
                //SetBTrackNumber(pFileMP3, (BYTE)atoi(strData));
                CString strTrack;
                strTrack.Format(_T("%d"), _ttoi(strData));
                SetTrackNumberSI(pFileMP3, strTrack);
            }
        } else if (_tcsicmp(strName, _T("WM/PartOfSet")) == 0) {
            // ディスク番号
            CString strDisc;
            strDisc.Format(_T("%d"), _ttoi(strData));
            SetDiscNumberSI(pFileMP3, strDisc);
        }  else if (_tcsicmp(strName, _T("WM/Year")) == 0) {
            // リリース
            if (bAppend) {
                strPrev = GetYearSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetYearSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/AlbumTitle")) == 0) {
            // アルバム
            if (bAppend) {
                strPrev = GetAlbumNameSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetAlbumNameSI(pFileMP3,  strData);
        } else if (_tcsicmp(strName, _T("Author")) == 0) {
            // アーティスト
            if (bAppend) {
                strPrev = GetArtistNameSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetArtistNameSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("Title")) == 0) {
            // タイトル
            if (bAppend) {
                strPrev = GetTrackNameSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetTrackNameSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("Description")) == 0) {
            // コメント
            if (bAppend) {
                strPrev = GetCommentSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetCommentSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/Genre")) == 0) {
            // ジャンル
            if (bAppend) {
                strPrev = GetGenreSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetGenreSI(pFileMP3, strData);
            //SetBGenre(pFileMP3, STEPGetGenreCode(strData));
        } else if (_tcsicmp(strName, _T("WM/GenreID")) == 0) {
            // ジャンル
            if (bAppend) {
                strPrev = GetGenreSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetGenreSI(pFileMP3, strData);
            //SetBGenre(pFileMP3, STEPGetGenreCode(strData));
        } else if (_tcsicmp(strName, _T("Copyright")) == 0) {
            if (bAppend) {
                strPrev = GetCopyrightSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetCopyrightSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/PromotionURL")) == 0) {
            if (bAppend) {
                strPrev = GetURLSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetURLSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/AlbumCoverURL")) == 0) {
            if (bAppend) {
                strPrev = GetOther(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetOther(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("Is_Protected")) == 0) {
            // DRM が有効なデータ
            *pIsProtected = (_ttoi(strData) == 1) ? TRUE : FALSE;
        } else if (_tcsicmp(strName, _T("WM/OriginalArtist")) == 0) {
            if (bAppend) {
                strPrev = GetOrigArtistSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetOrigArtistSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/Composer")) == 0) {
            if (bAppend) {
                strPrev = GetComposerSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetComposerSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/Publisher")) == 0) {
            if (bAppend) {
                strPrev = GetEngineerSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetEngineerSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/AlbumArtist")) == 0) {
            if (bAppend) {
                strPrev = GetAlbumArtistSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetAlbumArtistSI(pFileMP3, strData);
        } else if (_tcsicmp(strName, _T("WM/Writer")) == 0) {
            if (bAppend) {
                strPrev = GetWriterSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetWriterSI(pFileMP3, strData);
        } else if(_tcsicmp(strName, _T("WM/ToolName")) == 0){
            if (bAppend) {
                strPrev = GetSoftwareSI(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetSoftwareSI(pFileMP3, strData);
        } else if(_tcsicmp(strName, _T("WM/AlbumSortOrder")) == 0){
            if (bAppend) {
                strPrev = GetAlbumSort(pFileMP3);
                strData = strPrev + _T(";") + strData;
            }
            SetSoftwareSI(pFileMP3, strData);
        } else if(_tcsicmp(strName, _T("WM/Codec")) == 0){
            CString str = strData;
        }
    }
}

HRESULT LoadHeaderAttribute(IWMHeaderInfo *pHeaderInfo, LPCWSTR pwszName, FILE_INFO *pFileMP3, BOOL *pIsProtected)
{
    HRESULT hr         = S_OK ;

    WORD    nstreamNum = 0 ;
    WORD    cbLength   = 0 ;
    BYTE*   pValue     = NULL ;

    WMT_ATTR_DATATYPE type ;
    // Get the no of bytes to be allocated for pValue
    hr = pHeaderInfo->GetAttributeByName( &nstreamNum,
                                          pwszName,
                                          &type,
                                          NULL,
                                          &cbLength );

    if( FAILED( hr ) && hr != ASF_E_NOTFOUND ) {
        TRACE( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ), pwszName, hr) ;
        return hr ;
    }

    if( cbLength == 0 && hr == ASF_E_NOTFOUND ) {
        hr = S_OK ;
        return hr ;
    }

    if (cbLength == 0) return(S_OK);

    pValue = new BYTE[ cbLength ];

    if( NULL == pValue ) {
        hr = HRESULT_FROM_WIN32( GetLastError() ) ;

        TRACE( _T( "Internal Error (hr=0x%08x).\n" ), hr ) ;
        return hr ;
    }

    //
    // Now, Get the value
    //
    hr = pHeaderInfo->GetAttributeByName(&nstreamNum, pwszName, &type, pValue, &cbLength );
    if( FAILED( hr ) ) {
        delete[] pValue ;

        TRACE( _T( "GetAttributeByName failed for Attribute name %ws (hr=0x%08x).\n" ), pwszName, hr) ;
        return hr ;
    }

    SetAttribute(pwszName, type, pValue, cbLength, pFileMP3, pIsProtected, FALSE);

    delete[] pValue;
    pValue = NULL;

    return hr;
}

HRESULT LoadHeaderAttribute(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszName, FILE_INFO *pFileMP3, BOOL *pIsProtected)
{
#ifdef __IWMHeaderInfo3_FWD_DEFINED__
    HRESULT hr         = S_OK ;

    WORD    nstreamNum = 0 ;

    IWMHeaderInfo3*    pHeaderInfo3 = pHeaderInfo;
    WORD wCount = 0;
    WORD wLangIndex = 0;
    if (pHeaderInfo3->GetAttributeIndices(STREAM_NUM, pwszName, &wLangIndex, NULL, &wCount) != S_OK || wCount <= 0) {
        return hr;
    }
    WORD* wIndices = new WORD[wCount];
    if (pHeaderInfo3->GetAttributeIndices(STREAM_NUM, pwszName, &wLangIndex, wIndices, &wCount) == S_OK && wCount > 0) {
        WORD    Language = 0;
        BYTE   pbValue[1024+2];
        DWORD   cbValue  = 1024;
        WMT_ATTR_DATATYPE type;
        for (int i=0;i<wCount;i++) {
            Language = 0;
            cbValue = 1024;
            hr = pHeaderInfo3->GetAttributeByIndexEx(STREAM_NUM, wIndices[i], NULL, NULL, &type, &Language, pbValue, &cbValue);
            if (FAILED(hr)) {
                TRACE( _T( "GetAttributeByIndexEx failed for Attribute name %ws (hr=0x%08x).\n" ), pwszName, hr) ;
            } else {
                SetAttribute(pwszName, type, pbValue, cbValue, pFileMP3, pIsProtected, i == 0 ? FALSE : TRUE);
            }
        }
    }
    delete [] wIndices;

#else
    #error Undefined symbol <IWMHeaderInfo3>
#endif
    return hr;
}

void DeleteAttribute(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszAttrName)
{
    WORD wLangIndex = 0;
    WORD wCount = 0;
    if (pHeaderInfo->GetAttributeIndices(STREAM_NUM, pwszAttrName, &wLangIndex, NULL, &wCount) == S_OK && wCount > 0) {
        WORD* wIndices = new WORD[wCount];
        pHeaderInfo->GetAttributeIndices(STREAM_NUM, pwszAttrName, &wLangIndex, wIndices, &wCount);
        for (int i=0;i<wCount;i++) {
            HRESULT hr = pHeaderInfo->DeleteAttribute(STREAM_NUM, wIndices[i]);
            if (FAILED(hr)) {
                TRACE( _T( "DeleteAttribute failed for Attribute name %ws %02x (hr=0x%08x).\n" ), pwszAttrName, wIndices[i], hr) ;
            }
        }
        delete [] wIndices;
    }
}

bool WriteAttributeStr(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszAttrName, LPCTSTR sValue, BOOL bSeparate)
{
    //DeleteAttribute(pHeaderInfo, pwszAttrName);
    HRESULT hr = S_OK;
    WORD wLangIndex = 0;
    WORD wCount = 0;
    WORD* wIndices = NULL;
    if (pHeaderInfo->GetAttributeIndices(STREAM_NUM, pwszAttrName, &wLangIndex, NULL, &wCount) == S_OK && wCount > 0) {
        wIndices = new WORD[wCount];
        hr = pHeaderInfo->GetAttributeIndices(STREAM_NUM, pwszAttrName, &wLangIndex, wIndices, &wCount);
        if (FAILED(hr)) {
            TRACE( _T( "GetAttributeIndices failed for Attribute name %ws (hr=0x%08x).\n" ), pwszAttrName, hr) ;
            wCount = 0;
        }
    }

    CString strData = sValue;
    int nIndex = 0;
    while (strData.GetLength() > 0) {
        CString strToken;
        if (bSeparate) {
            strToken = strData.SpanExcluding(_T(";"));
        } else {
            strToken = strData;
        }
#ifndef _UNICODE
        LPWSTR    pwszValue = NULL;
        hr = ConvertMBtoWC(strToken, &pwszValue);
        if (FAILED(hr)) return(false);
#else
        LPWSTR pwszValue = strToken.GetBuffer(0);
#endif

        if (wCount > nIndex) {
            hr = pHeaderInfo->ModifyAttribute(STREAM_NUM, wIndices[nIndex], WMT_TYPE_STRING, 0,
                                            (LPBYTE)pwszValue, (wcslen(pwszValue)+1) * sizeof(WCHAR));
            TRACE( _T( "ModifyAttribute #%d for Attribute name %ws value %s #%02x (hr=0x%08x).\n" ), nIndex+1,
                                                                pwszAttrName, strToken, wIndices[nIndex], hr) ;
        } else {
            hr = pHeaderInfo->AddAttribute(STREAM_NUM, pwszAttrName, NULL, WMT_TYPE_STRING, 0,
                                            (LPBYTE)pwszValue, (wcslen(pwszValue)+1) * sizeof(WCHAR));
            TRACE( _T( "AddAttribute #%d for Attribute name %ws value %s (hr=0x%08x).\n" ), nIndex+1,
                                                                pwszAttrName, strToken, hr) ;
        }
#ifndef _UNICODE
        free(pwszValue);
#else
        strToken.ReleaseBuffer();
#endif
        pwszValue = NULL;

        if (FAILED(hr)) {
            TRACE( _T( "AddAttribute/ModifyAttribute failed for Attribute name %ws (hr=0x%08x).\n" ), pwszAttrName, hr) ;
            break;
        }
        if (strData == strToken) {
            strData.Empty();
        } else {
            strData = strData.Mid(strToken.GetLength() + 1);
        }
        nIndex++;
    }
    if (!FAILED(hr)) {
        int i;
        for (i=nIndex;i<wCount;i++) {
            HRESULT hr = pHeaderInfo->DeleteAttribute(STREAM_NUM, wIndices[i]);
            if (FAILED(hr)) {
                TRACE( _T( "DeleteAttribute failed for Attribute name %ws #%02x (hr=0x%08x).\n" ), pwszAttrName, wIndices[nIndex], hr) ;
            }
        }
    }
    delete [] wIndices;
    if (FAILED(hr)) {
        return(false);
    }
    return(true);
}

bool WriteAttributeDWORD(IWMHeaderInfo3 *pHeaderInfo, LPCWSTR pwszAttrName, DWORD dwValue)
{
    HRESULT hr = S_OK;
    hr = pHeaderInfo->SetAttribute(STREAM_NUM, pwszAttrName, WMT_TYPE_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
    if (FAILED(hr)) {
        TRACE( _T( "SetAttribute failed for Attribute name %ws (hr=0x%08x).\n" ), pwszAttrName, hr) ;
        return(false);
    }
    return(true);
}

#ifndef UNICODE

HRESULT ConvertMBtoWC( LPCTSTR ptszInString, LPWSTR *ppwszOutString )
{
    if( ptszInString == NULL || ppwszOutString == NULL )
    {
        return E_INVALIDARG ;
    }

    HRESULT hr = S_OK ;

    int nSizeCount = 0 ;
    do
    {
        //
        // Get the memory reqd for this string
        //
        nSizeCount = MultiByteToWideChar( CP_ACP, 0, ptszInString, -1, NULL, 0 ) ;
        if( 0 ==  nSizeCount )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() ) ;
            break ;
        }

        //*ppwszOutString = new WCHAR[nSizeCount] ;
        *ppwszOutString = (WCHAR*)malloc(nSizeCount*sizeof(WCHAR));
        if( NULL == *ppwszOutString )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() ) ;
            break ;
        }

        if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInString, -1, *ppwszOutString, nSizeCount ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() ) ;
            break ;
        }
    }
    while( FALSE ) ;

    if( FAILED( hr ) )
    {
        //delete[] (*ppwszOutString) ;
        if(*ppwszOutString){
            free(*ppwszOutString);
        }
        *ppwszOutString = NULL ;
        TRACE( _T( "Internal error (hr=0x%08x)\n" ), hr );
    }

    return hr;
}

#endif // UNICODE

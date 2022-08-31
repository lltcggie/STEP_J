#include "stdafx.h"
#include "STEP_mp3.h"
#include "Id3tagv1.h"
#include "Id3tagv2.h"
#include "RMP.h"
#include "STEPlugin.h"
#include "STEP_mp3_ConvFormat.h"

extern UINT nFileTypeMP3;
extern UINT nFileTypeMP3V1;
extern UINT nFileTypeMP3V11;
extern UINT nFileTypeID3V2;
extern UINT nFileTypeRMP;
extern bool bOptAutoConvID3v2;
extern bool bOptAutoConvRMP;
extern bool bOptID3v2ID3tagAutoWrite;
extern bool bOptRmpID3tagAutoWrite;
extern bool bOptChangeFileExt;
extern bool bOptID3v2ID3tagAutoDelete;

extern int      nId3v2EncodeNew;
extern int      nId3v2VersionNew;

extern bool WriteTagID3(FILE_INFO *pFileMP3);
extern bool WriteTagID3v2(FILE_INFO *pFileMP3);
extern bool WriteTagSIF(FILE_INFO *pFileMP3);
extern void ConvSIFieldToID3tag(FILE_INFO *pFileMP3);
extern bool ConvID3tagToSIField(FILE_INFO *pFileMP3);
extern bool ID3v1IsEmpty(FILE_INFO *pFileMP3);

bool ConvExt(FILE_INFO* pFileMP3)
{
    // 拡張子の変更
    UINT nFormat = GetFormat(pFileMP3);
    if (bOptChangeFileExt) {
        TCHAR   fname[_MAX_FNAME];
        _tsplitpath(GetFileName(pFileMP3), NULL, NULL, fname, NULL);
        CString strNewFileName = GetFileName(pFileMP3);

        if (nFormat == nFileTypeMP3 || nFormat == nFileTypeMP3V1 || nFormat == nFileTypeMP3V11 || nFormat == nFileTypeID3V2) {
            // MP3 形式に変換
            strNewFileName.Format(_T("%s%s"), fname, _T(".mp3"));
        } else if (nFormat == nFileTypeRMP) {
            // RIFF MP3 形式に変換
            strNewFileName.Format(_T("%s%s"), fname, _T(".rmp"));
        }
        if (STEPFileNameChange(pFileMP3, strNewFileName) == false) {
            return false;
        }
    }
    return true;
}

bool DeleteTagID3v1(const TCHAR *sFileName, HWND hWnd)
{
    // ID3v1 タグの取得
    CId3tagv1   id3v1/*(USE_SCMPX_GENRE_ANIMEJ)*/;
    if (id3v1.Load(sFileName) != ERROR_SUCCESS  // 読み込み失敗
        ||  id3v1.IsEnable() == FALSE) {        // ID3v1 ではない
        return false;
    }
    // ID3v1 タグの削除
    if (id3v1.DelTag(sFileName) != ERROR_SUCCESS) {
        return false;
    }
    return true;
}

bool DeleteTagID3v2(const TCHAR *sFileName, HWND hWnd)
{
    // ID3v2 タグの取得
    CId3tagv2   id3v2/*(USE_SCMPX_GENRE_ANIMEJ)*/;
    if (id3v2.Load(sFileName) != ERROR_SUCCESS  // 読み込み失敗
    ||  id3v2.IsEnable() == FALSE           // ID3v2 ではない
    ||  !TRUE/*id3v2.IsSafeVer()*/) {               // 未対応のバージョン
        return false;
    }
    // ID3v2 タグの削除
    if (id3v2.DelTag(sFileName) != ERROR_SUCCESS) {
        return false;
    }
    return true;
}

bool DeleteTagSIF(const TCHAR* sFileName, HWND hWnd) {
    CRMP rmp;
    if (rmp.Load(sFileName) != ERROR_SUCCESS) {
        return false;
    }
    if (rmp.DelTag(sFileName) != ERROR_SUCCESS) {
        return false;
    }
    return true;
}


bool MakeTagID3v2(const TCHAR *sFileName, HWND hWnd)
{
    CId3tagv2   id3v2/*(USE_SCMPX_GENRE_ANIMEJ)*/;
    switch (nId3v2VersionNew) {
    case 0: // v2.2
        id3v2.SetVer(0x0200);
        break;
    case 1: // v2.3
        id3v2.SetVer(0x0300);
        break;
    case 2: // v2.4
        id3v2.SetVer(0x0400);
        break;
    default:
        break;
    }
    switch (nId3v2EncodeNew) {
    case 0:
        id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_ISO_8859_1);
        break;
    case 1:
        id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_UTF_16);
        break;
    case 2:
        id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_UTF_8);
        break;
    //case 3:
    //  id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_UTF_16BE);
    //  break;
    default:
        break;
    }
    id3v2.SetUnSynchronization(false);
    if (id3v2.MakeTag(sFileName) != ERROR_SUCCESS) {
        // 変換失敗
        return(false);
    }
    return(true);
}

bool MakeTagSIF(const TCHAR* sFileName, HWND hWnd) {
    CRMP rmp;
    if (rmp.MakeTag(sFileName) != ERROR_SUCCESS) {
        return false;
    }
    return true;
}

bool ConvFileFormat(FILE_INFO* pFileMP3, UINT nType, HWND hWnd) {
    // 事前にタグを更新
    int nFormat = GetFormat(pFileMP3);
    if (nType == nFileTypeMP3 || nType == nFileTypeMP3V1 || nType == nFileTypeMP3V11) { // ID3v1 に変換
        if (nFormat == nFileTypeRMP) { // RIFF形式 => ID3v1
            if (DeleteTagSIF(GetFullPath(pFileMP3), hWnd) == false) {
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"),  GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return false;
            }
            if(ID3v1IsEmpty(pFileMP3)){//ID3v1 が空の場合は SIF からコピー
                ConvSIFieldToID3tag(pFileMP3);
            }
            WriteTagID3(pFileMP3);
            STEPInitDataSIF(pFileMP3);
        } 
        else if (nFormat == nFileTypeID3V2) { // ID3v2 => ID3v1
            if (DeleteTagID3v2(GetFullPath(pFileMP3), hWnd) == false) {
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"),  GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return false;
            }
            if(ID3v1IsEmpty(pFileMP3)){//ID3v1 が空の場合は SIF からコピー
                ConvSIFieldToID3tag(pFileMP3);
            }
            WriteTagID3(pFileMP3);
            STEPInitDataSIF(pFileMP3);
        } else {
            // 未対応の形式
            return true;
        }
    } else if (nType == nFileTypeID3V2) { // ID3v2 に変換
        if (nFormat == nFileTypeMP3 || nFormat == nFileTypeMP3V1 || nFormat == nFileTypeMP3V11) { // ID3v1 => ID3v2形式
            if (MakeTagID3v2(GetFullPath(pFileMP3), hWnd) == false) {
                STEPInitDataSIF(pFileMP3);              // SIF のクリア
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"), GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return false;
            }
            extern bool ConvID3tagToSIField(FILE_INFO *pFileMP3);
            ConvID3tagToSIField(pFileMP3);  // ID3v1 から SIF にコピー
            WriteTagID3v2(pFileMP3);
            if(bOptID3v2ID3tagAutoDelete){//ID3v1 を削除
                DeleteTagID3v1(GetFullPath(pFileMP3), hWnd);
                STEPInitDataID3(pFileMP3);
            }
            else if(ID3v1IsEmpty(pFileMP3)){
                DeleteTagID3v1(GetFullPath(pFileMP3), hWnd);
            }
        } else if (nFormat == nFileTypeRMP) { // RIFF形式 => ID3v2形式
            // RIFF MP3 => (標準MP3形式) => ID3v2 変換
            if (DeleteTagSIF(GetFullPath(pFileMP3), hWnd) == false) {
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"),  GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return false;
            }
            SetFormat(pFileMP3, nFileTypeMP3);
            if (MakeTagID3v2(GetFullPath(pFileMP3), hWnd) == false) {
                STEPInitDataSIF(pFileMP3);              // SIF のクリア
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"), GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return false;
            }
            SetFormat(pFileMP3, nFileTypeID3V2);
            WriteTagID3v2(pFileMP3);
            if(bOptID3v2ID3tagAutoDelete){//ID3v1 を削除
                DeleteTagID3v1(GetFullPath(pFileMP3), hWnd);
                STEPInitDataID3(pFileMP3);
            }
            else if(ID3v1IsEmpty(pFileMP3)){
                DeleteTagID3v1(GetFullPath(pFileMP3), hWnd);
            }
        } else {
            // 未対応の形式
            return true;
        }
    } else if (nType == nFileTypeRMP) { // RIFF形式に変換
        if (nFormat == nFileTypeMP3 || nFormat == nFileTypeMP3V1 || nFormat == nFileTypeMP3V11) { // ID3v1 => RIFF形式
            if (MakeTagSIF(GetFullPath(pFileMP3), hWnd) == FALSE) {
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"), GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return false;
            }
            ConvID3tagToSIField(pFileMP3);  // ID3v1 から SIF にコピー
            WriteTagSIF(pFileMP3);
        } else if (nFormat == nFileTypeID3V2) { // ID3v2形式 => RIFF形式
            // ID3v2 => (標準MP3形式) => RIFF MP3 変換
            if (DeleteTagID3v2(GetFullPath(pFileMP3), hWnd) == false) {
                // 変換失敗
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"), GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return(false);
            }
            if (MakeTagSIF(GetFullPath(pFileMP3), hWnd) == FALSE) {
                MessageBox(hWnd, _T("ファイル形式の変換に失敗しました"), GetFullPath(pFileMP3), MB_ICONSTOP|MB_OK|MB_TOPMOST);
                return false;
            }
            WriteTagSIF(pFileMP3);
        } else {
            // 未対応の形式
            return true;
        }
    } else {
        // 未対応の形式
        return false;
    }
    SetModifyFlag(pFileMP3, false);
    extern bool LoadFileMP3(FILE_INFO *pFileMP3);
    if (LoadFileMP3(pFileMP3) == false) {
        return false;
    }
    // 拡張子の変更
    return ConvExt(pFileMP3);
}

/**----- コールバック -----**/
bool WINAPI ConvFileFormatID3v1(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd) {
    if (nProcFlag == 3) {
        return false;
    }
    int nFormat = GetFormat(pFileInfo);
    if (!TYPE_IS_SUPPORT(nFormat)) { /* 2005.09.21 */
        return true;
    }
    if (ConvFileFormat(pFileInfo, nFileTypeMP3, hWnd) == false) {
        return false;
    }
    return true;
}

bool WINAPI ConvFileFormatRMP(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd) {
    if (nProcFlag == 3) {
        return false;
    }
    int nFormat = GetFormat(pFileInfo);
    if (!TYPE_IS_SUPPORT(nFormat)) { /* 2005.09.21 */
        return true;
    }
    if (ConvFileFormat(pFileInfo, nFileTypeRMP, hWnd) == false) {
        return false;
    }
    return true;
}
/*
bool WINAPI ConvFileFormatID3v2(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd) {
    if (nProcFlag == 3) {
        return false;
    }
    if (ConvFileFormat(pFileInfo, nFileTypeID3V2, hWnd) == false) {
        return false;
    }
    return true;
}
*/
bool WINAPI DeleteId3tagv1(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd) {
    if (nProcFlag == 3) {
        return false;
    }
    int nFormat = GetFormat(pFileInfo);
    if (!TYPE_IS_SUPPORT(nFormat)) { /* 2005.09.21 */
        return true;
    }
    // 事前にタグを更新
    if (isModify(pFileInfo)) {
        if (STEPWriteTag(pFileInfo) == false) {
            return false;
        }
    }
    /*bool result =*/ DeleteTagID3v1(GetFullPath(pFileInfo), hWnd);
    STEPInitDataID3(pFileInfo);
    STEPInitDataSIF(pFileInfo);
    extern bool LoadFileMP3(FILE_INFO *pFileMP3);
    LoadFileMP3(pFileInfo);
    //return result;//false を返すとそれ以降の選択ファイルを処理しなくなる
    return true;
}

bool WINAPI DeleteId3tagv2(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd) {
    if (nProcFlag == 3) {
        return false;
    }
    int nFormat = GetFormat(pFileInfo);
    if (!TYPE_IS_SUPPORT(nFormat)) { /* 2005.09.21 */
        return true;
    }
    // 事前にタグを更新
    if (isModify(pFileInfo)) {
        if (STEPWriteTag(pFileInfo) == false) {
            return false;
        }
    }
    /*bool result =*/ DeleteTagID3v2(GetFullPath(pFileInfo), hWnd);
    STEPInitDataID3(pFileInfo);
    STEPInitDataSIF(pFileInfo);
    extern bool LoadFileMP3(FILE_INFO *pFileMP3);
    LoadFileMP3(pFileInfo);
    //return result;//false を返すとそれ以降の選択ファイルを処理しなくなる
    return true;
}

bool WINAPI ConvID3v2Version(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd) 
{
    if (nProcFlag == 3) {
        return false;
    }
    if(!ConvFileFormat(pFileInfo, nFileTypeID3V2, hWnd)){//ID3v2 に変換
        return false;
    }
    int nFormat = GetFormat(pFileInfo);
    if (!TYPE_IS_SUPPORT(nFormat)) { /* 2005.09.21 */
        return true;
    }
    // 事前にタグを更新
    if (isModify(pFileInfo)) {
        if (STEPWriteTag(pFileInfo) == false) {
            return false;
        }
    }
    if (GetFormat(pFileInfo) != nFileTypeID3V2) return true;
    CId3tagv2   id3v2;
    if (id3v2.Load(GetFullPath(pFileInfo))  != ERROR_SUCCESS    // 読み込み失敗
    ||  id3v2.IsEnable() == FALSE           // ID3v2 ではない
    ||  !TRUE/*id3v2.IsSafeVer()*/) {       // 未対応のバージョン
        // 読み込み失敗
        return false;
    }
    CString szYear = id3v2.GetYear();//
    extern int  nId3v2EncodeConv;
    extern int  nId3v2VersionConv;
    extern bool bOptUnSyncConv;
    bool needConv = false;
    if (nId3v2VersionConv != 0) {
        // 変更しない以外
        switch (nId3v2VersionConv) {
        case 1: // v2.2
            if (id3v2.GetVer() != 0x0200)   needConv = true;
            id3v2.SetVer(0x0200);
            break;
        case 2: // v2.3
            if (id3v2.GetVer() != 0x0300)   needConv = true;
            id3v2.SetVer(0x0300);
            break;
        case 3: // v2.4
            if (id3v2.GetVer() != 0x0400)   needConv = true;
            id3v2.SetVer(0x0400);
            break;
        default:
            break;
        }
    }
    //ややこし過ぎ…。
    if(nId3v2EncodeConv == 0){//文字エンコードを変更しない
        //id3v2.4以外に変更で、変更前が UTF8 の場合は UTF16 に変換
        if(id3v2.GetCharEncoding() == CId3tagv2::ID3V2CHARENCODING_UTF_8){
            if(id3v2.GetVer() != 0x0400){
                nId3v2EncodeConv = 2;
            }
        }
    }
    else if(nId3v2EncodeConv == 3){//UTF8
        //id3v2.4以外に変更の場合は UTF16 に変換
        if(id3v2.GetVer() != 0x0400){
            nId3v2EncodeConv = 2;
        }
    }
    if (nId3v2EncodeConv != 0) {
        //GetCharEncoding() の戻り値は必ずしも全てのフレームに対してのものではなく
        //実際はフレームごとに文字エンコードが異なる
        //ここではなるべく全てのフレームに対して指定した文字エンコードに変換する
        needConv = true;
        switch (nId3v2EncodeConv) {
        case 1:
        //    if (id3v2.GetCharEncoding() != CId3tagv2::ID3V2CHARENCODING_ISO_8859_1) needConv = true;
            id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_ISO_8859_1);
            break;
        case 2:
        //    if (id3v2.GetCharEncoding() != CId3tagv2::ID3V2CHARENCODING_UTF_16) needConv = true;
            id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_UTF_16);
            break;
        case 3:
        //    if (id3v2.GetCharEncoding() != CId3tagv2::ID3V2CHARENCODING_UTF_8)  needConv = true;
            id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_UTF_8);
            break;
        //case 4:
        //  if (id3v2.GetCharEncoding() != CId3tagv2::ID3V2CHARENCODE_UTF_16BE)   needConv = true;
        //  id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_UTF_16BE);
        //  break;
        default:
            needConv = false;
            break;
        }
    }
    if(id3v2.GetUnSynchronization() != bOptUnSyncConv){
        needConv = true;
    }
    if (needConv == false)  return true;
    id3v2.SetUnSynchronization(bOptUnSyncConv);
    id3v2.ApplyStringEncode();//文字エンコードの変更を反映させる
    id3v2.SetYear(szYear);//"TYER" <=> "TDRC" 変換
    bool result = id3v2.Save(GetFullPath(pFileInfo)) == ERROR_SUCCESS ? true : false;
    if (result) {
        extern bool LoadFileMP3(FILE_INFO *pFileMP3);
        if (LoadFileMP3(pFileInfo) == false) {
            return false;
        }
        // 拡張子の変更
        return ConvExt(pFileInfo);
    }
    return result;
}

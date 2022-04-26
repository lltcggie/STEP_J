// STEP_ape.cpp : DLL 用の初期化処理の定義を行います。
//

#include "stdafx.h"
#include "STEP_ape.h"
#include "STEPlugin.h"

#include "Tag_Ape.h"
#include "Id3tagv1.h"
#include "DlgSetup.h"
#include "FileAPE.h"
#include "..\SuperTagEditor\INI\ini.h"
//設定の読み書き
//WritePrivateProfileString はファイルが存在しない場合や、
//元ファイルが ANSI だと ANSI で文字列を書き込む
//使い辛いので STEP 本体の INI 読み書きクラスを使い回す
//UTF8/UTF16/ANSI 対応

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID3_LEN_TRACK_NAME		30		// トラック名    (文字列…30BYTE)
#define ID3_LEN_ARTIST_NAME		30		// アーティスト名(文字列…30BYTE)
#define ID3_LEN_ALBUM_NAME		30		// アルバム名    (文字列…30BYTE)
#define ID3_LEN_COMMENT			30		// コメント      (文字列…30BYTE)
#define ID3_LEN_YEAR			4		// リリース年号  (文字列… 4BYTE)

//
//	メモ!
//
//		この DLL が MFC DLL に対して動的にリンクされる場合、
//		MFC 内で呼び出されるこの DLL からエクスポートされた
//		どの関数も関数の最初に追加される AFX_MANAGE_STATE 
//		マクロを含んでいなければなりません。
//
//		例:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 通常関数の本体はこの位置にあります
//		}
//
//		このマクロが各関数に含まれていること、MFC 内の
//		どの呼び出しより優先することは非常に重要です。
//		これは関数内の最初のステートメントでなければな
//		らないことを意味します、コンストラクタが MFC 
//		DLL 内への呼び出しを行う可能性があるので、オブ
//		ジェクト変数の宣言よりも前でなければなりません。
//
//		詳細については MFC テクニカル ノート 33 および
//		58 を参照してください。
//

/////////////////////////////////////////////////////////////////////////////
// CSTEP_apeApp

BEGIN_MESSAGE_MAP(CSTEP_apeApp, CWinApp)
	//{{AFX_MSG_MAP(CSTEP_apeApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSTEP_apeApp の構築

CSTEP_apeApp::CSTEP_apeApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance の中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CSTEP_apeApp オブジェクト

CSTEP_apeApp theApp;

UINT nPluginID;
//UINT nFileTypeAPE;
//UINT nFileTypeAPEID3; /* タグがID3形式 */

TCHAR g_szPluginFolder[MAX_PATH];
static struct FILE_TYPES
{
    UINT nFileTypeSIF;
    UINT nFileTypeID3;
    LPCTSTR cszBMP;
    LPCTSTR cszExt;
    LPCTSTR cszTypeName;
}g_FileTypes[] = {
    {0, 0, MAKEINTRESOURCE(IDB_BITMAP_APE), _T("ape"), _T("Monkey's Audio")},
    {0, 0, MAKEINTRESOURCE(IDB_BITMAP_MPC), _T("mpc"), _T("Musepack")},
    {0, 0, MAKEINTRESOURCE(IDB_BITMAP_MPC), _T("mp+"), _T("Musepack")},
    {0, 0, MAKEINTRESOURCE(IDB_BITMAP_APE), _T("tak"), _T("TAK")},
    {0, 0, MAKEINTRESOURCE(IDB_BITMAP_APE), _T("ofr"), _T("OptimFROG")},
    {0, 0, MAKEINTRESOURCE(IDB_BITMAP_APE), _T("ofs"), _T("OptimFROG")},
    {0, 0, MAKEINTRESOURCE(IDB_BITMAP_APE), _T("wv"), _T("WavPack")},
    {0, 0, NULL, NULL, NULL}
};

CString strINI;
bool bOptGenreListSelect;

UINT GetFileTypeFromExt(const TCHAR *cszExt)
{
    int i = 0;
    while(g_FileTypes[i].cszExt){
        if(_tcsicmp(cszExt, g_FileTypes[i].cszExt) == 0){
            return g_FileTypes[i].nFileTypeSIF;
        }
        i++;
    }
    return 0;
}
UINT GetFileTypeID3FromExt(const TCHAR *cszExt)
{
    int i = 0;
    while(g_FileTypes[i].cszExt){
        if(_tcsicmp(cszExt, g_FileTypes[i].cszExt) == 0){
            return g_FileTypes[i].nFileTypeID3;
        }
        i++;
    }
    return 0;
}
bool IsSupportTypeSIF(int nFormat)
{
    int i = 0;
    while(g_FileTypes[i].cszExt){
        if(g_FileTypes[i].nFileTypeSIF == nFormat){
            return true;
        }
        i++;
    }
    return false;
}
bool IsSupportTypeID3(int nFormat)
{
    int i = 0;
    while(g_FileTypes[i].cszExt){
        if(g_FileTypes[i].nFileTypeID3 == nFormat){
            return true;
        }
        i++;
    }
    return false;
}
bool IsSupportExt(const TCHAR *cszExt)
{
    int i = 0;
    while(g_FileTypes[i].cszExt){
        if(_tcsicmp(cszExt, g_FileTypes[i].cszExt) == 0){
            return true;
        }
        i++;
    }
    return false;
}
LPCTSTR GetFileTypeName(const TCHAR *cszExt)
{
    int i = 0;
    while(g_FileTypes[i].cszExt){
        if(_tcsicmp(cszExt, g_FileTypes[i].cszExt) == 0){
            return g_FileTypes[i].cszTypeName;
        }
        i++;
    }
    return _T("");
}
LPCTSTR GetFileTypeName(int nFormat)
{
    int i = 0;
    while(g_FileTypes[i].cszExt){
        if(nFormat == g_FileTypes[i].nFileTypeSIF){
            return g_FileTypes[i].cszTypeName;
        }
        if(nFormat == g_FileTypes[i].nFileTypeID3){
            return g_FileTypes[i].cszTypeName;
        }
        i++;
    }
    return _T("");
}

STEP_API LPCTSTR WINAPI STEPGetPluginInfo(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return _T("Version 1.01 Copyright (C) 2003-2006 haseta\r\n")
           _T("Version 1.03 Copyright (C) 2016 Kobarin\r\n")
           _T("Monkey's Audio/Musepack/TAK/WavPack/OptimFROG 形式をサポートしています");
}


STEP_API bool WINAPI STEPInit(UINT pID, LPCTSTR szPluginFolder)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (Initialize() == false)	return false;
	nPluginID = pID;
    _tcsncpy_s(g_szPluginFolder, szPluginFolder, _TRUNCATE);
	// INIファイルの読み込み
	strINI = szPluginFolder;
	strINI += _T("STEP_ape.ini");
    CIniFile iniFile(strINI);
	bOptGenreListSelect = iniFile.ReadInt(_T("APE"), _T("GenreListSelect"), 0) != 0;

    int i = 0;
    while(g_FileTypes[i].cszExt){
    	HBITMAP hBitmap = LoadBitmap(theApp.m_hInstance, g_FileTypes[i].cszBMP);
        g_FileTypes[i].nFileTypeSIF = STEPRegisterExt(nPluginID, g_FileTypes[i].cszExt, hBitmap);
	    g_FileTypes[i].nFileTypeID3 = STEPRegisterExt(nPluginID, g_FileTypes[i].cszExt, hBitmap);
        DeleteObject(hBitmap);
        i++;
    }
	return true;
}

STEP_API void WINAPI STEPFinalize() {
	Finalize();
}

STEP_API UINT WINAPI STEPGetAPIVersion(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return STEP_API_VERSION;
}

STEP_API LPCTSTR WINAPI STEPGetPluginName(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return _T("STEP_ape");
}

STEP_API bool WINAPI STEPSupportSIF(UINT nFormat) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return true;
}

STEP_API bool WINAPI STEPSupportTrackNumberSIF(UINT nFormat) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return true;
}

STEP_API CONTROLTYPE WINAPI STEPGetControlType(UINT nFormat, COLUMNTYPE nColumn, bool isEditSIF)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!isEditSIF)	return _NULL;
	switch (nColumn) {
	case COLUMN_TRACK_NAME:
	case COLUMN_ARTIST_NAME:
	case COLUMN_ALBUM_NAME:
    case COLUMN_ALBM_ARTIST:
	case COLUMN_TRACK_NUMBER:
	case COLUMN_TRACK_TOTAL:
    case COLUMN_DISC_NUMBER:
    case COLUMN_DISC_TOTAL:
	case COLUMN_YEAR:
    case COLUMN_COPYRIGHT:
	case COLUMN_WRITER:
	case COLUMN_COMPOSER:
	case COLUMN_ORIG_ARTIST:
    case COLUMN_SOFTWARE:
		return _EDIT;
	case COLUMN_COMMENT:
		return _MEDIT;
	case COLUMN_GENRE:
		if (bOptGenreListSelect) {
			return _CBOX;
		} else {
			return _EDIT;
		}
	}
	return _NULL;
}

STEP_API UINT WINAPI STEPGetColumnMax(UINT nFormat, COLUMNTYPE nColumn, bool isEditSIF) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (/*IsSupportTypeSIF(nFormat) ||*/ true) {
		switch (nColumn) {
		case COLUMN_TRACK_NAME:
		case COLUMN_ARTIST_NAME:
		case COLUMN_ALBUM_NAME:
		case COLUMN_ALBM_ARTIST:
        case COLUMN_YEAR:
		case COLUMN_GENRE:
        case COLUMN_COMMENT:
        case COLUMN_COPYRIGHT:
		case COLUMN_WRITER:
        case COLUMN_COMPOSER:
        case COLUMN_SOFTWARE:
		case COLUMN_ORIG_ARTIST:
            return 1024;
		case COLUMN_TRACK_NUMBER:
		case COLUMN_TRACK_TOTAL:
        case COLUMN_DISC_NUMBER:
        case COLUMN_DISC_TOTAL:
            return 32;
		}
	/*
	} else if (nFormat == nFileTypeAPEID3) {
		switch (nColumn) {
		case COLUMN_TRACK_NAME:		return ID3_LEN_TRACK_NAME;
		case COLUMN_ARTIST_NAME:	return ID3_LEN_ARTIST_NAME;
		case COLUMN_ALBUM_NAME:		return ID3_LEN_ALBUM_NAME;
		case COLUMN_TRACK_NUMBER:	return 3;
		case COLUMN_YEAR:			return ID3_LEN_YEAR;
		case COLUMN_GENRE:			return 3;
		case COLUMN_COMMENT:		return ID3_LEN_COMMENT;
		}
	*/
	}
	return 0;
}

bool ReadTagAPE(FILE_INFO *pFileMP3, const TCHAR *cszExt)
{
	CTag_Ape ape;
	if (ape.Load(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
		return false;
	}
	
	CString buff; 
//    if(ape.IsEnable()){
	    // トラック名
	    ape.GetComment(CTag_Ape::APE_TAG_FIELD_TITLE, buff);
	    SetTrackNameSI(pFileMP3, buff);
	    // アーティスト名
	    ape.GetComment(CTag_Ape::APE_TAG_FIELD_ARTIST, buff);
	    SetArtistNameSI(pFileMP3, buff);
	    // アルバム名
	    ape.GetComment(CTag_Ape::APE_TAG_FIELD_ALBUM, buff);
	    SetAlbumNameSI(pFileMP3, buff);
        // アルバムアーティスト
        ape.GetComment(_T("Album Artist"), buff);//Album と Artist の間に空白を含む
        SetAlbumArtistSI(pFileMP3, buff);
	    // リリース年号
	    ape.GetComment(CTag_Ape::APE_TAG_FIELD_YEAR, buff);
	    SetYearSI(pFileMP3, buff);
	    // コメント
	    ape.GetComment(CTag_Ape::APE_TAG_FIELD_COMMENT, buff);
	    SetCommentSI(pFileMP3, buff);
	    // トラック番号
	    ape.GetComment(CTag_Ape::APE_TAG_FIELD_TRACK, buff);
	    // x/y 形式になっていれば y の方をトラック数として処理
        {
            TCHAR *trk_number = buff.GetBuffer();
            TCHAR *trk_total = _tcschr(trk_number, _T('/'));
            if(trk_total){
                *trk_total++ = 0;
            }
            SetTrackNumberSI(pFileMP3, trk_number);
            if(trk_total){
                SetTrackTotalSI(pFileMP3, trk_total);
            }
            buff.ReleaseBuffer();
        }
	    // ディスク番号
	    ape.GetComment(_T("Disc"), buff);
	    // x/y 形式になっていれば y の方をディスク数として処理
        {
            TCHAR *disc_number = buff.GetBuffer();
            TCHAR *disc_total = _tcschr(disc_number, _T('/'));
            if(disc_total){
                *disc_total++ = 0;
            }
            SetDiscNumberSI(pFileMP3, disc_number);
            if(disc_total){
                SetDiscTotalSI(pFileMP3, disc_total);
            }
            buff.ReleaseBuffer();
        }
	    // ジャンル
	    ape.GetComment(CTag_Ape::APE_TAG_FIELD_GENRE, buff);
	    SetGenreSI(pFileMP3, buff);
	    //SetBGenre(STEPGetGenreCode(buff));
	    // 著作権
        ape.GetComment(_T("Copyright"), buff);
        SetCopyrightSI(pFileMP3, buff);
        // 作詞者
	    ape.GetComment(_T("Lyricist"), buff);
	    SetWriterSI(pFileMP3, buff);
	    // 作曲者
	    ape.GetComment(_T("Composer"), buff);
	    SetComposerSI(pFileMP3, buff);
	    // 演奏者
	    ape.GetComment(_T("Performer"), buff);
	    SetOrigArtistSI(pFileMP3, buff);
        // ソフトウェア
        ape.GetComment(_T("Encoded By"), buff);
        SetSoftwareSI(pFileMP3, buff);
//    }
/*    else if(ape.HasId3tag()){
        CId3tagv1 id3v1;
        id3v1.Load(GetFullPath(pFileMP3));
	    SetTrackNameSI(pFileMP3, id3v1.GetTitle());
	    SetArtistNameSI(pFileMP3, id3v1.GetArtist());
	    SetAlbumNameSI(pFileMP3, id3v1.GetAlbum());
	    SetYearSI(pFileMP3, id3v1.GetYear());
	    SetCommentSI(pFileMP3, id3v1.GetComment());
        SetTrackNumberSI(pFileMP3, id3v1.GetTrackNo());
	    SetGenreSI(pFileMP3, id3v1.GetGenre());
    }
*/
	// ファイル形式
    TCHAR szFileTypeName[1024];
    _tcsncpy_s(szFileTypeName, GetFileTypeName(cszExt), _TRUNCATE);
	if (!ape.IsEnable() && ape.HasId3tag()) {
		SetFormat(pFileMP3, GetFileTypeID3FromExt(cszExt));
        _tcsncat_s(szFileTypeName, _T("(ID3)"), _TRUNCATE);
	} else {
		SetFormat(pFileMP3, GetFileTypeFromExt(cszExt));
		if (ape.IsEnable()) {
			if (ape.GetApeVersion() == 1000) {
               _tcsncat_s(szFileTypeName, _T("(APE)"), _TRUNCATE);
			} else {
               _tcsncat_s(szFileTypeName, _T("(APEv2)"), _TRUNCATE);
			}
		}
	}
    SetFileTypeName(pFileMP3, szFileTypeName);
    // 
    SetAudioFormatFromExt(pFileMP3, cszExt);
	return true;
}

STEP_API UINT WINAPI STEPLoad(FILE_INFO *pFileMP3, LPCTSTR szExt)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if(IsSupportExt(szExt)){
        if (ReadTagAPE(pFileMP3, szExt) == false) {
			CString	strMsg;
			strMsg.Format(_T("%s の読み込みに失敗しました"), GetFullPath(pFileMP3));
            CString strTitle;
            strTitle.Format(_T("%sファイルの読み込み失敗"), GetFileTypeName(szExt));
			MessageBox(NULL, strMsg, strTitle, MB_ICONSTOP|MB_OK|MB_TOPMOST);
			return STEP_ERROR;
		} else {
			return STEP_SUCCESS;
		}
	}
	return STEP_UNKNOWN_FORMAT;
}

bool WriteTagAPE(FILE_INFO *pFileMP3)
{
	CTag_Ape ape;
	if (ape.Load(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
		return false;
	}
	// トラック名
	ape.SetComment(CTag_Ape::APE_TAG_FIELD_TITLE, GetTrackNameSI(pFileMP3));
	// アーティスト名
	ape.SetComment(CTag_Ape::APE_TAG_FIELD_ARTIST, GetArtistNameSI(pFileMP3));
	// アルバム名
	ape.SetComment(CTag_Ape::APE_TAG_FIELD_ALBUM, GetAlbumNameSI(pFileMP3));
	// アルバムアーティスト
	ape.SetComment(_T("Album Artist"), GetAlbumArtistSI(pFileMP3));
	// リリース年号
	ape.SetComment(CTag_Ape::APE_TAG_FIELD_YEAR, GetYearSI(pFileMP3));
	// コメント
	ape.SetComment(CTag_Ape::APE_TAG_FIELD_COMMENT, GetCommentSI(pFileMP3));
	// トラック番号/トラック数
    {//TrackTotal というフィールド名は Ape にはないので "トラック番号/トラック数" の
     //ような文字列に変換して書き込む
        CString strTrackNumber = GetTrackNumberSI(pFileMP3);
        CString strTrackTotal = GetTrackTotalSI(pFileMP3);
        if(!strTrackNumber.IsEmpty() && !strTrackTotal.IsEmpty()){
            strTrackNumber = strTrackNumber + _T("/") + strTrackTotal;
        }
        ape.SetComment(CTag_Ape::APE_TAG_FIELD_TRACK, strTrackNumber);
    }
    // ディスク番号/ディスク数
    {//DiscTotal というフィールド名は Ape にはないので "ディスク番号/ディスク数" の
     //ような文字列に変換して書き込む
        CString strDiscNumber = GetDiscNumberSI(pFileMP3);
        CString strDiscTotal = GetDiscTotalSI(pFileMP3);
        if(!strDiscNumber.IsEmpty() && !strDiscTotal.IsEmpty()){
            strDiscNumber = strDiscNumber + _T("/") + strDiscTotal;
        }
        ape.SetComment(_T("Disc"), strDiscNumber);
    }
    // ジャンル番号
	ape.SetComment(CTag_Ape::APE_TAG_FIELD_GENRE, GetGenreSI(pFileMP3));
	// 著作権
    ape.SetComment(_T("Copyright"), GetCopyrightSI(pFileMP3));
	// 作詞者
	ape.SetComment(_T("Lyricist"), GetWriterSI(pFileMP3));
	// 作曲者
	ape.SetComment(_T("Composer"), GetComposerSI(pFileMP3));
	// 演奏者
	ape.SetComment(_T("Performer"), GetOrigArtistSI(pFileMP3));
    // ソフトウェア
	ape.SetComment(_T("Encoded By"), GetSoftwareSI(pFileMP3));
    //  ID3v1 を保存しない
    ape.SetDonotsaveId3v1(TRUE);
	if (ape.Save(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
		return false;
	}
	return true;
}

STEP_API UINT WINAPI STEPSave(FILE_INFO *pFileMP3)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	UINT nFormat = GetFormat(pFileMP3);

	if(IsSupportTypeSIF(nFormat) || IsSupportTypeID3(nFormat)){
        if (WriteTagAPE(pFileMP3) == false) {
			CString	strMsg;
            CString strTitle;
			strMsg.Format(_T("%s の書き込みに失敗しました"), GetFullPath(pFileMP3));
            strTitle.Format(_T("%sファイルの書き込み失敗"), GetFileTypeName(nFormat));
			MessageBox(NULL, strMsg, strTitle, MB_ICONSTOP|MB_OK|MB_TOPMOST);
			return STEP_ERROR;
		}
		return STEP_SUCCESS;
	}
	return STEP_UNKNOWN_FORMAT;
}

STEP_API void WINAPI STEPShowOptionDialog(HWND hWnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDlgSetup dlg1;
	CPropertySheet page;
	dlg1.m_bGenreListSelect = bOptGenreListSelect;
	page.AddPage(&dlg1);
	page.SetTitle(CString(STEPGetPluginName()) + _T(" オプション設定"));
	if (page.DoModal() == IDOK) {
		bOptGenreListSelect = dlg1.m_bGenreListSelect ? true : false;
        CIniFile iniFile(strINI);
        iniFile.WriteInt(_T("APE"), _T("GenreListSelect"), bOptGenreListSelect);
        iniFile.Flush();//保存実行
	}
}

STEP_API LPCTSTR WINAPI STEPGetColumnName(UINT nFormatType, COLUMNTYPE nColumn)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch (nColumn) {
	case COLUMN_ORIG_ARTIST:
		return _T("演奏者");
	}
	return NULL;
}

STEP_API bool WINAPI STEPHasSpecificColumnName(UINT)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return true;
}

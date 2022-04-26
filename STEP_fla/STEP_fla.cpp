// STEP_fla.cpp : DLL 用の初期化処理の定義を行います。
//

#include "stdafx.h"
#include "STEP_fla.h"
#include "STEPlugin.h"

#include "DlgSetup.h"
#include "flac/format.h"

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
// CSTEP_flaApp

BEGIN_MESSAGE_MAP(CSTEP_flaApp, CWinApp)
	//{{AFX_MSG_MAP(CSTEP_flaApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSTEP_flaApp の構築

CSTEP_flaApp::CSTEP_flaApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance の中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CSTEP_flaApp オブジェクト

CSTEP_flaApp theApp;

UINT nPluginID;
UINT nFileTypeFLAC;

CString strINI;
bool bOptGenreListSelect;
CString strPluginInfo;

STEP_API LPCTSTR WINAPI STEPGetPluginInfo(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	strPluginInfo = _T("Version 1.00 Copyright (C) 2004-2005 haseta\r\n")
                    _T("Version 1.03 Copyright (C) 2016 Kobarin\r\n")
                    _T("FLAC形式をサポートしています\r\n")
                    _T("タグ更新にはlibFLAC(");
	strPluginInfo += FLAC__VERSION_STRING;
	strPluginInfo += _T(")を使用しています");
	return strPluginInfo;
}

STEP_API bool WINAPI STEPInit(UINT pID, LPCTSTR szPluginFolder)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (Initialize() == false)	return false;
	nPluginID = pID;
    //FLAC のファイルオープン関数を utf8 モードにする
//    flac_set_utf8_filenames(true);
	// INIファイルの読み込み
	strINI = szPluginFolder;
	strINI += "STEP_fla.ini";
    CIniFile iniFile(strINI);
	bOptGenreListSelect = iniFile.ReadInt(_T("FLAC"), _T("GenreListSelect"), 0) != 0;

	HBITMAP hFLACBitmap = LoadBitmap(theApp.m_hInstance, MAKEINTRESOURCE(IDB_BITMAP_FLAC));
	nFileTypeFLAC = STEPRegisterExt(nPluginID, _T("flac"), hFLACBitmap);
	DeleteObject(hFLACBitmap);
	HBITMAP hFLABitmap = LoadBitmap(theApp.m_hInstance, MAKEINTRESOURCE(IDB_BITMAP_FLAC));
	nFileTypeFLAC = STEPRegisterExt(nPluginID, _T("fla"), hFLABitmap);
	DeleteObject(hFLABitmap);

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
	return _T("STEP_fla");
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
	case COLUMN_ORIG_ARTIST:
	case COLUMN_SOFTWARE:
        return 1024;
	case COLUMN_TRACK_NUMBER:
	case COLUMN_TRACK_TOTAL:
	case COLUMN_DISC_NUMBER:
	case COLUMN_DISC_TOTAL:    
        return 32;
	}
	return 0;
}

STEP_API UINT WINAPI STEPLoad(FILE_INFO *pFileMP3, LPCTSTR szExt)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_tcsicmp(szExt, _T("flac")) == 0 || _tcsicmp(szExt, _T("fla")) == 0) {
		extern	bool LoadFileFLAC(FILE_INFO *pFile);
		if (LoadFileFLAC(pFileMP3) == false) {
			CString	strMsg;
			strMsg.Format(_T("%s の読み込みに失敗しました"), GetFullPath(pFileMP3));
			MessageBox(NULL, strMsg, _T("FLACファイルの読み込み失敗"), MB_ICONSTOP|MB_OK|MB_TOPMOST);
			return STEP_ERROR;
		} else {
			SetFormat(pFileMP3, nFileTypeFLAC);
			SetFileTypeName(pFileMP3, _T("FLAC"));
			return STEP_SUCCESS;
		}
	}
	return STEP_UNKNOWN_FORMAT;
}

STEP_API UINT WINAPI STEPSave(FILE_INFO *pFileMP3)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	UINT nFormat = GetFormat(pFileMP3);

	if (nFormat == nFileTypeFLAC) {
		extern bool WriteFileFLAC(FILE_INFO *pFileMP3);
		if (WriteFileFLAC(pFileMP3) == false) {
			CString	strMsg;
			strMsg.Format(_T("%s の書き込みに失敗しました"), GetFullPath(pFileMP3));
			MessageBox(NULL, strMsg, _T("FLACファイルの書き込み失敗"), MB_ICONSTOP|MB_OK|MB_TOPMOST);
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
		iniFile.WriteInt(_T("FLAC"), _T("GenreListSelect"), bOptGenreListSelect);
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

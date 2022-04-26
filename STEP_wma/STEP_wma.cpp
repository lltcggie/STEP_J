// STEP_wma.cpp : DLL 用の初期化処理の定義を行います。
//

#include "stdafx.h"
#include "STEP_wma.h"
#include "STEPlugin.h"

#include "FileWMA.h"
#include "DlgSetup.h"

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
// CSTEP_wmaApp

BEGIN_MESSAGE_MAP(CSTEP_wmaApp, CWinApp)
	//{{AFX_MSG_MAP(CSTEP_wmaApp)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSTEP_wmaApp の構築

CSTEP_wmaApp::CSTEP_wmaApp()
{
	// TODO: この位置に構築用のコードを追加してください。
	// ここに InitInstance の中の重要な初期化処理をすべて記述してください。
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CSTEP_wmaApp オブジェクト

CSTEP_wmaApp theApp;

UINT nPluginID;
UINT nFileTypeWMA;
UINT nFileTypeWMV;
UINT nFileTypeASF;

CString strINI;
bool bOptGenreListSelect;

STEP_API LPCTSTR WINAPI STEPGetPluginInfo(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return _T("Version 1.01 Copyright (C) 2003-2006 haseta\r\n")
           _T("Version 1.03M Copyright (C) 2008-2010 Mimura\r\n")
           _T("Version 1.05 Copyright (C) 2016 Kobarin\r\n")
           _T("WMA/WMV/ASF形式をサポートしています");
}

STEP_API bool WINAPI STEPInit(UINT pID, LPCTSTR szPluginFolder)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (Initialize() == false)	return false;
	nPluginID = pID;

	// INIファイルの読み込み
	strINI = szPluginFolder;
	strINI += _T("STEP_wma.ini");
    CIniFile iniFile(strINI);
	bOptGenreListSelect = iniFile.ReadInt(_T("WMA"), _T("GenreListSelect"), 0) != 0;

	HBITMAP hWMABitmap = LoadBitmap(theApp.m_hInstance, MAKEINTRESOURCE(IDB_BITMAP_WMA));
	HBITMAP hWMVBitmap = LoadBitmap(theApp.m_hInstance, MAKEINTRESOURCE(IDB_BITMAP_WMA));
	HBITMAP hASFBitmap = LoadBitmap(theApp.m_hInstance, MAKEINTRESOURCE(IDB_BITMAP_WMA));
	nFileTypeWMA = STEPRegisterExt(nPluginID, _T("wma"), hWMABitmap);
	nFileTypeWMV = STEPRegisterExt(nPluginID, _T("wmv"), hWMVBitmap);
	nFileTypeASF = STEPRegisterExt(nPluginID, _T("asf"), hASFBitmap);
	DeleteObject(hWMABitmap);
	DeleteObject(hWMVBitmap);
	DeleteObject(hASFBitmap);

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
	return _T("STEP_wma");
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
	case COLUMN_TRACK_NUMBER:
	case COLUMN_DISC_NUMBER:
	case COLUMN_YEAR:
	case COLUMN_URL:
	case COLUMN_COPYRIGHT:
	case COLUMN_OTHER:
	case COLUMN_ORIG_ARTIST: /* 2005.08.19 add */
	case COLUMN_COMPOSER: /* 2005.08.19 add */
	case COLUMN_ENGINEER: /* 2005.09.07 add */
	case COLUMN_ALBM_ARTIST: /* Mimura add */
	case COLUMN_WRITER: /* Mimura add */
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
	case COLUMN_TRACK_NAME:		return 1024;
	case COLUMN_ARTIST_NAME:	return 1024;
	case COLUMN_ALBUM_NAME:		return 1024;
	case COLUMN_YEAR:			return 1024;
	case COLUMN_GENRE:			return 1024;
	case COLUMN_COMMENT:		return 1024;
	case COLUMN_URL:			return 1024;
	case COLUMN_COPYRIGHT:		return 1024;
	case COLUMN_OTHER:			return 1024;
	case COLUMN_ORIG_ARTIST:	return 1024; /* 2005.08.19 add */
	case COLUMN_COMPOSER:		return 1024; /* 2005.08.19 add */
	case COLUMN_ENGINEER:		return 1024; /* 2005.09.07 add */
	case COLUMN_ALBM_ARTIST:	return 1024; /* Mimura add */
	case COLUMN_WRITER:			return 1024; /* Mimura add */
    case COLUMN_SOFTWARE:		return 1024; 
	case COLUMN_TRACK_NUMBER:	return 32;
	case COLUMN_DISC_NUMBER:	return 32;
	}
	return 0;
}

STEP_API UINT WINAPI STEPLoad(FILE_INFO *pFileMP3, LPCTSTR szExt)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_tcsicmp(szExt, _T("wma")) == 0 || _tcsicmp(szExt, _T("wmv")) == 0 || _tcsicmp(szExt, _T("asf")) == 0) {
		if (LoadFileWMA(pFileMP3) == false) {
			CString	strMsg;
			strMsg.Format(_T("%s の読み込みに失敗しました"), GetFullPath(pFileMP3));
			MessageBox(NULL, strMsg, _T("WMAファイルの読み込み失敗"), MB_ICONSTOP|MB_OK|MB_TOPMOST);
			return STEP_ERROR;
		} else {
			if (_tcsicmp(szExt, _T("wma")) == 0) {
				SetFormat(pFileMP3, nFileTypeWMA);
				SetFileTypeName(pFileMP3, _T("WMA"));
				return STEP_SUCCESS;
			}
			if (_tcsicmp(szExt, _T("wmv")) == 0) {
				SetFormat(pFileMP3, nFileTypeWMV);
				SetFileTypeName(pFileMP3, _T("WMV"));
				return STEP_SUCCESS;
			}
			if (_tcsicmp(szExt, _T("asf")) == 0) {
				SetFormat(pFileMP3, nFileTypeASF);
				SetFileTypeName(pFileMP3, _T("ASF"));
				return STEP_SUCCESS;
			}
		}
	}
	return STEP_UNKNOWN_FORMAT;
}

STEP_API UINT WINAPI STEPSave(FILE_INFO *pFileMP3)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	UINT nFormat = GetFormat(pFileMP3);

	if (nFormat == nFileTypeWMA || nFormat == nFileTypeWMV || nFormat == nFileTypeASF) {
		if (WriteFileWMA(pFileMP3) == false) {
			CString	strMsg;
			strMsg.Format(_T("%s の書き込みに失敗しました"), GetFullPath(pFileMP3));
			MessageBox(NULL, strMsg, _T("WMAファイルの書き込み失敗"), MB_ICONSTOP|MB_OK|MB_TOPMOST);
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
		iniFile.WriteInt(_T("WMA"), _T("GenreListSelect"), bOptGenreListSelect);
        iniFile.Flush();//保存実行
	}
}



/*
STEP_API LPCTSTR WINAPI STEPGetToolTipText(UINT nID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return NULL;
}
*/

/*
STEP_API LPCTSTR WINAPI STEPGetStatusMessage(UINT nID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return NULL;
}
*/

/*
STEP_API bool WINAPI STEPOnUpdateCommand(UINT nID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return false;
}
*/

/*
STEP_API bool WINAPI STEPOnCommand(UINT nID, HWND hWnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return false;
}
*/

/*
STEP_API void WINAPI STEPOnLoadMenu(HMENU hMenu, UINT nType)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}
*/


STEP_API LPCTSTR WINAPI STEPGetColumnName(UINT nFormatType, COLUMNTYPE nColumn)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch (nColumn) {
	case COLUMN_URL:			// URL
		return _T("URL(関連)");
	case COLUMN_OTHER:			// その他
		return _T("URL(Album)");
		break;
	case COLUMN_ENGINEER:		// エンジニア(制作者) /* 2005.09.07 add */
		return _T("レーベル");
		break;
	}
	return NULL;
}

STEP_API bool WINAPI STEPHasSpecificColumnName(UINT)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return true;
}


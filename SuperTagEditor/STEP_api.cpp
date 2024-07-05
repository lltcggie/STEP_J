#include "stdafx.h"

#define APSTUDIO_INVOKED
#include "resource.h"
#undef APSTUDIO_INVOKED

#include "STEP_api.h"

#include "DkToolBar.h"
#include "SuperTagEditor.h"
#include "FileMP3.h"
#include "MySuperGrid.h"
#include "SuperTagEditorDoc.h"

#include "Plugin.h"

#include "INI/ini.h"

CDkToolBar* STEP_wndToolBar;
CImageList* STEP_ImageList;
HWND STEP_hWnd;
CMySuperGrid* STEP_List;

UINT nCommandID = _APS_NEXT_COMMAND_VALUE;
UINT nFileTypeID = 1;
CUIntArray arFormatType; // FILE_MP3.nFormatに設定される値のリスト

extern "C" {
extern STEP_API UINT WINAPI STEPGetCommandID(void);
extern STEP_API HMENU WINAPI STEPGetMenu(UINT);
extern STEP_API void WINAPI STEPAddToolBarButton(HBITMAP, UINT, TCHAR*);
extern STEP_API UINT WINAPI STEPRegisterExt(UINT, LPCTSTR, HBITMAP);
extern STEP_API const TCHAR* WINAPI STEPGetGenreNameSIF(BYTE byGenre);
extern STEP_API BYTE WINAPI STEPGetGenreCode(const TCHAR* szGenre);
extern STEP_API bool WINAPI STEPIsUserGenre(const TCHAR* szGenre);
extern STEP_API int WINAPI STEPGetNumericTrackNumber(const TCHAR* szTrackNumber, TCHAR* szNumericNumber, int size); /* STEP 037 */
extern STEP_API int WINAPI STEPGetIntegerTrackNumber(const TCHAR* szTrackNumber); /* STEP 037 */
extern STEP_API bool WINAPI STEPIsNumeric(const TCHAR* szText); /* STEP 037 */
extern STEP_API void WINAPI STEPConvSiFieldToId3tag(FILE_INFO* pFileInfo); /* STEP 037 */
}

CPlugin plugins;

PSTEPlugin STEPluginLoadFile(LPCTSTR strPluginFile)
{
    HINSTANCE hLib;
    TCHAR szPluginFullPath[MAX_PATH];//プラグインファイル名(フルパス)
    TCHAR szPluginFolder[MAX_PATH];  //プラグインのフォルダ(フルパス)
    {
        TCHAR   szCurDir[MAX_PATH];
        TCHAR   szExePath[MAX_PATH];
        TCHAR   drive[_MAX_DRIVE];
        TCHAR   dir[_MAX_DIR];
        TCHAR   fname[_MAX_FNAME];
        TCHAR   ext[_MAX_EXT];

        GetCurrentDirectory(MAX_PATH, szCurDir);//カレントディレクトリを保存
        GetModuleFileName(NULL, szExePath, MAX_PATH);//EXE のパス
        _tsplitpath_s(szExePath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
        _tmakepath_s(szExePath, drive, dir, _T(""), _T(""));
        //Exe のフォルダをカレントディレクトリに設定
        SetCurrentDirectory(szExePath);
        //Exe(カレントディレクトリ) からの相対パスからプラグインの絶対パスを取得
        _tfullpath(szPluginFullPath, strPluginFile, MAX_PATH);
        //プラグインのフォルダを取得
        _tsplitpath_s(szPluginFullPath, drive, dir, fname, ext);
        _tmakepath_s(szPluginFolder, drive, dir, _T(""), _T(""));
        //プラグインのフォルダをDLL検索ディレクトリに設定
        //(プラグインの依存 DLL をプラグイン自身と同じフォルダから優先的に
        // ロードされるようにするため)
        SetDllDirectory(szPluginFolder);
        hLib = LoadLibrary(szPluginFullPath);
        SetDllDirectory(NULL);//DLL検索ディレクトリを元に戻す
        SetCurrentDirectory(szCurDir);//カレントディレクトリを元に戻す
        if (hLib == NULL) return NULL;
    }

    UINT (WINAPI *STEPGetAPIVersion)(void);
    bool (WINAPI *STEPInit)(UINT, LPCTSTR);
    LPCTSTR (WINAPI *STEPGetPluginName)(void);

    (FARPROC&)STEPInit = GetProcAddress(hLib, "STEPInit");
    (FARPROC&)STEPGetAPIVersion = GetProcAddress(hLib, "STEPGetAPIVersion");
    (FARPROC&)STEPGetPluginName = GetProcAddress(hLib, "STEPGetPluginName");
    if (STEPInit == NULL || STEPGetAPIVersion == NULL || STEPGetPluginName == NULL) return  NULL;
    if ((STEPGetAPIVersion() & 0xFFFF00) > (STEP_API_VERSION & 0xFFFF00)) return NULL; /* STEP 029 */
    PSTEPlugin pPlugin = new STEPlugin;
    pPlugin->hLib = hLib;
    pPlugin->sFileName = strPluginFile;
    pPlugin->sPluginName = STEPGetPluginName();
    (FARPROC&)pPlugin->STEPGetToolTipText = GetProcAddress(hLib, "STEPGetToolTipText");
    (FARPROC&)pPlugin->STEPGetStatusMessage = GetProcAddress(hLib, "STEPGetStatusMessage");
    (FARPROC&)pPlugin->STEPOnUpdateCommand = GetProcAddress(hLib, "STEPOnUpdateCommand");
    (FARPROC&)pPlugin->STEPOnCommand = GetProcAddress(hLib, "STEPOnCommand");
    (FARPROC&)pPlugin->STEPSupportSIF = GetProcAddress(hLib, "STEPSupportSIF");
    (FARPROC&)pPlugin->STEPSupportTrackNumberSIF = GetProcAddress(hLib, "STEPSupportTrackNumberSIF");
    (FARPROC&)pPlugin->STEPSupportGenreSIF = GetProcAddress(hLib, "STEPSupportGenreSIF");
    (FARPROC&)pPlugin->STEPGetControlType = GetProcAddress(hLib, "STEPGetControlType");
    (FARPROC&)pPlugin->STEPGetColumnMax = GetProcAddress(hLib, "STEPGetColumnMax");
    (FARPROC&)pPlugin->STEPGetColumnName = GetProcAddress(hLib, "STEPGetColumnName");
    (FARPROC&)pPlugin->STEPHasSpecificColumnName = GetProcAddress(hLib, "STEPHasSpecificColumnName");
    (FARPROC&)pPlugin->STEPLoad = GetProcAddress(hLib, "STEPLoad");
    (FARPROC&)pPlugin->STEPSave = GetProcAddress(hLib, "STEPSave");
    (FARPROC&)pPlugin->STEPShowOptionDialog = GetProcAddress(hLib, "STEPShowOptionDialog");
    (FARPROC&)pPlugin->STEPOnLoadMenu = GetProcAddress(hLib, "STEPOnLoadMenu");
    (FARPROC&)pPlugin->STEPOnLoadMainMenu = GetProcAddress(hLib, "STEPOnLoadMainMenu");
    (FARPROC&)pPlugin->STEPGetPluginInfo = GetProcAddress(hLib, "STEPGetPluginInfo");
    (FARPROC&)pPlugin->STEPInitFileSpecificInfo = GetProcAddress(hLib, "STEPInitFileSpecificInfo");
    (FARPROC&)pPlugin->STEPOnConvSiFieldToId3tag = GetProcAddress(hLib, "STEPOnConvSiFieldToId3tag");

    plugins.arPlugins.Add(pPlugin);

    if ((*STEPInit)(plugins.arPlugins.GetSize(), szPluginFolder) == false) {
        return NULL;
    }

    return pPlugin;
}

static void CreateDefaultPluginINI(const TCHAR *cszPath)
{//cszPath に INI ファイルは存在していないことが前提
    CIniFile iniFile(cszPath);
    TCHAR   drive[_MAX_DRIVE];
    TCHAR   dir[_MAX_DIR];
    TCHAR   ext[_MAX_EXT];
    _tsplitpath_s(cszPath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
    TCHAR   szPluginPath[MAX_PATH];
    _tmakepath_s(szPluginPath,_MAX_PATH, drive, dir, _T(""), _T(""));
    _tcsncat_s(szPluginPath, _T("Plugin\\*.*"), _TRUNCATE);
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(szPluginPath, &fd);
    if(hFind == INVALID_HANDLE_VALUE){
        return ;
    }
    int nCount = 0;
    do{
        if(_tcscmp(fd.cFileName, _T(".")) == 0 ||
           _tcscmp(fd.cFileName, _T("..")) == 0){
            continue;
        }
        _tsplitpath_s(fd.cFileName, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT);
        if(_tcsicmp(ext, _T(".ste")) != 0){//プラグインではない
            continue;
        }
        TCHAR szSection[MAX_PATH];
        TCHAR szPath[MAX_PATH];
        _tcsncpy_s(szPath, _T(".\\Plugin\\"), _TRUNCATE);
        _tcsncat_s(szPath, fd.cFileName, _TRUNCATE);
        _sntprintf_s(szSection, _TRUNCATE, _T("Load%03d"), nCount);
        nCount++;
        iniFile.WriteStr(szSection, _T("Path"), szPath);
        iniFile.WriteInt(szSection, _T("Use"), 1);
    }while(FindNextFile(hFind, &fd));
    FindClose(hFind);
    iniFile.Flush();
}
void STEPluginLoad(HWND hWnd)
{
    STEP_hWnd = hWnd;

    CSuperTagEditorApp    *pApp = (CSuperTagEditorApp *)AfxGetApp();
    CString strINI;
    {
        TCHAR*  szName = pApp->MakeFileName(_T("ini"));
        TCHAR   drive[_MAX_DRIVE];
        TCHAR   dir[_MAX_DIR];
        TCHAR   buff[_MAX_PATH] = {0};

        _tsplitpath_s(szName, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
        _tmakepath_s(buff,_MAX_PATH, drive, dir, _T("Plugin"), _T("ini"));
        strINI = buff;
        BOOL isExists = FALSE;
        DWORD dwCode = GetFileAttributes(strINI);
        if (dwCode != 0xffffffff) {
            isExists = TRUE;
        }
        if (!isExists) {
            _tmakepath_s(buff,_MAX_PATH, drive, dir, _T("DefaultPlugin"), _T("ini"));
            strINI = buff;
            dwCode = GetFileAttributes(strINI);
            if (dwCode == 0xffffffff) {
                CreateDefaultPluginINI(strINI);
            }
        }
        free(szName);
    }

    CString strSection;
    CIniFile IniFile(strINI);
    int i;
    for ( i=0;;i++) {
        TCHAR   buff[_MAX_PATH] = {0};
        strSection.Format(_T("Load%03d"), i);
        IniFile.ReadStr(strSection, _T("Path"), _T(""), buff, _countof(buff));
        if (_tcslen(buff) == 0) {
            break;
        }
        PSTEPlugin pPlugin = STEPluginLoadFile(buff);
        if (pPlugin == NULL) {
            //pPlugin->bUse = false;
            MessageBox(hWnd, CString(_T("プラグイン(")) + buff + _T(")の読み込みに失敗しました"), _T("プラグインエラー"), MB_ICONSTOP|MB_OK|MB_TOPMOST);
            continue;
        }
        pPlugin->bUse = IniFile.ReadInt(strSection, _T("Use"), 1) ? true : false;
    }
    STEP_wndToolBar->UpdatePluginButton();
}

bool OnToolTipNotify(UINT nID, LPTSTR& szText) {
    UINT nNextID = _APS_NEXT_COMMAND_VALUE;
    if (nID < nNextID) return false;
    int i; for ( i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin pPlugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!pPlugin->bUse) continue;
        if (pPlugin->STEPGetToolTipText != NULL) {
            szText = (LPTSTR)((*pPlugin->STEPGetToolTipText)(nID));
            if (szText != NULL) {
                return true;
            }
        }
    }
    return false;
}
bool GetMessageString(UINT nID, CString& strMessage) {
    UINT nNextID = _APS_NEXT_COMMAND_VALUE;
    if (nID < nNextID) return false;
    int i; for ( i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin pPlugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!pPlugin->bUse) continue;
        if (pPlugin->STEPGetStatusMessage != NULL) {
            LPCTSTR szText = (*pPlugin->STEPGetStatusMessage)(nID);
            strMessage = _T("[");
            strMessage += pPlugin->sPluginName;
            strMessage += _T("] ");
            if (szText != NULL) {
                strMessage += szText;
                return true;
            }
        }
    }
    return false;
}


UINT GetFileTypeCount() {
    UINT nCount = 0;
    int i; for ( i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        nCount += plugin->arExtInfo.GetSize();
    }
    return nCount;
}


BOOL OnUpdatePluginCommand(UINT nID) {
    int i; for ( i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!plugin->bUse) continue;
        if (plugin->nOptionCommandID == nID) {
            return true;
        }
        if (plugin->STEPOnUpdateCommand != NULL) {
            if (plugin->STEPOnUpdateCommand(nID)) {
                return true;
            }
        }
    }
    return FALSE;
}

void OnPluginCommand(UINT nID) {
    int i; for ( i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!plugin->bUse) continue;
        /* DlgPluginSetup に移動
        if (plugin->nOptionCommandID == nID) {
            plugin->STEPShowOptionDialog(STEP_hWnd);
            return;
        }
        */
        if (plugin->STEPOnCommand != NULL) {
            if (plugin->STEPOnCommand(nID, STEP_hWnd)) {
                return;
            }
        }
    }
    return;
}

void OnLoadMenu(CMenu* pMenu, UINT nType) {
    int i; for ( i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!plugin->bUse) continue;
        if (plugin->STEPOnLoadMenu != NULL) {
            plugin->STEPOnLoadMenu(pMenu->GetSubMenu(0)->m_hMenu, nType);
        }
    }
}

void OnLoadMainMenu() {
    int i; for ( i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!plugin->bUse) continue;
        if (plugin->STEPOnLoadMainMenu != NULL) {
            plugin->STEPOnLoadMainMenu();
        }
    }
}

BOOL IsPluginSupportSIF(const FILE_MP3 *fileMP3) {
    int nIndex = plugins.GetPluginIndex(fileMP3->nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return FALSE;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
        if (!plugin->bUse) return FALSE;
    if (plugin->STEPSupportSIF == NULL) {
        return FALSE;
    }
    return plugin->STEPSupportSIF(fileMP3->nFormat) ? TRUE : FALSE;
}

BOOL IsPluginSupportTrackNumberSIF(const FILE_MP3 *fileMP3) {
    int nIndex = plugins.GetPluginIndex(fileMP3->nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return FALSE;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
        if (!plugin->bUse) return FALSE;
    if (plugin->STEPSupportTrackNumberSIF == NULL) {
        return IsPluginSupportSIF(fileMP3);// FALSE;
    }
    return plugin->STEPSupportTrackNumberSIF(fileMP3->nFormat) ? TRUE : FALSE;
}

BOOL IsPluginSupportGenreSIF(const FILE_MP3 *fileMP3) {
    int nIndex = plugins.GetPluginIndex(fileMP3->nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return FALSE;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
        if (!plugin->bUse) return FALSE;
    if (plugin->STEPSupportGenreSIF == NULL) {
        return IsPluginSupportSIF(fileMP3);// FALSE;
    }
    return plugin->STEPSupportGenreSIF(fileMP3->nFormat) ? TRUE : FALSE;
}

CONTROLTYPE GetControlType(UINT nFormat, COLUMNTYPE nColumn, bool isEditSIF) {
    int nIndex = plugins.GetPluginIndex(nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return invalid;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
        if (!plugin->bUse) return invalid;
    if (plugin->STEPGetControlType == NULL) {
        return invalid;
    }
    return plugin->STEPGetControlType(nFormat, nColumn, isEditSIF);
}

int GetColumnMax(UINT nFormat, COLUMNTYPE nColumn, bool isEditSIF) {
    int nIndex = plugins.GetPluginIndex(nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return 0;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    if (!plugin->bUse) return 0;
    if (plugin->STEPGetColumnMax == NULL) {
        return 0;
    }
    return plugin->STEPGetColumnMax(nFormat, nColumn, isEditSIF);
}

LPCTSTR GetColumnName(UINT nFormat, COLUMNTYPE nColumn) {
    int nIndex = plugins.GetPluginIndex(nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return 0;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    if (!plugin->bUse) return 0;
    if (plugin->STEPGetColumnName == NULL) {
        return 0;
    }
    return plugin->STEPGetColumnName(nFormat, nColumn);
}

bool HasOriginalColumnName(UINT nFormat) {
    int nIndex = plugins.GetPluginIndex(nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return false;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    if (!plugin->bUse) return false;
    if (plugin->STEPGetColumnName == NULL) {
        return false;
    }
    if (plugin->STEPHasSpecificColumnName == NULL) {
        return false;
    }
    return plugin->STEPHasSpecificColumnName(nFormat);
}

UINT GetFormatType(UINT nIndex)
{
    return arFormatType.GetAt(nIndex);
}
UINT GetFormatTypeIndex(UINT nFormat)
{
    int i; for (i=0;i<arFormatType.GetSize();i++) {
        if (arFormatType.GetAt(i) == nFormat) {
            return i;
        }
    }
    return -1;
}

bool LoadFile(const TCHAR *sFileName, const TCHAR *sExt, FILE_MP3* pFileMP3)
{
    UINT result = false;
    FILE_INFO fileInfo;
    fileInfo.pFileMP3 = pFileMP3;
    int i; for (i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!plugin->bUse) continue;
        if (plugin->STEPLoad != NULL) {
            result = plugin->STEPLoad(&fileInfo, sExt);
            if (result == STEP_SUCCESS) {
                break;
            } else if (result == STEP_ERROR) {
                CFileMP3::InitData(pFileMP3);
            }
        }
    }
    if (result != STEP_SUCCESS) {
        return false;
    }
    return true;
}

bool SaveFile(FILE_MP3* pFileMP3)
{
    if (!pFileMP3->bModifyFlag) return true;
    UINT result = STEP_ERROR;
    FILE_INFO fileInfo;
    fileInfo.pFileMP3 = pFileMP3;
    int nIndex = plugins.GetPluginIndex(pFileMP3->nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return false;
    }
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    if (!plugin->bUse) return false;
    if (plugin->STEPSave != NULL) {
        //タグの更新に失敗後、pFileMP3 の中身が変わって場合は更新前に戻す
        //本来はプラグインのバグであり、本体でやるべきことではない
        FILE_MP3 file_mp3_saved = *pFileMP3;//書き換え前の情報を保存(失敗したら更新前に戻す)
        result = plugin->STEPSave(&fileInfo);
        if (result == STEP_SUCCESS) {
            result = CFileMP3::ConvFileName(pFileMP3) == true ? STEP_SUCCESS : STEP_ERROR;
        }
        else{//失敗
            //更新前の状態に戻す
            //ただし、fileSpecificInfo と pTreeItem のみ、関数呼び出し後に
            //書き変わっていたら元に戻さない(中身が不明のため)
            void *fileSpecificInfo = pFileMP3->fileSpecificInfo;//更新後の fileSpecificInfo
            void *pTreeItem = pFileMP3->pTreeItem;              //更新後の pTreeItem
            *pFileMP3 = file_mp3_saved;//更新前に戻す
            if(pFileMP3->fileSpecificInfo != fileSpecificInfo){
                pFileMP3->fileSpecificInfo = fileSpecificInfo;//更新後の fileSpecificInfo を適用
            }
            if(pFileMP3->pTreeItem != pTreeItem){
                pFileMP3->pTreeItem = pTreeItem;//更新後の pTreeItem を適用
            }
        }
    }
    if (result != STEP_SUCCESS) {
        return false;
    }
    pFileMP3->bModifyFlag = false;
    //by Kobarin
    //ファイルを再読み込みして表示更新
    CString strFullPath = pFileMP3->strFullPathName;
    const TCHAR *sExt = _tcsrchr(strFullPath, _T('.'));
    if(sExt){
        LoadFile(strFullPath, sExt+1, pFileMP3);
    }
    STEP_List->UpdateFileStatus((CSuperGridCtrl::CTreeItem*)pFileMP3->pTreeItem, pFileMP3);
    //by Kobarin
    return true;
}

int GetFileImageIndex(UINT nFormat, bool bModify)
{
    int nIndex = plugins.GetPluginIndex(nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return -1;
    }
    PSTEPlugin pPlugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    if (!pPlugin->bUse) return -1;
    int i; for (i=0;i<pPlugin->arExtInfo.GetSize();i++) {
        PSTEPExtInfo info = (PSTEPExtInfo)pPlugin->arExtInfo.GetAt(i);
        if (info->nFormatType == nFormat && info->nImageIndex != -1) {
            return bModify ? (info->nImageIndex+1) : info->nImageIndex;
        }
    }
    return -1;
}

void GetFileExtList(CStringArray& arExt)
{
    int i; for (i=0;i<plugins.arPlugins.GetSize();i++) {
        PSTEPlugin pPlugin = (PSTEPlugin)plugins.arPlugins.GetAt(i);
        if (!pPlugin->bUse) continue;
        int j; for (j=0;j<pPlugin->arExtInfo.GetSize();j++) {
            PSTEPExtInfo info = (PSTEPExtInfo)pPlugin->arExtInfo.GetAt(j);
            for (int k=0;k<arExt.GetSize();k++) {
                if (arExt.GetAt(k) == info->strExt) {
                    info = NULL;
                    break;
                }
            }
            if (info != NULL) {
                arExt.Add(info->strExt);
            }
        }
    }
}

bool CheckFileExt(const FILE_MP3* pFileMP3, const TCHAR* ext)
{
    if (pFileMP3 == NULL) return false;
    int nIndex = plugins.GetPluginIndex(pFileMP3->nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return false;
    }
    PSTEPlugin pPlugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    int j; for (j=0;j<pPlugin->arExtInfo.GetSize();j++) {
        PSTEPExtInfo info = (PSTEPExtInfo)pPlugin->arExtInfo.GetAt(j);
        if (_tcsicmp(ext, info->strExt) == 0) {
            return true;
        }
        if (ext[0] == _T('.') && _tcsicmp(ext+1, info->strExt) == 0) {
            return true;
        }
    }
    return false;
}

void InitFileSpecificInfo(FILE_MP3* pFileMP3)
{
    int nIndex = plugins.GetPluginIndex(pFileMP3->nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return;
    }
    FILE_INFO fileInfo;
    fileInfo.pFileMP3 = pFileMP3;
    PSTEPlugin pPlugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    if (pPlugin->STEPInitFileSpecificInfo != NULL) {
        pPlugin->STEPInitFileSpecificInfo(&fileInfo);
        return;
    }
}

void ConvSIFieldToID3tag(FILE_MP3* pFileMP3) /* STEP 037 */
{

    int nIndex = STEP_List->GetIndexFromFileMP3(pFileMP3);
    ASSERT(nIndex >= 0);
    if (nIndex < 0) return;

    CString     strText;
    bool saveEditField = g_bOptEditFieldSIF;
    int i; for (i=    COLUMN_TRACK_NAME;i<COLUMN_MAX;i++) {
        g_bOptEditFieldSIF = TRUE; // 表示フィールド切替
        // SI/ID3v2フィールドの内容を取得
        CString strTextSI = STEP_List->GetFileColumnText(pFileMP3, i);
        g_bOptEditFieldSIF = FALSE; // 表示フィールド切替
        // ID3tagフィールドへ入力
        STEP_List->ChangeSubItemText(nIndex, g_nColumnNumberList[i], strTextSI, 0, false, "", "", true);
    }
    g_bOptEditFieldSIF = saveEditField; // 表示フィールド戻す
    STEP_List->UpdateFileStatus((CSuperGridCtrl::CTreeItem*)pFileMP3->pTreeItem, pFileMP3);
}

void OnConvSIFieldToID3tag(FILE_MP3* pFileMP3) /* STEP 037 */
{
    int nIndex = plugins.GetPluginIndex(pFileMP3->nFormat);
    if (nIndex < 0) {
        // 対応プラグインなし
        return;
    }
    FILE_INFO fileInfo;
    fileInfo.pFileMP3 = pFileMP3;
    PSTEPlugin pPlugin = (PSTEPlugin)plugins.arPlugins.GetAt(nIndex);
    if (pPlugin->STEPOnConvSiFieldToId3tag != NULL) {
        if (pPlugin->STEPOnConvSiFieldToId3tag(&fileInfo) == false) {
            return; // 未サポート or プラグイン側で実行
        }
    } else {
        return; // 未サポート
    }
    ConvSIFieldToID3tag(pFileMP3);
}

/*----------------------------*/
extern "C" STEP_API UINT WINAPI STEPGetCommandID()
{
    return nCommandID++;
}

extern "C"  STEP_API HMENU WINAPI STEPGetMenu(UINT nType)
{
    CWnd    *pMainWnd = AfxGetMainWnd();
    CMenu   *STEP_hMenu = pMainWnd ? pMainWnd->GetMenu() : NULL;
    CMenu* subMenu;
    switch (nType) {
    case MENU_FILE:
        subMenu = STEP_hMenu->GetSubMenu(0);
        return subMenu->m_hMenu;
    case MENU_EDIT:
        subMenu = STEP_hMenu->GetSubMenu(1);
        return subMenu->m_hMenu;
    case MENU_DISP:
        subMenu = STEP_hMenu->GetSubMenu(2);
        return subMenu->m_hMenu;
    case MENU_CONV:
        subMenu = STEP_hMenu->GetSubMenu(3);
        return subMenu->m_hMenu;
    case MENU_PLAYLIST:
        subMenu = STEP_hMenu->GetSubMenu(4);
        return subMenu->m_hMenu;
    case MENU_PLAYER:
        subMenu = STEP_hMenu->GetSubMenu(5);
        return subMenu->m_hMenu;
    case MENU_OPTION:
        subMenu = STEP_hMenu->GetSubMenu(6);
        return subMenu->m_hMenu;
/*
    case MENU_OPTION_PLUGIN:
        subMenu = STEP_hMenu->GetSubMenu(6);
        subMenu = subMenu->GetSubMenu(13);
        return subMenu->m_hMenu;
*/
    case MENU_HELP:
        subMenu = STEP_hMenu->GetSubMenu(6);
        return subMenu->m_hMenu;
    }
    return NULL;
}

extern "C"  STEP_API void WINAPI STEPAddToolBarButton(HBITMAP hBitmap, UINT nCommandID, TCHAR* lpszRegName)
{
    plugins.arHandle.Add(hBitmap);
    CBitmap* pBitmap = CBitmap::FromHandle(hBitmap);
    CToolBarCtrl* pToolbarCtrl = &STEP_wndToolBar->GetToolBarCtrl();
    pToolbarCtrl->AddBitmap(1, pBitmap);
    int nButCount = pToolbarCtrl->GetButtonCount();
    int nImageCount = pToolbarCtrl->GetImageList()->GetImageCount();
    TBBUTTON tb;
    tb.iBitmap = nImageCount-1;
    tb.idCommand = nCommandID;
    tb.fsState = TBSTATE_ENABLED;
    tb.fsStyle = TBSTYLE_BUTTON;
    tb.dwData = 0;
    tb.iString = NULL;
    STEP_wndToolBar->InsertButton(nButCount, &tb, lpszRegName);
}

extern "C" STEP_API void WINAPI STEPKeyAssign(UINT nCommandID, TCHAR* lpszName, TCHAR* lpszRegName)
{
    KEY_CONFIG* key = new KEY_CONFIG;
    key->wCmdID = nCommandID;
    key->dwKeyCode = 0;
    key->nGroupID = COMMAND_GROUP_PLUGIN;
    key->sName = lpszName;
    key->sRegName = lpszRegName;
    plugins.arPluginKey.Add(key);
}

extern "C" STEP_API UINT WINAPI STEPRegisterExt(UINT nID, LPCTSTR szExt, HBITMAP hBitmap)
{
    PSTEPlugin plugin = (PSTEPlugin)plugins.arPlugins.GetAt(nID-1);
    UINT nFormatType = static_cast<long long>(nID) << 8 | (plugin->arExtInfo.GetSize());
    arFormatType.Add(nFormatType);

    PSTEPExtInfo info = new STEPExtInfo;
    info->strExt = szExt;
    CBitmap* pBitmap = CBitmap::FromHandle(hBitmap);
    if (hBitmap == NULL) {
        info->nImageIndex = -1;
    } else {
        info->nImageIndex = STEP_ImageList->Add(pBitmap, RGB(255, 0, 255));
        pBitmap->Detach();
    }
    info->nFormatType = nFormatType;
    plugin->arExtInfo.Add(info);
    return nFormatType;
}

extern "C" STEP_API const TCHAR* WINAPI STEPGetGenreNameSIF(BYTE byGenre)
{
    return GetGenreNameSIF(byGenre);
}

extern "C" STEP_API BYTE WINAPI STEPGetGenreCode(const TCHAR* szGenre)
{
    return GetGenreCode(szGenre);
}

extern "C" STEP_API bool WINAPI STEPIsUserGenre(const TCHAR* szGenre)
{
    return IsUserGenre(szGenre);
}

extern "C" STEP_API int WINAPI STEPGetNumericTrackNumber(const TCHAR* szTrackNumber, TCHAR* szNumericNumber, int size) /* STEP 037 */
{
    CString strTrackNumber = szTrackNumber;
    CFileMP3::GetIntTrackNo(strTrackNumber);
    _tcsncpy_s(szNumericNumber, size, strTrackNumber, _TRUNCATE);
    return strTrackNumber.GetLength();
}

extern "C" STEP_API int WINAPI STEPGetIntegerTrackNumber(const TCHAR* szTrackNumber) /* STEP 037 */
{
    return _ttoi(CFileMP3::GetIntTrackNo(szTrackNumber));
}

extern "C" STEP_API int WINAPI STEPGetNumericDiscNumber(const TCHAR* szDiscNumber, TCHAR* szNumericNumber, int size) /* STEP 037 */
{
    CString strDiscNumber = szDiscNumber;
    CFileMP3::GetIntDiscNo(strDiscNumber);
    _tcsncpy_s(szNumericNumber, size, strDiscNumber, _TRUNCATE);
    return strDiscNumber.GetLength();
}

extern "C" STEP_API int WINAPI STEPGetIntegerDiscNumber(const TCHAR* szDiscNumber) /* STEP 037 */
{
    return _ttoi(CFileMP3::GetIntDiscNo(szDiscNumber));
}

extern "C" STEP_API bool WINAPI STEPIsNumeric(const TCHAR* szText) /* STEP 037 */
{
    return CFileMP3::IsNumeric(szText);
}

extern "C" STEP_API void WINAPI STEPConvSiFieldToId3tag(FILE_INFO* pFileInfo) /* STEP 037 */
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;

    ConvSIFieldToID3tag(pFileMP3);
}

extern "C" STEP_API LPCTSTR WINAPI STEPGetValue(FILE_INFO* pFileInfo, FIELDTYPE nField)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case FIELD_FULL_PATH_NAME:          // ファイル名(フルパス)
        return pFileMP3->strFullPathName;
    case FIELD_FILE_NAME:               // strFullPathName のファイル名部分
        return pFileMP3->strFileName;
    case FIELD_FILE_PATH:               // strFullPathName のパス部分
        return pFileMP3->strFilePath;
    case FIELD_VISUAL_FORMAT:           // 映像フォーマット
        return pFileMP3->strVisualFormat;
    case FIELD_AUDIO_FORMAT:            // 音声フォーマット
        return pFileMP3->strAudioFormat;
    case FIELD_TRACK_NAME:              // トラック名
        return pFileMP3->strTrackName;
    case FIELD_ARTIST_NAME:             // アーティスト名
        return pFileMP3->strArtistName;
    case FIELD_ALBUM_NAME:              // アルバム名
        return pFileMP3->strAlbumName;
    case FIELD_COMMENT:                 // コメント
        return pFileMP3->strComment;
    case FIELD_YEAR:                    // リリース年号
        return pFileMP3->strYear;
    case FIELD_TRACK_NUMBER:            // トラック番号
        return pFileMP3->strTrackNumber;
    case FIELD_GENRE:                   // ジャンル名
        return pFileMP3->strGenre;
    case FIELD_TRACK_NAME_SI:           // 曲名
        return pFileMP3->strTrackNameSI;
    case FIELD_ARTIST_NAME_SI:          // アーティスト名
        return pFileMP3->strArtistNameSI;
    case FIELD_ALBUM_NAME_SI:           // 製品名
        return pFileMP3->strAlbumNameSI;
    case FIELD_COMMENT_SI:              // コメント文字列
        return pFileMP3->strCommentSI;
    case FIELD_YEAR_SI:                 // リリース年号
        return pFileMP3->strYearSI;
    case FIELD_TRACK_NUMBER_SI:         // トラック番号
        return pFileMP3->strTrackNumberSI;
    case FIELD_Disc_NUMBER_SI:          // トラック番号
        return pFileMP3->strDiscNumberSI;
    case FIELD_TRACK_TOTAL_SI:          // トラック数(by Kobarin)
        return pFileMP3->strTrackTotalSI;
    case FIELD_DISC_TOTAL_SI:           // ディスク数(by Kobarin)
        return pFileMP3->strDiscTotalSI;
    case FIELD_GENRE_SI:                // ジャンル名
        return pFileMP3->strGenreSI;
    case FIELD_COPYRIGHT_SI:            // 著作権
        return pFileMP3->strCopyrightSI;
    case FIELD_ENGINEER_SI:             // エンジニア
        return pFileMP3->strEngineerSI;
    case FIELD_SOURCE_SI:               // ソース
        return pFileMP3->strSourceSI;
    case FIELD_SOFTWARE_SI:             // ソフトウェア
        return pFileMP3->strSoftwareSI;
    case FIELD_KEYWORD_SI:              // キーワード
        return pFileMP3->strKeywordSI;
    case FIELD_TECHNICIAN_SI:           // 技術者
        return pFileMP3->strTechnicianSI;
    case FIELD_LYRIC_SI:                // 歌詞
        return pFileMP3->strLyricSI;
    case FIELD_COMMISSION_SI:           // コミッション
        return pFileMP3->strCommissionSI;
    case FIELD_WRITER_SI:               // 作詞
        return pFileMP3->strWriterSI;
    case FIELD_COMPOSER_SI:             // 作曲
        return pFileMP3->strComposerSI;
    case FIELD_ALBM_ARTIST_SI:          // albm.アーティスト
        return pFileMP3->strAlbmArtistSI;
    case FIELD_ORIG_ARTIST_SI:          // Orig.アーティスト
        return pFileMP3->strOrigArtistSI;
    case FIELD_URL_SI:                  // URL
        return pFileMP3->strURLSI;
    case FIELD_ENCODEST:                // エンコードした人
        return pFileMP3->strEncodest;
    case FIELD_OTHER:                   // その他
        return pFileMP3->strOther;
    case FIELD_ALBUM_SORT:              // アルバム読み
        return pFileMP3->strAlbumSort;
    case FIELD_ALBM_ARTIST_SORT:        // Albm.アーティスト読み
        return pFileMP3->strAlbumArtistSort;
    case FILED_FILE_TYPE_NAME:          // ファイルタイプ文字列
        return pFileMP3->strFileTypeName;
    }
    return _T("");
}

extern "C" STEP_API void WINAPI STEPSetValue(FILE_INFO* pFileInfo, FIELDTYPE nField, LPCTSTR szValue)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case FIELD_FULL_PATH_NAME:          // ファイル名(フルパス)
        pFileMP3->strFullPathName = szValue;
        break;
    case FIELD_FILE_NAME:               // strFullPathName のファイル名部分
        pFileMP3->strFileName = szValue;
        break;
    case FIELD_FILE_PATH:               // strFullPathName のパス部分
        pFileMP3->strFilePath = szValue;
        break;
    case FIELD_VISUAL_FORMAT:           // 映像フォーマット
        pFileMP3->strVisualFormat = szValue;
        break;
    case FIELD_AUDIO_FORMAT:            // 音声フォーマット
        pFileMP3->strAudioFormat = szValue;
        break;
    case FIELD_TRACK_NAME:              // トラック名
        pFileMP3->strTrackName = szValue;
        break;
    case FIELD_ARTIST_NAME:             // アーティスト名
        pFileMP3->strArtistName = szValue;
        break;
    case FIELD_ALBUM_NAME:              // アルバム名
        pFileMP3->strAlbumName = szValue;
        break;
    case FIELD_COMMENT:                 // コメント
        pFileMP3->strComment = szValue;
        break;
    case FIELD_YEAR:                    // リリース年号
        pFileMP3->strYear = szValue;
        break;
    case FIELD_TRACK_NUMBER:            // トラック番号
        pFileMP3->strTrackNumber = szValue;
        if (_ttoi(szValue) == 0 || _ttoi(szValue) > 0x0ff) {
            pFileMP3->byTrackNumber = 0x0ff;
        } else {
            pFileMP3->byTrackNumber = _ttoi(szValue);
        }
        break;
    case FIELD_GENRE:                   // ジャンル名
        pFileMP3->strGenre = szValue;
        break;
    case FIELD_TRACK_NAME_SI:           // 曲名
        pFileMP3->strTrackNameSI = szValue;
        break;
    case FIELD_ARTIST_NAME_SI:          // アーティスト名
        pFileMP3->strArtistNameSI = szValue;
        break;
    case FIELD_ALBUM_NAME_SI:           // 製品名
        pFileMP3->strAlbumNameSI = szValue;
        break;
    case FIELD_COMMENT_SI:              // コメント文字列
        pFileMP3->strCommentSI = szValue;
        break;
    case FIELD_YEAR_SI:                 // リリース年号
        pFileMP3->strYearSI = szValue;
        break;
    case FIELD_TRACK_NUMBER_SI:         // トラック番号
        pFileMP3->strTrackNumberSI = szValue;
        break;
    case FIELD_TRACK_TOTAL_SI:          // トラック数(by Kobarin)
        pFileMP3->strTrackTotalSI = szValue;
        break;
    case FIELD_Disc_NUMBER_SI:          // ディスク番号
        pFileMP3->strDiscNumberSI = szValue;
        break;
    case FIELD_DISC_TOTAL_SI:           // ディスク数(by Kobarin)
        pFileMP3->strDiscTotalSI = szValue;
        break;
    case FIELD_GENRE_SI:                // ジャンル名
        pFileMP3->strGenreSI = szValue;
        break;
    case FIELD_COPYRIGHT_SI:            // 著作権
        pFileMP3->strCopyrightSI = szValue;
        break;
    case FIELD_ENGINEER_SI:             // エンジニア
        pFileMP3->strEngineerSI = szValue;
        break;
    case FIELD_SOURCE_SI:               // ソース
        pFileMP3->strSourceSI = szValue;
        break;
    case FIELD_SOFTWARE_SI:             // ソフトウェア
        pFileMP3->strSoftwareSI = szValue;
        break;
    case FIELD_KEYWORD_SI:              // キーワード
        pFileMP3->strKeywordSI = szValue;
        break;
    case FIELD_TECHNICIAN_SI:           // 技術者
        pFileMP3->strTechnicianSI = szValue;
        break;
    case FIELD_LYRIC_SI:                // 歌詞
        pFileMP3->strLyricSI = szValue;
        break;
    case FIELD_COMMISSION_SI:           // コミッション
        pFileMP3->strCommissionSI = szValue;
        break;
    case FIELD_WRITER_SI:
        pFileMP3->strWriterSI = szValue;
        break;
    case FIELD_COMPOSER_SI:             // 作曲
        pFileMP3->strComposerSI = szValue;
        break;
    case FIELD_ALBM_ARTIST_SI:          // Albm.アーティスト
        pFileMP3->strAlbmArtistSI = szValue;
        break;
    case FIELD_ORIG_ARTIST_SI:          // Orig.アーティスト
        pFileMP3->strOrigArtistSI = szValue;
        break;
    case FIELD_URL_SI:                  // URL
        pFileMP3->strURLSI = szValue;
        break;
    case FIELD_ENCODEST:                // エンコードした人
        pFileMP3->strEncodest = szValue;
        break;
    case FIELD_OTHER:                   // その他
        pFileMP3->strOther = szValue;
        break;
    case FIELD_ALBUM_SORT:              // アルバム読み
        pFileMP3->strAlbumSort = szValue;
        break;
    case FIELD_ALBM_ARTIST_SORT:        // Albm.アーティスト読み
        pFileMP3->strAlbumArtistSort = szValue;
        break;
    case FILED_FILE_TYPE_NAME:          // ファイルタイプ文字列
        pFileMP3->strFileTypeName = szValue;
        break;
    }
}

extern "C" STEP_API bool WINAPI STEPGetBooleanValue(FILE_INFO* pFileInfo, UINT nField)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // 外部から編集されたかどうかのフラグ
        return pFileMP3->bModifyFlag;
    case 1:            // チェック状態
        return pFileMP3->bCheckFlag;
    case 2:            // 歌詞ファイルの存在
        return pFileMP3->bExistLyricFile;
    }
    return false;
}

extern "C" STEP_API void WINAPI STEPSetBooleanValue(FILE_INFO* pFileInfo, UINT nField, bool bFlag)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // 外部から編集されたかどうかのフラグ
        pFileMP3->bModifyFlag = bFlag;
        break;
    case 1:            // チェック状態
        pFileMP3->bCheckFlag = bFlag;
        break;
    case 2:            // 歌詞ファイルの存在
        pFileMP3->bExistLyricFile = bFlag;
        break;
    }
}

extern "C" STEP_API long WINAPI STEPGetLongValue(FILE_INFO* pFileInfo, UINT nField)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // ファイルサイズ
        return pFileMP3->lFileSize;
    }
    return 0;
}

extern "C" STEP_API void WINAPI STEPSetLongValue(FILE_INFO* pFileInfo, UINT nField, long lValue)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // ファイルサイズ
        pFileMP3->lFileSize = lValue;
        break;
    }
}

extern "C" STEP_API int WINAPI STEPGetIntValue(FILE_INFO* pFileInfo, UINT nField)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // ファイルフォーマット
        return pFileMP3->nFormat;
    case 1:            // 演奏時間(単位：秒)
        return pFileMP3->nPlayTime;
    case 2:            // トラック番号(数値編集用)
        return pFileMP3->byTrackNumber;
    case 3:            // ジャンル番号
        return pFileMP3->byGenre;
    }
    return 0;
}

extern "C" STEP_API void WINAPI STEPSetIntValue(FILE_INFO* pFileInfo, UINT nField, int nValue)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // ファイルフォーマット
        pFileMP3->nFormat = nValue;
        break;
    case 1:            // 演奏時間(単位：秒)
        pFileMP3->nPlayTime = nValue;
        break;
    case 2:            // トラック番号(数値編集用)
        pFileMP3->byTrackNumber = nValue;
        if (pFileMP3->byTrackNumber != 0 && pFileMP3->byTrackNumber < 0xff) {
            CString strTrackNumber;
            strTrackNumber.Format(_T("%d"), pFileMP3->byTrackNumber);
            STEPSetValue(pFileInfo, FIELD_TRACK_NUMBER, strTrackNumber);
        } else {
            STEPSetValue(pFileInfo, FIELD_TRACK_NUMBER, _T(""));
        }
        break;
    case 3:            // ジャンル番号
        pFileMP3->byGenre = nValue;
        break;
    }
}

extern "C" STEP_API void* WINAPI STEPGetVoidValue(FILE_INFO* pFileInfo, UINT nField)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // ファイル固有情報
        return pFileMP3->fileSpecificInfo;
    }
    return NULL;
}

extern "C" STEP_API void WINAPI STEPSetVoidValue(FILE_INFO* pFileInfo, UINT nField, void* pInfo)
{
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    switch (nField) {
    case 0:            // ファイル固有情報
        pFileMP3->fileSpecificInfo = pInfo;
        break;
    }
}

extern "C" STEP_API void WINAPI STEPProcessSelectedFiles(LPCTSTR szTitle, STEPProcessSelectedFilesCB* callback)
{
    STEP_List->ProcessSelectedFiles(szTitle, callback);
}

extern "C" STEP_API void WINAPI STEPProcessSelectedFilesForUpdate(LPCTSTR szTitle, STEPProcessSelectedFilesCB* callback)
{
    STEP_List->ProcessSelectedFilesForUpdate(szTitle, callback);
}

extern "C" STEP_API bool WINAPI STEPIsRangeSelected(void)
{
    return STEP_List->IsRangeSelected();
}

extern "C" STEP_API int WINAPI STEPGetSelectedCount(void)
{
    return STEP_List->GetSelectedCount();
}

extern "C" STEP_API int WINAPI STEPGetSelectedItem(void)
{
    return STEP_List->GetSelectedItem();
}

extern "C" STEP_API bool WINAPI STEPIsCurrentCellEditOK(void)
{
    return STEP_List->IsCurrentCellEditOK();
}

extern "C" STEP_API bool WINAPI STEPItemHasChildren(int nIndex)
{
    return STEP_List->ItemHasChildren(STEP_List->GetTreeItem(nIndex/*STEPGetSelectedItem()*/)) ? true : false;
}

extern "C" STEP_API bool WINAPI STEPGetSelectedRange(int* sx, int* sy, int* ex, int* ey)
{
    return STEP_List->GetSelectedRange(*sx, *sy, *ex, *ey);
}

extern "C" STEP_API bool WINAPI STEPIsItemFile(int nItem)
{
    CSuperGridCtrl::CTreeItem    *pItem = STEP_List->GetTreeItem(nItem);
    if (pItem != NULL && STEP_List->IsItemFile(pItem) == TRUE) {
        return true;
    }
    return false;
}

extern "C" STEP_API void WINAPI STEPChangeSubItemText(int nItem, int nColumn, LPCTSTR szText)
{
    STEP_List->ChangeSubItemText(nItem, nColumn, szText);
    STEP_List->InvalidateItemRect(nItem);
}

extern "C" STEP_API const TCHAR* WINAPI STEPGetSubItemText(int nItem, int nColumn)
{
    CSuperGridCtrl::CTreeItem    *pItem = STEP_List->GetTreeItem(nItem);
    CItemInfo    *pItemInfo = STEP_List->GetData(pItem);
    return pItemInfo->GetSubItem(nColumn-1);
}

extern "C" STEP_API void WINAPI STEPGetFileInfo(int nItem, FILE_INFO* info)
{
    CSuperGridCtrl::CTreeItem    *pItem = STEP_List->GetTreeItem(nItem);
    CItemInfo    *pItemInfo = STEP_List->GetData(pItem);
    FILE_MP3    *fileMP3 = STEP_List->m_pDoc->GetListMP3((int)pItemInfo->GetLParam());
    info->pFileMP3 = fileMP3;
}



extern "C" STEP_API void WINAPI STEPInitDataSIF(FILE_INFO* pFileInfo) {
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    CFileMP3::InitDataSIF(pFileMP3);
}

extern "C" STEP_API void WINAPI STEPInitDataID3(FILE_INFO* pFileInfo) {
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    CFileMP3::InitDataID3(pFileMP3);
}

extern int g_nColumnTypeList[];
extern "C" STEP_API UINT WINAPI STEPGETColumnType(int nColumn)
{
    return g_nColumnTypeList[nColumn];
}


extern "C" STEP_API void WINAPI STEPInitData(FILE_INFO* pFileInfo) {
    FILE_MP3* pFileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    CFileMP3::InitData(pFileMP3);
}

extern "C" STEP_API bool WINAPI STEPFileNameChange(FILE_INFO* pFileInfo, LPCTSTR szNewFileName) {
    FILE_MP3* fileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;

    TCHAR    fname[_MAX_FNAME];
    CString     strFileNameOld = fileMP3->strFileName;
    fileMP3->strFileName = szNewFileName;
    _tsplitpath_s(fileMP3->strFileName, NULL, 0, NULL, 0, fname, _MAX_FNAME, NULL, 0);

    // ファイル名変更
    CFileStatus    fileStatus;
    bool    bKeepTimeStamp = g_bOptKeepTimeStamp;
    // タイムスタンプを取得
    fileStatus.m_mtime = -1;
    try{

    if (bKeepTimeStamp
    &&  CFile::GetStatus(fileMP3->strFullPathName, fileStatus) == FALSE) {
        bKeepTimeStamp = false;
    }

    }
    catch(...){
        bKeepTimeStamp = false;
    }
    if (CFileMP3::ConvFileName(fileMP3) == false) {
        // 失敗した場合は、ファイル名を元に戻す
        fileMP3->bModifyFlag = TRUE;
        fileMP3->strFileName = strFileNameOld;
        return false;
    } else {
        if (bKeepTimeStamp) {
            // ファイル更新時間を設定
            fileStatus.m_mtime = fileMP3->time;
            if (g_bOptSyncCreateTime) fileStatus.m_ctime = fileMP3->time;
            try{
            CFile::SetStatus(fileMP3->strFullPathName, fileStatus);
            }catch(...){}
        } else {
            if (fileStatus.m_mtime.GetTime() != -1) {
                fileMP3->time = fileStatus.m_mtime;
            }
        }
    }
    return true;
}

extern "C" STEP_API bool WINAPI STEPWriteTag(FILE_INFO* pFileInfo) {
    FILE_MP3* fileMP3 = (FILE_MP3*)pFileInfo->pFileMP3;
    return SaveFile(fileMP3);
}

extern "C" STEP_API void WINAPI STEPUpdateCellInfo(void) {
    STEP_List->UpdateCellInfo();
    STEP_List->UpdateAllFiles();
}

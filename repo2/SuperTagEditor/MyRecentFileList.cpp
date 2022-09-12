// MyRecentFileList.cpp: CMyRecentFileList クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "supertageditor.h"
#include "MyRecentFileList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

void CMyRecentFileList::UpdateMenu(CCmdUI* pCmdUI) /* StartInaction 053 */
{
    int iMRU;
    ASSERT(m_arrNames != NULL);

    CMenu* pMenu = pCmdUI->m_pMenu;

    if (pMenu == NULL) return;

    if (m_strOriginal.IsEmpty() && pMenu != NULL) {
        pMenu->GetMenuString(pCmdUI->m_nID, m_strOriginal, MF_BYCOMMAND);
    }

    if (m_arrNames[0].IsEmpty()) {
        if (!m_strOriginal.IsEmpty())
                pCmdUI->SetText(m_strOriginal);
        pCmdUI->Enable(FALSE);
        return;
    }

    ASSERT(pMenu == pCmdUI->m_pMenu);  // make sure preceding code didn't mess with it
    ASSERT(pMenu->m_hMenu);

    // look for a submenu to use instead
    CMenu *pSubMenu = NULL;
    if (pMenu) pSubMenu = pMenu->GetSubMenu(pCmdUI->m_nIndex);
    if (pSubMenu) {
        ASSERT(pSubMenu->m_hMenu);
        pMenu = pSubMenu;
    }

    ASSERT(pMenu->m_hMenu);

    for (iMRU = 0; iMRU < m_nSize; iMRU++) {
        pMenu->RemoveMenu(pCmdUI->m_nID + iMRU, MF_BYCOMMAND);
    }


    TCHAR szCurDir[_MAX_PATH+1];
    GetCurrentDirectory(_MAX_PATH, szCurDir);
    int nCurDir = _tcslen(szCurDir);
    ASSERT(nCurDir >= 0);
    szCurDir[nCurDir] = _T('\\');
    szCurDir[++nCurDir] = 0;

    CString strName;
    CString strTemp;
    for (iMRU = 0; iMRU < m_nSize; iMRU++) {
        if (!GetDisplayName(strName, iMRU, szCurDir, nCurDir)) break;
        // double up any '&' characters so they are not underlined
        LPCTSTR lpszSrc = strName;

        LPTSTR lpszDest = strTemp.GetBuffer(strName.GetLength()*2);

        while (*lpszSrc != 0) {
            if (*lpszSrc == _T('&')) *lpszDest++ = _T('&');

            if (_istlead(*lpszSrc)) *lpszDest++ = *lpszSrc++;

            *lpszDest++ = *lpszSrc++;
        }
        *lpszDest = 0;

        strTemp.ReleaseBuffer();

        // insert mnemonic + the file name
        TCHAR buf[10];
        _sntprintf_s(buf, _TRUNCATE, _T("&%d "), (iMRU+1+m_nStart) % 10);

        // Note we use our pMenu which may not be pCmdUI->m_pMenu
        pMenu->InsertMenu(pCmdUI->m_nIndex++,
                            MF_STRING | MF_BYPOSITION, pCmdUI->m_nID++,
                            CString(buf) + strTemp);
    }

    // update end menu count
    pCmdUI->m_nIndex--; // point to last menu added
    pCmdUI->m_nIndexMax = pMenu->GetMenuItemCount();

    pCmdUI->m_bEnableChanged = TRUE;    // all the added items are enabled
}

void CMyRecentFileList::Add(LPCTSTR lpszPathName)
{
    // ディレクトリ以外の場合は無視する /* BeachMonster 095 */
    DWORD dwAttr = GetFileAttributes(lpszPathName);
    if (dwAttr == 0xFFFFFFFF || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        return;
    }
    CRecentFileList::Add(lpszPathName);
}

void CMyRecentFileList::ReadList()
{// reads from registry or ini file
 //by Kobarin
 //CSuperTagEditorApp::m_IniFile と同じファイルに対して別のルーチンで読み込んでも
 //CSuperTagEditorApp::m_IniFile 側には反映されないため、必ず CSuperTagEditorApp
 //経由で読み書きを行う
    ASSERT(m_arrNames != NULL);
    ASSERT(!m_strSectionName.IsEmpty());
    ASSERT(!m_strEntryFormat.IsEmpty());
    int nLen = m_strEntryFormat.GetLength() + 10;
    LPTSTR pszEntry = new TCHAR[nLen];
    TCHAR buf[2048];
    CIniFile *pIniFile = ((CSuperTagEditorApp*)AfxGetApp())->GetIniFile();
    for (int iMRU = 0; iMRU < m_nSize; iMRU++)
    {
        _stprintf_s(pszEntry, nLen, m_strEntryFormat, iMRU + 1);
        m_arrNames[iMRU] = pIniFile->ReadStr(m_strSectionName,
                                             pszEntry, _T(""),
                                             buf, _countof(buf));
    }
    delete[] pszEntry;
}
void CMyRecentFileList::WriteList()
{// writes to registry or ini file
 //by Kobarin
 //CSuperTagEditorApp::m_IniFile と同じファイルに対して別のルーチンで読み込んでも
 //CSuperTagEditorApp::m_IniFile 側には反映されないため、必ず CSuperTagEditorApp
 //経由で読み書きを行う
    ASSERT(m_arrNames != NULL);
    ASSERT(!m_strSectionName.IsEmpty());
    ASSERT(!m_strEntryFormat.IsEmpty());
    int nLen = m_strEntryFormat.GetLength() + 10;
    LPTSTR pszEntry = new TCHAR[nLen];
    CIniFile *pIniFile = ((CSuperTagEditorApp*)AfxGetApp())->GetIniFile();
    for (int iMRU = 0; iMRU < m_nSize; iMRU++)
    {
        _stprintf_s(pszEntry, nLen, m_strEntryFormat, iMRU + 1);
        if (!m_arrNames[iMRU].IsEmpty())
        {
            pIniFile->WriteStr(m_strSectionName, pszEntry,
                               m_arrNames[iMRU]);
        }
    }
    pIniFile->Flush();
    delete[] pszEntry;
}

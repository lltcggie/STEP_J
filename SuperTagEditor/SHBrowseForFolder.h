// ======================================================
// 概  要    ： フォルダ選択ダイアログのラッパクラス
// ターゲット： Windows95/98/NT
// 処理系    ： Visual C++ Ver 6.0
// 作成者    ： MERCURY
// 作成日    ： 00/06/17(土)
// 著作権表記： Copyright(C) 2000 MERCURY.
// ======================================================

#ifndef __SH_BROWSE_FOR_FOLDER_H__
#define __SH_BROWSE_FOR_FOLDER_H__


// ======================================
// =====   条件コンパイルフラグ     =====
// ======================================



// ======================================
// =====   インクルードファイル     =====
// ======================================



// ======================================
// =====           定  数           =====
// ======================================



// ======================================
// =====           マクロ           =====
// ======================================



// ======================================
// =====       typedef／enum        =====
// ======================================



// ======================================
// =====       構造体／共用体       =====
// ======================================



// ======================================
// =====         const 変数         =====
// ======================================



// ======================================
// =====        extern 変数         =====
// ======================================



// ======================================
// =====        static 変数         =====
// ======================================



// ======================================
// =====     関数プロトタイプ       =====
// ======================================



// ======================================
// =====      プログラム領域        =====
// ======================================


#ifdef __cplusplus

class   CSHBrowseForFolder  {
public:
    CSHBrowseForFolder(bool = false, bool = true);
    virtual ~CSHBrowseForFolder();

public:
    void  SetEnableSubDirButton(bool bFlag) {m_bEnableSubDirButton = bFlag;}
    bool  GetEnableSubDirButton(void) {return(m_bEnableSubDirButton);}
    void  SetSearchSubDirState(bool bFlag) {m_bSearchSubDirState = bFlag;}
    bool  GetSearchSubDirState(void) {return(GetEnableSubDirButton() ? m_bSearchSubDirState : false);}
    bool  Exec(TCHAR *sLocal, int size);
    void  SetCheckBoxTitle(const TCHAR* title) {_tcsncpy_s(m_szCheckBoxTitle, title, _TRUNCATE);}
    const TCHAR* GetCheckBoxTitle(void) {return m_szCheckBoxTitle;}

private:
    TCHAR    m_szInitialFolder[MAX_PATH];
    TCHAR    m_szCheckBoxTitle[256];
    CButton *m_pSubDir;
    bool     m_bEnableSubDirButton;
    bool     m_bSearchSubDirState;
    WNDPROC  m_VSSelectOrgProc;

    static  LRESULT CALLBACK VSSelectFolderSubProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT VSSelectFolderSubProcInternal(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static  int CALLBACK CallbackSelectDir(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
    int CallbackSelectDirInternal(HWND hWnd, UINT uMsg, LPARAM lParam);

protected:  // ======================================
            // =====    protected メンバ関数    =====
            // ======================================
};

#endif

#endif

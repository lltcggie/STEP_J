#include "stdafx.h"
#include "player_control.h"
#include "winamp.h"
#include "wa_ipc.h"
#include "scmpx.h"
#include "kbdde.h"
#include "SuperTagEditor.h"
#include "strcnv.h"
//Winamp
static const TCHAR g_cszClassNameWinamp[] = _T("Winamp v1.x");
//SCMPX
static const TCHAR g_cszClassNameSCMPX[] = _T("SCMPX");
//foobar2000
static const TCHAR g_cszClassNameFoobar[] = _T("{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}");//バージョンによって変わる可能性がある？
//Lilith/uLilith
static const TCHAR g_cszClassNameOldLilith[] = _T("Lilith");
static const TCHAR g_cszClassNameNewLilith[] = _T("uLilith");
static const TCHAR g_cszServiceNameOldLilith[] = _T("LILITH_EXT_CONTROL");//旧SoundPlayer Lilith(UNICODE 非対応版)
static const TCHAR g_cszTopicNameOldLilith[] =   _T("LILITH_EXT_CONTROL");//旧SoundPlayer Lilith(UNICODE 非対応版)
static const TCHAR g_cszServiceNameNewLilith[] = _T("uLilithDdeControl"); //uLilith(UNICODE 対応版)
static const TCHAR g_cszTopicNameNewLilith[] = _T("uLilithDdeControl"); //uLilith(UNICODE 対応版)
//static const TCHAR g_cszTopicNameNewLilith[] = _T("uLilithDdeControlFixed"); //uLilith(UNICODE 対応版)
//KbMedia Player
static const TCHAR g_cszMutexKbmplay[] = _T("KbMedia Player");    //起動済 Mutex 名(2.80beta9以降は64bit版でも有効)
static const TCHAR g_cszMutexKbmplay64[] = _T("KbMedia Player64");//64bit版起動済 Mutex 名
static const TCHAR g_cszServiceNameKbmplay[] = _T("KbMedia Player");//32bit/64bit共通
static const TCHAR g_cszTopicNameKbmplay[] = _T("KbMedia Player");  //32bit/64bit共通

static const DWORD DDE_TIMEOUT = 3000;
///////////////////////////////////////////////////////////////////////////////
CPlayerControl::CPlayerControl(void)
{
    m_szPlayerPath[0] = 0;
    m_nPlayerType = PLAYER_EXTEND;
};
///////////////////////////////////////////////////////////////////////////////
CPlayerControl::~CPlayerControl(void)
{
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CPlayerControl::ExecPlayer(const TCHAR *cszFileName, BOOL bClear)
{//cszFileName を指定して開く
 //bClear == TRUE の場合は（対応していれば）プレイリストを破棄する
    if(!m_szPlayerPath[0]){
        return FALSE;
    }
    TCHAR szCommand[2048];
    BOOL bExecCommand = FALSE;//後で ExecCommand を実行する場合は TRUE;
    switch(m_nPlayerType){
        case PLAYER_KBMPLAY:
            if(bClear){//リストを破棄=>追加=>再生
                _sntprintf_s(szCommand, _TRUNCATE, 
                             _T("\"%s\" \"%s\" /clear /a /p"), m_szPlayerPath, cszFileName);
            }
            else{
                //リストに追加しないで再生
                _sntprintf_s(szCommand, _TRUNCATE, 
                             _T("\"%s\" \"%s\" /na /play"), m_szPlayerPath, cszFileName);
            }
            break;
        case PLAYER_LILITH:
        case PLAYER_ULILITH:
        	// 起動するだけ(後で DDE コマンド発行)
            _sntprintf_s(szCommand, _TRUNCATE, 
                         _T("\"%s\""), m_szPlayerPath);//"" で括る
            bExecCommand = TRUE;
            break;
        case PLAYER_WINAMP:
        case PLAYER_SCMPX:
            // 起動するだけ(後で再生開始コマンド発行)
            _sntprintf_s(szCommand, _TRUNCATE, 
                         _T("\"%s\""), m_szPlayerPath);//"" で括る
            bExecCommand = TRUE;
            break;
        case PLAYER_FOOBAR:
        default:
        	// 引数にファイル名を渡して起動
            _sntprintf_s(szCommand, _TRUNCATE, 
                         _T("\"%s\" \"%s\""), m_szPlayerPath, cszFileName);//"" で括る
            break;
    }
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    if(CreateProcess(m_szPlayerPath,
                     szCommand,
                     NULL,
                     NULL,
                     FALSE, //bInheritHandles
                     0,     //dwCreationFlags
                     0,
                     NULL,  //lpCurrentDirectory,
                     &si,
                     &pi)){
        DWORD dwWaitRet = WaitForInputIdle(pi.hProcess, 3000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if(dwWaitRet != WAIT_TIMEOUT && bExecCommand){
            if(m_nPlayerType == PLAYER_LILITH ||
               m_nPlayerType == PLAYER_ULILITH){
            //DDE 制御が可能になるまで待つ
                DWORD dwStartTime = GetTickCount();
                do{
                    TCHAR *pszReturn;
                    RequestCommand(_T("/query_dde_status"), &pszReturn);
                    if(pszReturn){
                        if(_tcsicmp(pszReturn, _T("dde_acceptable")) == 0){
                            free(pszReturn);
                            break;
                        }
                        free(pszReturn);
                    }
                    Sleep(500);
                }while(GetTickCount() - dwStartTime < 10000);//最大で10秒間待つ
            }
            else{
                Sleep(2000);//少し待ってからにする
            }
            if(bClear){//リストを破棄+追加+演奏
                ExecCommand(COMMAND_PLAY, cszFileName);
            }
            else{//追加しないで再生
                ExecCommand(COMMAND_NOADDPLAY, cszFileName);
            }
        }
        return TRUE;
    }
    else{
        return FALSE;
    }
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CPlayerControl::ExecCommand(int nCommand, const TCHAR *cszFileName)
{
    if(!SupportCommand(nCommand)){
        if(nCommand == COMMAND_NOADDPLAY){
            nCommand = COMMAND_PLAY;
        }
        else{
            return;
        }
    }
    if(!PlayerIsActive()){
        return;
    }
    switch(m_nPlayerType){
        case PLAYER_WINAMP:
        case PLAYER_SCMPX:
            ExecCommandWinamp(nCommand, cszFileName);
            break;
        case PLAYER_KBMPLAY:
            ExecCommandKbmplay(nCommand, cszFileName);
            break;
        case PLAYER_LILITH:
        case PLAYER_ULILITH:
            ExecCommandLilith(nCommand, cszFileName);
            break;
        case PLAYER_FOOBAR:
            ExecCommandFoobar(nCommand, cszFileName);
        default:
            break;
    }
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CPlayerControl::SupportCommand(int nCommand)
{//制御対象のプレイヤーが nCommand に対応していれば TRUE を返す
/*
    enum{
        COMMAND_NOADDPLAY,//追加しないで再生(未対応の場合は COMMAND_PLAY を呼ぶ)
        COMMAND_PLAY,  //再生
        COMMAND_STOP,  //停止
        COMMAND_NEXT,  //次の曲
        COMMAND_PREV,  //前の曲
        COMMAND_CLOSE, //停止&ファイルクローズ(再生中の曲のタグ更新が出来るように)
        COMMAND_CLEAR, //リストを破棄
        COMMAND_ADD,   //リストに追加
        COMMAND_QUIT   //プレイヤー終了
    };
*/
    if(!m_szPlayerPath[0]){
        return FALSE;
    }
    if(nCommand == COMMAND_PLAY){//全てのプレイヤーが必ず対応
        return TRUE;
    }
    BOOL bRet = FALSE;
    if(m_nPlayerType == PLAYER_WINAMP){
        switch(nCommand){
            //case COMMAND_NOADDPLAY://追加しないで再生(未対応の場合は COMMAND_PLAY を呼ぶ)
            case COMMAND_STOP:  //停止
            case COMMAND_NEXT:  //次の曲
            case COMMAND_PREV:  //前の曲
            //case COMMAND_CLOSE: //停止&ファイルクローズ(再生中の曲のタグ更新が出来るように)
            case COMMAND_CLEAR: //リストを破棄
            case COMMAND_ADD:   //リストに追加
            case COMMAND_QUIT:  //プレイヤー終了
                bRet = TRUE;
                break;
        }
    }
    else if(m_nPlayerType == PLAYER_SCMPX){
        switch(nCommand){
            //case COMMAND_NOADDPLAY://追加しないで再生(未対応の場合は COMMAND_PLAY を呼ぶ)
            //case COMMAND_STOP:  //停止
            //case COMMAND_NEXT:  //次の曲
            //case COMMAND_PREV:  //前の曲
            //case COMMAND_CLOSE: //停止&ファイルクローズ(再生中の曲のタグ更新が出来るように)
            case COMMAND_CLEAR: //リストを破棄
            case COMMAND_ADD:   //リストに追加
            case COMMAND_QUIT:  //プレイヤー終了
                bRet = TRUE;
                break;
        }
    }
    else if(m_nPlayerType == PLAYER_KBMPLAY){
        switch(nCommand){
            case COMMAND_NOADDPLAY://追加しないで再生(未対応の場合は COMMAND_PLAY を呼ぶ)
            case COMMAND_STOP:  //停止
            case COMMAND_NEXT:  //次の曲
            case COMMAND_PREV:  //前の曲
            case COMMAND_CLOSE: //停止&ファイルクローズ(再生中の曲のタグ更新が出来るように)
            case COMMAND_CLEAR: //リストを破棄
            case COMMAND_ADD:   //リストに追加
            case COMMAND_QUIT:  //プレイヤー終了
                bRet = TRUE;//全部対応
                break;
        }
    }
    else if(m_nPlayerType == PLAYER_LILITH ||
            m_nPlayerType == PLAYER_ULILITH){
        switch(nCommand){
            case COMMAND_NOADDPLAY://追加しないで再生(未対応の場合は COMMAND_PLAY を呼ぶ)
            case COMMAND_STOP:  //停止
            case COMMAND_NEXT:  //次の曲
            case COMMAND_PREV:  //前の曲
            case COMMAND_CLOSE: //停止&ファイルクローズ(再生中の曲のタグ更新が出来るように)
            case COMMAND_CLEAR: //リストを破棄
            case COMMAND_ADD:   //リストに追加
            case COMMAND_QUIT:  //プレイヤー終了
                bRet = TRUE;//全部対応
                break;
        }
    }
    else if(m_nPlayerType == PLAYER_FOOBAR){
        switch(nCommand){
            //case COMMAND_NOADDPLAY://追加しないで再生(未対応の場合は COMMAND_PLAY を呼ぶ)
            case COMMAND_STOP:  //停止
            case COMMAND_NEXT:  //次の曲
            case COMMAND_PREV:  //前の曲
            //case COMMAND_CLOSE: //停止&ファイルクローズ(再生中の曲のタグ更新が出来るように)
            //case COMMAND_CLEAR: //リストを破棄
            case COMMAND_ADD:   //リストに追加
            case COMMAND_QUIT:  //プレイヤー終了
                bRet = TRUE;
                break;
        }
    }
    return bRet;
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CPlayerControl::RequestCommand(const TCHAR *cszCommand, 
                                               TCHAR **ppszReturn)
{//*ppszReturn は呼び出し側で free() すること
    *ppszReturn = NULL;
    if(!PlayerIsActive()){//制御対象プレイヤーが未起動
        return FALSE;
    }
    if(m_nPlayerType == PLAYER_KBMPLAY){
        return RequestCommandKbmplay(cszCommand, ppszReturn);
    }
    else if(m_nPlayerType == PLAYER_LILITH ||
            m_nPlayerType == PLAYER_ULILITH){
        return RequestCommandLilith(cszCommand, ppszReturn);
    }
    return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CPlayerControl::RequestCommandKbmplay(const TCHAR *cszCommand, 
                                                      TCHAR **ppszReturn)
{//*ppszReturn は呼び出し側で free() すること
    KbDDEClient client(NULL, g_cszServiceNameKbmplay, g_cszTopicNameKbmplay);
    return client.Request(ppszReturn, cszCommand, DDE_TIMEOUT, FALSE); 
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CPlayerControl::RequestCommandLilith(const TCHAR *cszCommand, 
                                               TCHAR **ppszReturn)
{//*ppszReturn は呼び出し側で free() すること
    BOOL bIsNewLilith = (m_nPlayerType == PLAYER_ULILITH);
    const TCHAR *cszServiceName;
    const TCHAR *cszTopicName;
    if(bIsNewLilith){
        cszServiceName = g_cszServiceNameNewLilith;
        cszTopicName = g_cszTopicNameNewLilith;
    }
    else{
        cszServiceName = g_cszServiceNameOldLilith;
        cszTopicName = g_cszTopicNameOldLilith;
    }
    KbDDEClient client(NULL, cszServiceName, cszTopicName);
    return client.Request(ppszReturn, cszCommand, DDE_TIMEOUT, bIsNewLilith); 
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CPlayerControl::ExecCommandWinamp(int nCommand, const TCHAR *cszFileName)
{
	HWND hWndPlayer = NULL;
    if(m_nPlayerType == PLAYER_WINAMP){
        hWndPlayer = FindWindow(g_cszClassNameWinamp, NULL);
    }
    else if(m_nPlayerType == PLAYER_SCMPX){
        hWndPlayer = FindWindow(g_cszClassNameSCMPX, NULL);
    }
    if(!hWndPlayer){
        return;
    }
    switch(nCommand){
    case COMMAND_NOADDPLAY://未対応なので追加して再生と同等に扱う
    case COMMAND_PLAY:{
        //リストを破棄 => 追加 => 再生
        SendMessage(hWndPlayer, WM_WA_IPC, 0, IPC_DELETE);//リストを破棄
        ExecCommandWinamp(COMMAND_ADD, cszFileName);      //追加
        SendMessage(hWndPlayer, WM_WA_IPC, 0, IPC_STARTPLAY);//再生
		break;
    }
    case COMMAND_STOP://停止
		SendMessage(hWndPlayer, WM_COMMAND, WINAMP_BUTTON4, 0);
		break;
	case COMMAND_PREV://前の曲
		SendMessage(hWndPlayer, WM_COMMAND, WINAMP_BUTTON1, 0);
		break;
	case COMMAND_NEXT://次の曲
		SendMessage(hWndPlayer, WM_COMMAND, WINAMP_BUTTON5, 0);
		break;
    case COMMAND_CLEAR://リストを破棄
        ::SendMessage(hWndPlayer, WM_WA_IPC, 0, IPC_DELETE);
        break;
    case COMMAND_ADD://リストに追加
		if (m_nPlayerType == PLAYER_WINAMP) {
			// Winamp
            COPYDATASTRUCT cds = {0};
#ifdef _UNICODE
            cds.dwData = IPC_PLAYFILEW;
#else
            cds.dwData = IPC_PLAYFILE;
#endif
            cds.lpData = _tcsdup(cszFileName);
            cds.cbData = (_tcslen(cszFileName) + 1) * sizeof(TCHAR);
			::SendMessage(hWndPlayer, WM_COPYDATA, NULL, (LPARAM)&cds);
            free(cds.lpData);
		} else {
			// SCMPX
#ifdef _UNICODE
            char *str_ansi = conv_utf16_to_ansi(cszFileName);
#else
            const char *str_ansi = cszFileName;
#endif
            int len_ansi = strlen(str_ansi) + 1;
			int i; 
            for (i = 0; i < len_ansi; i++) {
				SendMessage(hWndPlayer, WM_WA_IPC, str_ansi[i], IPC_PLAYFILE);
			}
#ifdef _UNICODE
            free(str_ansi);
#endif
		} 
        break;
    case COMMAND_QUIT://終了
        SendMessage(hWndPlayer, WM_CLOSE, NULL, NULL);
        break;
    }
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CPlayerControl::ExecCommandKbmplay(int nCommand, const TCHAR *cszFileName)
{//KbMedia Player は起動済みであることが前提
    TCHAR szCommand[2048];
    szCommand[0] = 0;
    switch (nCommand){
    case COMMAND_NOADDPLAY://追加しないで再生
        _sntprintf_s(szCommand, _TRUNCATE, _T("\"%s\" /na /play"), cszFileName);
        break;       
    case COMMAND_PLAY://リストを破棄 => 追加 => 再生
        _sntprintf_s(szCommand, _TRUNCATE, _T("\"%s\" /clear /a /play"), cszFileName);//リストを破棄してから追加
        break;
    case COMMAND_STOP:// 停止 
		_tcsncpy_s(szCommand, _T("/stop"), _TRUNCATE);
		break;
	case COMMAND_PREV:// 前の曲
		_tcsncpy_s(szCommand, _T("/prev"), _TRUNCATE);
		break;
	case COMMAND_NEXT:// 次の曲
		_tcsncpy_s(szCommand, _T("/next"), _TRUNCATE);
		break;
    case COMMAND_CLOSE://演奏停止＆ファイルクローズ
		_tcsncpy_s(szCommand, _T("/fileclose"), _TRUNCATE);
        break;
    case COMMAND_CLEAR://リストを破棄
		_tcsncpy_s(szCommand, _T("/clear"), _TRUNCATE);
        break;
    case COMMAND_ADD://追加(再生はしない)
        _sntprintf_s(szCommand, _TRUNCATE, _T("\"%s\" /a /np"), cszFileName);
        break;
    case COMMAND_QUIT://終了
		_tcsncpy_s(szCommand, _T("/quit"), _TRUNCATE);
		break;
    }//switch(nCommand)
    if(szCommand[0]){
        KbDDEClient client(NULL, g_cszServiceNameKbmplay, g_cszTopicNameKbmplay);
        client.Execute(szCommand, DDE_TIMEOUT);
    }
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CPlayerControl::ExecCommandLilith(int nCommand, const TCHAR *cszFileName)
{//Lilith は起動済みであることが前提
    const TCHAR *cszTopicName;
    const TCHAR *cszServiceName;
    if(m_nPlayerType == PLAYER_ULILITH){
        cszServiceName = g_cszServiceNameNewLilith;
        cszTopicName = g_cszTopicNameNewLilith;
    }
    else{
        cszServiceName = g_cszServiceNameOldLilith;
        cszTopicName = g_cszTopicNameOldLilith;
    }

    TCHAR szCommand[2048];
    szCommand[0] = 0;
    switch (nCommand){
    case COMMAND_NOADDPLAY://追加しないで再生
        _sntprintf_s(szCommand, _TRUNCATE, _T("/dopen \"%s\" /play"), cszFileName);
        break;
    case COMMAND_PLAY://リストを破棄 => 追加 => 再生
        _sntprintf_s(szCommand, _TRUNCATE, _T("/clear /open \"%s\" /play"), cszFileName);
        break;
    case COMMAND_STOP:// 停止 
        _tcsncpy_s(szCommand, _T("/stop"), _TRUNCATE);
		break;
	case COMMAND_PREV:// 前の曲
        _tcsncpy_s(szCommand, _T("/back"), _TRUNCATE);
		break;
	case COMMAND_NEXT:// 次の曲
        _tcsncpy_s(szCommand, _T("/next"), _TRUNCATE);
		break;
    case COMMAND_CLOSE://演奏停止＆ファイルクローズ
        _tcsncpy_s(szCommand, _T("/stop"), _TRUNCATE);
        break;
    case COMMAND_CLEAR://リストを破棄
        _tcsncpy_s(szCommand, _T("/clear"), _TRUNCATE);
        break;
    case COMMAND_ADD:{//追加
        _sntprintf_s(szCommand, _TRUNCATE, _T("/add \"%s\""), cszFileName);
        break;
    }
    case COMMAND_QUIT://終了
        _tcsncpy_s(szCommand, _T("/exit"), _TRUNCATE);
		break;
    }//switch(nCommand)
    if(szCommand[0]){
        KbDDEClient client(NULL, cszServiceName, cszTopicName);
        client.Execute(szCommand, DDE_TIMEOUT);
    }
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CPlayerControl::ExecCommandFoobar(int nCommand, const TCHAR *cszFileName)
{//Foobar は起動済みであることが前提
    TCHAR szCommandLine[2048];
    TCHAR szCommand[2048];
    szCommand[0] = 0;
    switch (nCommand){
    case COMMAND_NOADDPLAY://追加しないで再生(未対応なので COMMAND_PLAY と同等に処理)
    case COMMAND_PLAY://リストを破棄 =>追加=>再生
        //ファイル名を渡して起動するだけで↑のようになる
        _sntprintf_s(szCommand, _TRUNCATE, _T("\"%s\""), cszFileName);
        break;
    case COMMAND_ADD://追加
        _sntprintf_s(szCommand, _TRUNCATE, _T("\"%s\" /add"), cszFileName);
        break;
    case COMMAND_STOP://停止 
        _tcsncpy_s(szCommand, _T("/stop"), _TRUNCATE);
		break;
	case COMMAND_PREV://前の曲
        _tcsncpy_s(szCommand, _T("/prev"), _TRUNCATE);
		break;
	case COMMAND_NEXT://次の曲
        _tcsncpy_s(szCommand, _T("/next"), _TRUNCATE);
		break;
    case COMMAND_CLOSE://演奏停止＆ファイルクローズ
        _tcsncpy_s(szCommand, _T("/stop"), _TRUNCATE);
        break;
    case COMMAND_QUIT://終了
        _tcsncpy_s(szCommand, _T("/exit"), _TRUNCATE);
		break;
    //case COMMAND_CLEAR://リストを破棄
    //    break;
    }//switch(nCommand)
    _sntprintf_s(szCommandLine, _TRUNCATE, 
                 _T("\"%s\" %s"), 
                 m_szPlayerPath, szCommand);

    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    if(CreateProcess(m_szPlayerPath,
                     szCommandLine,
                     NULL,
                     NULL,
                     FALSE, //bInheritHandles
                     0,     //dwCreationFlags
                     0,
                     NULL,  //lpCurrentDirectory,
                     &si,
                     &pi)){
        WaitForSingleObject(pi.hProcess, 2000);//プロセスが終了するまで最大2秒待つ
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if(nCommand == COMMAND_PLAY){
        //多重起動時はファイル名を渡してから一定時間が経過しないとリストが
        //破棄されないようなので少し待つ（どの程度待てば良いかは不明)
            Sleep(2000);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CPlayerControl::SetPlayer(const TCHAR *cszPlayerPath, int nPlayerType)
{//制御対象のプレイヤーを設定
    _tcsncpy_s(m_szPlayerPath, cszPlayerPath, _TRUNCATE);
    m_nPlayerType = nPlayerType;
    switch(m_nPlayerType){
        case PLAYER_WINAMP:
        case PLAYER_SCMPX:
        case PLAYER_KBMPLAY:
        case PLAYER_LILITH:
        case PLAYER_ULILITH:
        case PLAYER_FOOBAR:
            break;
        default:
            m_nPlayerType =PLAYER_EXTEND;
            break;
    }
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CPlayerControl::PlayerIsActive(void)
{//制御対象のプレイヤーが起動済みなら TRUE を返す
    BOOL bRet = FALSE;
    switch(m_nPlayerType){
        case PLAYER_WINAMP:{
            bRet = FindWindow(g_cszClassNameWinamp, NULL) != NULL;
            break;
        }
        case PLAYER_SCMPX:{
            bRet = FindWindow(g_cszClassNameSCMPX, NULL) != NULL;
            break;
        }
        case PLAYER_LILITH:{
            bRet = FindWindow(g_cszClassNameOldLilith, NULL) != NULL;
            break;
        }
        case PLAYER_ULILITH:{
            bRet = FindWindow(g_cszClassNameNewLilith, NULL) != NULL;
            break;
        }
        case PLAYER_KBMPLAY:{
            HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, g_cszMutexKbmplay);
            if(!hMutex){
                hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, g_cszMutexKbmplay64);
            }
            if(hMutex){
                CloseHandle(hMutex);
                bRet = TRUE;
            }
            break;
        }
        case PLAYER_FOOBAR:{//バージョンによってクラス名が異なるかも？
            bRet = FindWindow(g_cszClassNameFoobar, NULL) != NULL;
            break;
        }
    }
    return bRet;
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CPlayerControl::Play(const TCHAR *cszFileName, BOOL bAdd)
{//bAdd == TRUE の場合は追加して再生
    if(!PlayerIsActive()){//プレイヤーが未起動なら起動
        BOOL bClear = bAdd;//追加して再生する場合はリストを破棄
        ExecPlayer(cszFileName, bClear);
        return;
    }
    if(bAdd){
        ExecCommand(COMMAND_PLAY, cszFileName);
    }
    else{
        ExecCommand(COMMAND_NOADDPLAY, cszFileName);
    }
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CPlayerControl::GetPlayingFileName(TCHAR *pszFileName, int nSize)//nSize は文字数
{//プレイヤーが再生中のファイル名を pszFileName に格納
    pszFileName[0] = 0;
    TCHAR *pszReturn = NULL;
    //RequestCommandXXXX を直接呼ばないようにする
    //m_szPlayerPath の有効性チェック、プレイヤーの起動済みチェック等が RequestCommand 内
    //で行われる
    if(m_nPlayerType == PLAYER_KBMPLAY){
        RequestCommand(_T("filename"), &pszReturn);
        if(pszReturn){
            _tcsncpy_s(pszFileName, nSize, pszReturn, _TRUNCATE);
            free(pszReturn);
        }
    }
    else if(m_nPlayerType == PLAYER_LILITH ||
            m_nPlayerType == PLAYER_ULILITH){
        //filepath => filename の順に取得(一発で取得する方法がない)
        RequestCommand(_T("/fileinfo filepath"), &pszReturn);
        if(!pszReturn){
            return FALSE;
        }
        if(!pszReturn[0]){
            free(pszReturn);
            return FALSE;
        }
        TCHAR *pszReturn2;
        RequestCommand(_T("/fileinfo filename"), &pszReturn2);
        if(!pszReturn2){
            free(pszReturn);
            return FALSE;
        }
        if(pszReturn2[0]){
            _sntprintf_s(pszFileName, nSize, _TRUNCATE, _T("%s\\%s"), pszReturn, pszReturn2);
        }
        free(pszReturn2);
        free(pszReturn);
    }
    return pszFileName[0] != 0;
}
///////////////////////////////////////////////////////////////////////////////

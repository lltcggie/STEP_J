//---------------------------------------------------------------------------
#include "stdafx.h"
#include <windows.h>
#include "kbdde.h"

//---------------------------------------------------------------------------
HDDEDATA CALLBACK DefCallback(UINT uType, UINT uFmt,
    HCONV hConv, HSZ hszTpc1, HSZ hszTpc2, HDDEDATA hdata,
    ULONG_PTR dwData1, ULONG_PTR dwData2);
HDDEDATA CALLBACK DefCallback(UINT uType, UINT uFmt,
    HCONV hConv, HSZ hszTpc1, HSZ hszTpc2, HDDEDATA hdata,
    ULONG_PTR dwData1, ULONG_PTR dwData2)
{
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
KbDDE::KbDDE(PFNCALLBACK pfnCallBack, LPCTSTR cszService, LPCTSTR cszTopic, DWORD afCmd)
{
    m_ddeInst = 0;
    m_hszService = NULL;
    m_hszTopic = NULL;
    if(!pfnCallBack){
        pfnCallBack = DefCallback;
    }
    if(DdeInitialize(&m_ddeInst, pfnCallBack, afCmd, 0) != DMLERR_NO_ERROR) {
        m_ddeInst = 0;
        return;
    }
#ifndef _UNICODE
    m_hszService = DdeCreateStringHandle(m_ddeInst, cszService, CP_WINANSI);
    m_hszTopic = DdeCreateStringHandle(m_ddeInst, cszTopic, CP_WINANSI);
#else
    m_hszService = DdeCreateStringHandle(m_ddeInst, cszService, CP_WINUNICODE);
    m_hszTopic = DdeCreateStringHandle(m_ddeInst, cszTopic, CP_WINUNICODE);
#endif
    if(DdeGetLastError(m_ddeInst) != DMLERR_NO_ERROR) {
        DdeUninitialize(m_ddeInst);
        m_ddeInst= 0;
        m_hszService = NULL;
        m_hszTopic = NULL;
        return;
    }
}
///////////////////////////////////////////////////////////////////////////////
KbDDE::~KbDDE(void)
{
    if(!m_ddeInst){
        return;
    }
    DdeFreeStringHandle(m_ddeInst, m_hszService);
    DdeFreeStringHandle(m_ddeInst, m_hszTopic);
    DdeUninitialize(m_ddeInst);
}
///////////////////////////////////////////////////////////////////////////////
DWORD __fastcall KbDDEServer::QueryString(HSZ hsz, TCHAR *szBuffer, int Size)
{
#ifndef _UNICODE
    return DdeQueryString(m_ddeInst, hsz, szBuffer, Size, CP_WINANSI);
#else
    return DdeQueryString(m_ddeInst, hsz, szBuffer, Size, CP_WINUNICODE);
#endif
}
///////////////////////////////////////////////////////////////////////////////
HDDEDATA __fastcall KbDDEServer::CreateDataHandle(LPBYTE pSrc, DWORD cb, HSZ hsz, UINT wFmt)
{
    return DdeCreateDataHandle(m_ddeInst, pSrc, cb, 0, hsz, wFmt, 0);
}
///////////////////////////////////////////////////////////////////////////////
/*
    KbDDEServer
*/
///////////////////////////////////////////////////////////////////////////////
KbDDEServer::KbDDEServer(PFNCALLBACK pfnCallBack, LPCTSTR cszTopic, LPCTSTR cszService)
    :KbDDE(pfnCallBack, cszService, cszTopic, APPCLASS_STANDARD)
{
    DdeNameService(m_ddeInst, m_hszService, 0, DNS_REGISTER);
}
///////////////////////////////////////////////////////////////////////////////
KbDDEServer::~KbDDEServer(void)
{
    if(m_ddeInst){
        DdeNameService(m_ddeInst, m_hszService, 0, DNS_UNREGISTER);
    }
}
///////////////////////////////////////////////////////////////////////////////
/*
    KbDDEClient
*/
///////////////////////////////////////////////////////////////////////////////
KbDDEClient::KbDDEClient(PFNCALLBACK pfnCallBack, LPCTSTR cszService, LPCTSTR cszTopic)
    :KbDDE(pfnCallBack, cszService, cszTopic, APPCMD_CLIENTONLY)
{
    m_hConv = DdeConnect(m_ddeInst, m_hszService, m_hszTopic, NULL);
}
///////////////////////////////////////////////////////////////////////////////
KbDDEClient::~KbDDEClient(void)
{
    if(m_hConv){
        DdeDisconnect(m_hConv);
    }
}
///////////////////////////////////////////////////////////////////////////////
HDDEDATA KbDDEClient::ClientTransaction(
            LPBYTE pData,       // サーバーに渡すデータの先頭バイトのポインタ
            DWORD cbData,       // データの長さ
        //    HCONV hConv,        // 通信ハンドル
            HSZ hszItem,        // データ項目のハンドル
            UINT wFmt,          // クリップボードフォーマット
            UINT wType,         // トランザクションタイプ
            DWORD dwTimeout,    // 待ち時間
            LPDWORD pdwResult   // トランザクションの結果へのポインタ
        )
{
    if(!m_hConv)return NULL;
    return DdeClientTransaction(
            pData,       // サーバーに渡すデータの先頭バイトのポインタ
            cbData,      // データの長さ
            m_hConv,     // 通信ハンドル
            hszItem,     // データ項目のハンドル
            wFmt,        // クリップボードフォーマット
            wType,       // トランザクションタイプ
            dwTimeout,   // 待ち時間
            pdwResult    // トランザクションの結果へのポインタ
           );
}
///////////////////////////////////////////////////////////////////////////////
bool __fastcall KbDDEClient::Execute(LPCTSTR cszCommand, DWORD dwTimeOut)
{
    static TCHAR cszEmpty[] = _T("");
    if(!m_hConv){
        return false;
    }
    if(!cszCommand){
        cszCommand = cszEmpty;
    }
    TCHAR *pszTopic = _tcsdup(cszCommand);
    DWORD cbTopic = (_tcslen(pszTopic) + 1) * sizeof(TCHAR);
    HDDEDATA hRet;
    hRet = DdeClientTransaction(
            (BYTE*)pszTopic,
            cbTopic,
            m_hConv,
            NULL,
            NULL,/*CF_TEXT,*/
            XTYP_EXECUTE,
            dwTimeOut,//待機時間
            NULL);

    free(pszTopic);
    if(!hRet && DdeGetLastError(m_ddeInst) != DMLERR_NO_ERROR){
        return false;
    }
    else if(hRet){
        DdeFreeDataHandle(hRet);
    }
    return true;
}
bool __fastcall KbDDEClient::Request(TCHAR** ppszReturn,
                                     const TCHAR *cszCommand,
                                     DWORD dwWait,
                                     BOOL bCfTextAsUnicode)
{//XTYP_REQUEST コマンドを発行する
 //戻り値を *ppszReturn に格納
 //bCfTextAsUnicode == TRUE の場合、CF_TEXT で要求したときの
 //戻り値を ANSI ではなく UNICODE として処理(uLilith 対策)
    if (!ppszReturn) {
        return false;
    }
    *ppszReturn = NULL;
    if (!cszCommand) {
        return false;
    }
    if (!m_hConv) {
        return false;
    }
    HSZ      hszItem;
    HDDEDATA hRet;
#ifdef _UNICODE
    hszItem = DdeCreateStringHandle(m_ddeInst, cszCommand, CP_WINUNICODE);
    hRet = DdeClientTransaction(NULL, 0, m_hConv, hszItem, CF_UNICODETEXT, 
                                XTYP_REQUEST, dwWait, NULL);
    if (hRet) {
        int nSize = DdeGetData(hRet, NULL, 0, 0);
        WCHAR *pszReturn = (WCHAR*)malloc(nSize + 2);
        DdeGetData(hRet, (BYTE*)pszReturn, nSize, 0);
        pszReturn[nSize/sizeof(WCHAR)] = 0;
        DdeFreeStringHandle(m_ddeInst, hszItem);
        DdeFreeDataHandle(hRet);
        *ppszReturn = pszReturn;
        return true;
    }
    //CF_TEXT で再取得を試みる
    //CF_TEXT でも実際のデータは UNICODE の場合と ANSI の場合とがあり、
    //どちらになるかはサーバーにより異なる
    //uLilith は CF_UNICODETEXT だと失敗、CF_TEXT だと UNICODE
    //Lilith は CF_UNICODETEXT だと失敗、CF_TEXT だと ANSI
    //KbMedia Player v2.80 以降は CF_UNICODETEXT だと UNICODE、CF_TEXT だと ANSI
    //KbMedia Player v2.6x は CF_UNICODETEXT だと失敗、CF_TEXT だと ANSI
    hRet = DdeClientTransaction(NULL, 0, m_hConv, hszItem,
                                CF_TEXT, XTYP_REQUEST, dwWait, NULL);
#else
    hszItem = DdeCreateStringHandle(m_ddeInst, cszCommand, CP_WINANSI);
    hRet = DdeClientTransaction(NULL, 0, m_hConv, hszItem, CF_TEXT, 
                                XTYP_REQUEST, dwWait, NULL);
#endif
    if (hRet) {
        int nSize = DdeGetData(hRet, NULL, 0, 0);
        CHAR *pszReturn = (CHAR*)malloc(nSize+3);
        DdeGetData(hRet, (BYTE*)pszReturn, nSize, 0);
        pszReturn[nSize] = pszReturn[nSize+1] = pszReturn[nSize+2];
        DdeFreeStringHandle(m_ddeInst, hszItem);
        DdeFreeDataHandle(hRet);
        if(bCfTextAsUnicode){//pszReturn を UNICODE として扱う(uLilith対策)
#ifdef _UNICODE
            *ppszReturn = (WCHAR*)pszReturn;
#else
            //pszReturn を ANSI に変換(pszReturn をそのまま使うのではない(ややこしい)
            int lenReturnA = WideCharToMultiByte(CP_ACP, 0, (WCHAR*)pszReturn, -1, 0, 0, NULL, NULL);
            CHAR *pszReturnA = (CHAR*)malloc(lenReturnA);
            WideCharToMultiByte(CP_ACP, 0, (WCHAR*)pszReturn, -1, pszReturnA, lenReturnA, NULL, NULL);
            *ppszReturn  = pszReturnA;
            free(pszReturn);
#endif
        }
        else{
#ifdef _UNICODE
            //pszReturn を UNICODE に変換
            int lenReturnW = MultiByteToWideChar(CP_ACP, 0, pszReturn, -1, 0, 0);
            WCHAR *pszReturnW = (WCHAR*)malloc(lenReturnW*sizeof(WCHAR));
            MultiByteToWideChar(CP_ACP, 0, (CHAR*)pszReturn, -1, pszReturnW, lenReturnW);
            *ppszReturn = pszReturnW;
            free(pszReturn);
#else]
            *ppszReturn = pszReturn;
#endif
        }
        return true;
    }
    else {
        DdeFreeStringHandle(m_ddeInst, hszItem);
        return false;
    }
}


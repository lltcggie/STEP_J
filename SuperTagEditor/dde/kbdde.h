//---------------------------------------------------------------------------
#ifndef kbddeH
#define kbddeH
//---------------------------------------------------------------------------
#include <ddeml.h>

class KbDDE
{
protected:
    DWORD m_ddeInst;
    HSZ   m_hszService;
    HSZ   m_hszTopic;
public:
    KbDDE(PFNCALLBACK pfnCallBack, LPCTSTR cszService, LPCTSTR cszTopic, DWORD afCmd);
    ~KbDDE(void);
};
///////////////////////////////////////////////////////////////////////////////
class KbDDEServer : public KbDDE
{
public:
    DWORD __fastcall QueryString(HSZ hsz, TCHAR *szBuffer, int Size);
    HDDEDATA __fastcall CreateDataHandle(LPBYTE pSrc, DWORD cb, HSZ hsz, UINT wFmt);
    KbDDEServer(PFNCALLBACK pfnCallBack, LPCTSTR cszService, LPCTSTR cszTopic);
    ~KbDDEServer(void);
};
///////////////////////////////////////////////////////////////////////////////
class KbDDEClient : public KbDDE
{
private:
    HCONV m_hConv;
    HDDEDATA ClientTransaction(
        LPBYTE pData,       // サーバーに渡すデータの先頭バイトのポインタ
        DWORD cbData,       // データの長さ
    //    HCONV hConv,        // 通信ハンドル
        HSZ hszItem,        // データ項目のハンドル
        UINT wFmt,          // クリップボードフォーマット
        UINT wType,         // トランザクションタイプ
        DWORD dwTimeout,    // 待ち時間
        LPDWORD pdwResult   // トランザクションの結果へのポインタ
    );
public:
    bool __fastcall Execute(LPCTSTR cszCommand, DWORD dwTimeOut);
    bool __fastcall Request(TCHAR** ppszReturn, const TCHAR *cszCommand, DWORD dwWait, BOOL bCfTextAsUnicode);
    KbDDEClient(PFNCALLBACK pfnCallBack, LPCTSTR cszService, LPCTSTR cszTopic);
    ~KbDDEClient(void);
};
///////////////////////////////////////////////////////////////////////////////
#endif

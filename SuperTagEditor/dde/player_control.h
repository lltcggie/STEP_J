#pragma once

//プレイヤー制御を管理
//SuperTagEditorView.h/cpp から分離

class CPlayerControl
{
private:
    enum{
        COMMAND_NOADDPLAY,//追加しないで再生(未対応の場合は COMMAND_PLAY を呼ぶ)
        COMMAND_PLAY,   //再生
        COMMAND_STOP,   //停止
        COMMAND_NEXT,   //次の曲
        COMMAND_PREV,   //前の曲
        COMMAND_CLOSE,  //停止&ファイルクローズ(再生中の曲のタグ更新が出来るように)
        COMMAND_CLEAR,  //リストを破棄
        COMMAND_ADD,    //リストに追加
        COMMAND_QUIT    //プレイヤー終了
    };
    TCHAR m_szPlayerPath[MAX_PATH];
    int   m_nPlayerType;
    //対応プレイヤーを増やした場合は以下の関数を変更する
    BOOL __fastcall PlayerIsActive(void);//対象プレイヤーが起動済みなら TRUE を返す
    BOOL __fastcall ExecPlayer(const TCHAR *cszFileName, BOOL bClear);//プレイヤーを起動する(bClear=TRUEの場合は起動後リストを破棄)
    void __fastcall ExecCommand(int nCommand, const TCHAR *cszFileName);
    BOOL __fastcall SupportCommand(int nCommand);//COMMAND_XXXX に対応していれば TRUE を返す
    BOOL __fastcall RequestCommand(const TCHAR *cszCommand, TCHAR **ppszReturn);
    //ここまで
    //必要に応じて ExecCommandXXXX/RequestCommandXXXX を追加
    void __fastcall ExecCommandWinamp(int nCommand, const TCHAR *cszFileName);
    void __fastcall ExecCommandKbmplay(int nCommand, const TCHAR *cszFileName);
    void __fastcall ExecCommandLilith(int nCommand, const TCHAR *cszFileName);
    void __fastcall ExecCommandFoobar(int nCommand, const TCHAR *cszFileName);
    BOOL __fastcall RequestCommandKbmplay(const TCHAR *cszCommand, TCHAR **ppszReturn);
    BOOL __fastcall RequestCommandLilith(const TCHAR *cszCommand, TCHAR **ppszReturn);
public:
    CPlayerControl(void);
    CPlayerControl(const TCHAR *cszPlayerPath, int nPlayerType){
        SetPlayer(cszPlayerPath, nPlayerType);
    }
    ~CPlayerControl(void);
    void __fastcall SetPlayer(const TCHAR *cszPlayerPath, int nPlayerType);
    BOOL __fastcall SupportNoAddPlay(void){return SupportCommand(COMMAND_NOADDPLAY);}
    BOOL __fastcall SupportNext(void){return SupportCommand(COMMAND_NEXT);}
    BOOL __fastcall SupportPrev(void){return SupportCommand(COMMAND_PREV);}
    BOOL __fastcall SupportStop(void){return SupportCommand(COMMAND_STOP);}
    BOOL __fastcall SupportQuit(void){return SupportCommand(COMMAND_QUIT);}
    BOOL __fastcall SupportClose(void){return SupportCommand(COMMAND_CLOSE);}
    BOOL __fastcall SupportClear(void){return SupportCommand(COMMAND_CLEAR);}
    BOOL __fastcall SupportAdd(void){return SupportCommand(COMMAND_ADD);}
    void __fastcall Play(const TCHAR *cszFileName, BOOL bAdd);
    void __fastcall Stop(void){ExecCommand(COMMAND_STOP, NULL);}
    void __fastcall FileClose(void){ExecCommand(COMMAND_CLOSE, NULL);}
    void __fastcall Next(void){ExecCommand(COMMAND_NEXT, NULL);}
    void __fastcall Prev(void){ExecCommand(COMMAND_PREV, NULL);}
    void __fastcall Quit(void){ExecCommand(COMMAND_QUIT, NULL);}
    void __fastcall Clear(void){ExecCommand(COMMAND_CLEAR, NULL);}
    void __fastcall Add(const TCHAR *cszFileName){ExecCommand(COMMAND_ADD, cszFileName);}
    BOOL __fastcall GetPlayingFileName(TCHAR *pszFileName, int nSize);//nSize は文字数
};

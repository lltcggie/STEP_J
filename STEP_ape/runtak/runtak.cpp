#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "tak.h"

//tak_deco_lib.dll を使用して演奏時間とフォーマット情報を取得
//64bit版 STEP_ape.ste から呼び出される
//tak_deco_lib.dll の 64bit 版が公開されたら不要になる
//
//ソリューションプラットフォームが x86 の場合はビルドされない
//(STEP_ape.ste 自身で取得するので)
//ソリューションプラットフォームが x64 の場合は
//このプログラム自体は 32bit でビルドされるように設定してある
//
//
int WINAPI _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{//コマンドラインの第1引数はTAKファイル名
 //コマンドラインの第2引数は情報を格納するファイルマップオブジェクト名
    if(__argc < 3){
        return 0;
    }
    LPCTSTR cszFileName = __targv[1];
    LPCTSTR cszFileMapName = __targv[2];
    GetAudioFormatTakToFileMap(cszFileName, cszFileMapName);
    return 0;
}

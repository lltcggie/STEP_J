@echo off
echo VC++ で作ったプログラムの作業ファイル一式を削除するバッチファイル
echo バッチファイルを実行します
pause

del %~dp0*.ilk /s
del %~dp0*.iobj /s
del %~dp0*.ipdb /s
del %~dp0*.pch /s
del %~dp0*.pdb /s
del %~dp0*.bsc /s
del %~dp0*.idb /s
del %~dp0*.sbr /s
del %~dp0*.aps /s
del %~dp0*.plg /s
del %~dp0*.ncb /s
del %~dp0*.exp /s
del %~dp0*.map /s
del %~dp0*.opt /s
del %~dp0*.sdf /s
del %~dp0*.log /s
del %~dp0*.tlog /s
del %~dp0*.obj /s
del %~dp0*.suo /s
del %~dp0*.user /s
del %~dp0*.vc.db /s

rem del %~dp0*.filters /s 削除してはいけない

rem ディレクトリ削除
rmdir /s %~dp0SuperTagEditor\_temp /q
rmdir /s %~dp0SuperTagEditor\.vs /q
rmdir /s %~dp0SuperTagEditor\ipch /q

echo バイナリを削除します。設定ファイルも削除されます。
pause
rmdir /s %~dp0SuperTagEditor\_bin /q

#pragma once

bool ConvExt(FILE_INFO* pFileMP3);
bool DeleteTagID3v1(const TCHAR *sFileName, HWND hWnd);
bool DeleteTagID3v2(const TCHAR *sFileName, HWND hWnd);
bool DeleteTagSIF(const TCHAR* sFileName, HWND hWnd);
bool MakeTagID3v2(const TCHAR *sFileName, HWND hWnd);
bool MakeTagSIF(const TCHAR* sFileName, HWND hWnd);
bool ConvFileFormat(FILE_INFO* pFileMP3, UINT nType, HWND hWnd);
bool WINAPI ConvFileFormatID3v1(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd);
bool WINAPI ConvFileFormatRMP(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd);

//bool WINAPI ConvFileFormatID3v2(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd);//ConvID3v2Version Ç∆ìùçá
bool WINAPI DeleteId3tagv1(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd);
bool WINAPI DeleteId3tagv2(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd);
bool WINAPI ConvID3v2Version(FILE_INFO* pFileInfo, int nProcFlag, HWND hWnd);

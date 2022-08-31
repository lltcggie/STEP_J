// STEP_dsf.h : STEP_dsf DLL のメイン ヘッダー ファイル
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル

#define TYPE_IS_DSF(nFormat) (nFormat == nFileTypeDSF)
#define TYPE_IS_SUPPORT(nFormat) (TYPE_IS_DSF(nFormat))

// CSTEPdsfApp
// このクラスの実装に関しては STEP_dsf.cpp をご覧ください
//
#pragma pack(push, 4)
// DSD chunk構造体
typedef struct
{
	char cHeader[4];					// HEADER(ASC II"DSD ")
	UINT64	uiChunkSize;				// Chunk Size(28Byte)
	UINT64	uiTotalFileSize;			// ファイルサイズ
	UINT64	uiPointerToMetadataChunk;	// メタデータ(ID3v2タグ)位置
}STDSDCHUNK,*PSTDSDCHUNK;

// FMT chunk構造体
typedef struct
{
	char cHeader[4];					// HEADER(ASC II"fmt ")
	UINT64	uiChunkSize;				// Chunk Size(52Byte)
	ULONG	ulFormatVersion;			// Format Version(1)
	ULONG	uiFormatID;					// Format ID(0:DSD raw)
	ULONG	ulChannelType;				// Channelタイプ
	ULONG	ulChannelNum;				// Channel数
	ULONG	ulSamplingFrequency;		// サンプリング周波数(Hz) 2822400, 5644800,・・・
	ULONG	ulBitsPerSample;			// Bits Per Sample(1 or 8)
	UINT64	uiSampleCount;				// Sample count ※1chのサンプリング周波数 × n秒
	ULONG	ulBlockSizePerChannel;		// Block size per channel(4096)
	ULONG	ulReserved;					// 予備
}STFMTCHUNK,*PSTFMTCHUNK;

typedef struct
{
	STDSDCHUNK stDsdChunk;
	STFMTCHUNK stFmtChunk;
}STDSFHEADER, *PSTDSFHEADER;
#pragma pack(pop)

class CSTEPdsfApp : public CWinApp
{
public:
	CSTEPdsfApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

#include "stdafx.h"
#include <windows.h>
#include <mmreg.h>
#include <time.h>
#include "id3tag.h"
#include "crc32.h"
#include "ttadec.h"
#include "STEPlugin.h"

#include "Id3tagv1.h"
#include "Id3tagv2.h"

#include "FileTTA.h"

extern bool bOptID3TagAutoWrite;
extern bool bOptID3TagAutoDelete;

//HANDLE heap;
#define	heap GetProcessHeap()

static void get_id3v1_tag (tta_info *ttainfo);
static void get_id3v2_tag (tta_info *ttainfo);
static bool del_id3v1_tag (tta_info *ttainfo);
//static void del_id3v2_tag (tta_info *ttainfo);

static int open_tta_file (const TCHAR *filename, tta_info *ttainfo) {
	tta_hdr ttahdr;
	unsigned long checksum;
	unsigned long datasize;
	unsigned long origsize;
	DWORD result;

	// clear the memory
	ZeroMemory (ttainfo, sizeof(tta_info));

	ttainfo->HFILE = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (ttainfo->HFILE == INVALID_HANDLE_VALUE) {
		ttainfo->STATE = OPEN_ERROR;
		return -1;
	}

	// get file size
	ttainfo->FILESIZE = GetFileSize(ttainfo->HFILE, NULL);
	_tcsncpy_s(ttainfo->filename, filename, _TRUNCATE);

	// ID3V1 support
	get_id3v1_tag(ttainfo);

	// ID3V2 minimal support
	get_id3v2_tag(ttainfo);

	// read TTA header
	if (!ReadFile(ttainfo->HFILE, &ttahdr, sizeof(tta_hdr), &result, NULL) ||
		result != sizeof(tta_hdr)) {
		CloseHandle(ttainfo->HFILE);
		ttainfo->STATE = READ_ERROR;
		return -1;
	}

	// check for TTA3 signature
	if (ttahdr.TTAid != TTA1_SIGN) {
		CloseHandle(ttainfo->HFILE);
		ttainfo->STATE = FORMAT_ERROR;
		return -1;
	}

	checksum = crc32((unsigned char *) &ttahdr,
	sizeof(tta_hdr) - sizeof(long));
	if (checksum != ttahdr.CRC32) {
		CloseHandle(ttainfo->HFILE);
		ttainfo->STATE = FILE_ERROR;
		return -1;
	}

	// check for player supported formats
	if ((ttahdr.AudioFormat != WAVE_FORMAT_PCM &&
		ttahdr.AudioFormat != WAVE_FORMAT_IEEE_FLOAT &&
		ttahdr.AudioFormat != WAVE_FORMAT_EXTENSIBLE) ||
		ttahdr.BitsPerSample > MAX_BPS ||
		ttahdr.NumChannels > MAX_NCH) {
		CloseHandle(ttainfo->HFILE);
		ttainfo->STATE = PLAYER_ERROR;
		return -1;
	}
	CloseHandle(ttainfo->HFILE);

	// fill the File Info
	ttainfo->NCH = ttahdr.NumChannels;
	ttainfo->BPS = ttahdr.BitsPerSample;
	ttainfo->BSIZE = (ttahdr.BitsPerSample + 7)/8;
	ttainfo->FORMAT = ttahdr.AudioFormat;
	ttainfo->SAMPLERATE = ttahdr.SampleRate;
	ttainfo->DATALENGTH = ttahdr.DataLength;
	ttainfo->FRAMELEN = (long) (FRAME_TIME * ttahdr.SampleRate);
	ttainfo->LENGTH = ttahdr.DataLength / ttahdr.SampleRate * 1000;

	datasize = ttainfo->FILESIZE - ttainfo->id3v2.size;
	origsize = ttainfo->DATALENGTH * ttainfo->BSIZE * ttainfo->NCH;

	ttainfo->COMPRESS = (float) datasize / origsize;
	ttainfo->BITRATE = (long) ((ttainfo->COMPRESS *
		ttainfo->SAMPLERATE * ttainfo->NCH * ttainfo->BPS) / 1000);

	return 0;
}

static void get_id3v1_tag (tta_info *ttainfo) {
	SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
}

static char *unwrap (char *str, int length) {
	char *ptr = str, *e = str + length;
	for (;ptr < e; ptr++) {
		if (*ptr == '\r' && *(ptr + 1) == '\n') {
			MoveMemory(ptr, ptr+1, e - ptr);
			if (*ptr == '\n') *ptr = ' ';
		}
	}
	return str;
}

static bool del_id3v1_tag (tta_info *ttainfo) {
	HANDLE hFile;

	if (!ttainfo->id3v1.id3has) return true;

	// delete ID3V1 tag
	hFile = CreateFile(ttainfo->filename, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		//tta_error(OPEN_ERROR, ttainfo->filename);
		return false;
	}

	SetFilePointer(hFile, -(int) sizeof(id3v1_tag), NULL, FILE_END);
	SetEndOfFile(hFile);
	CloseHandle(hFile);

	ttainfo->id3v1.id3has = 0;
	return true;
}

/***********************************************************************
 * ID3 tags manipulation routines
 *
 * Provides read/write access to ID3v1 tags v1.1
 * Provides read/write access to ID3v2 tags v2.3.x and above
 * Supported ID3v2 frames: Title, Artist, Album, Track, Year,
 *                         Genre, Comment.
 *
 **********************************************************************/

static void pack_sint28 (unsigned int value, char *ptr) {
	ptr[0] = (value >> 21) & 0x7f;
	ptr[1] = (value >> 14) & 0x7f;
	ptr[2] = (value >>  7) & 0x7f;
	ptr[3] = (value & 0x7f);
}

static unsigned int unpack_sint28 (const char *ptr) {
	unsigned int value = 0;

	if (ptr[0] & 0x80) return 0;

	value =  value       | (ptr[0] & 0x7f);
	value = (value << 7) | (ptr[1] & 0x7f);
	value = (value << 7) | (ptr[2] & 0x7f);
	value = (value << 7) | (ptr[3] & 0x7f);

	return value;
}

static void pack_sint32 (unsigned int value, char *ptr) {
	ptr[0] = (value >> 24) & 0xff;
	ptr[1] = (value >> 16) & 0xff;
	ptr[2] = (value >>  8) & 0xff;
	ptr[3] = (value & 0xff);
}

static unsigned int unpack_sint32 (const char *ptr) {
	unsigned int value = 0;

	if (ptr[0] & 0x80) return 0;

	value = (value << 8) | ptr[0];
	value = (value << 8) | ptr[1];
	value = (value << 8) | ptr[2];
	value = (value << 8) | ptr[3];

	return value;
}

static int get_frame_id (const char *id) {
	if (!memcmp(id, "TIT2", 4)) return TIT2;	// Title
	if (!memcmp(id, "TPE1", 4)) return TPE1;	// Artist
	if (!memcmp(id, "TALB", 4)) return TALB;	// Album
	if (!memcmp(id, "TRCK", 4)) return TRCK;	// Track
	if (!memcmp(id, "TYER", 4)) return TYER;	// Year
	if (!memcmp(id, "TCON", 4)) return TCON;	// Genre
	if (!memcmp(id, "COMM", 4)) return COMM;	// Comment
	return 0;
}

static void get_id3v2_tag (tta_info *ttainfo) {
	id3v2_tag id3v2;
	ULONG result;
	int id3v2_size;

	if (!ReadFile(ttainfo->HFILE, &id3v2, sizeof(id3v2_tag), &result, NULL) ||
		result != sizeof(id3v2_tag) || memcmp(id3v2.id, "ID3", 3)) {
		SetFilePointer(ttainfo->HFILE, 0, NULL, FILE_BEGIN);
		return;
	}

	id3v2_size = unpack_sint28((const char*)id3v2.size) + 10;

	if (id3v2.flags & ID3_FOOTERPRESENT_FLAG) id3v2_size += 10;
	SetFilePointer(ttainfo->HFILE, id3v2_size, NULL, FILE_BEGIN);
	ttainfo->id3v2.size = id3v2_size;

	ttainfo->id3v2.id3has = 1;
}


bool LoadFileTTA(FILE_INFO *pFileMP3)
{
	CId3tagv1 id3v1;
    if (id3v1.Load(GetFullPath(pFileMP3)) == ERROR_SUCCESS && id3v1.IsEnable()) {
		SetTrackName(pFileMP3, id3v1.GetTitle());
		SetArtistName(pFileMP3, id3v1.GetArtist());
		SetAlbumName(pFileMP3, id3v1.GetAlbum());
		SetYear(pFileMP3, id3v1.GetYear());
		SetGenre(pFileMP3, STEPGetGenreNameSIF(id3v1.GetGenreNum()));
		SetComment(pFileMP3, id3v1.GetComment());
		if (id3v1.GetTrackNoNum() > 0) {
			SetBTrackNumber(pFileMP3, id3v1.GetTrackNoNum());
		}
	}

	CId3tagv2 id3v2;
	if (id3v2.Load(GetFullPath(pFileMP3)) == ERROR_SUCCESS && id3v2.IsEnable()) {
        SetTrackNameSI(pFileMP3, id3v2.GetTitle());     // タイトル
        SetArtistNameSI(pFileMP3, id3v2.GetArtist());   // アーティスト名
        SetAlbumNameSI(pFileMP3, id3v2.GetAlbum());     // アルバム名
        SetYearSI(pFileMP3, id3v2.GetYear());           // リリース
        SetCommentSI(pFileMP3, id3v2.GetComment());     // コメント
        SetGenreSI(pFileMP3, id3v2.GetGenre());         // ジャンル名

        SetSoftwareSI(pFileMP3, id3v2.GetEncoder());        // エンコーダ
        SetCopyrightSI(pFileMP3, id3v2.GetCopyright());     // 著作権
        SetWriterSI(pFileMP3, id3v2.GetWriter());           // 作詞
        SetComposerSI(pFileMP3, id3v2.GetComposer());       // 作曲
        SetAlbumArtistSI(pFileMP3, id3v2.GetAlbumArtist()); // Albm. アーティスト
        SetOrigArtistSI(pFileMP3, id3v2.GetOrigArtist());   // Orig.アーティスト
        SetURLSI(pFileMP3, id3v2.GetUrl());                 // URL
        SetEncodest(pFileMP3, id3v2.GetEncodedBy());        // エンコードした人
        SetEngineerSI(pFileMP3, id3v2.GetEngineer());       // エンジニア（出版）
        {//トラック番号
         // x/y 形式になっていれば y の方をトラック数として処理
            CString strTrackNo = id3v2.GetTrackNo();
            TCHAR *trk_number = strTrackNo.GetBuffer();
            TCHAR *trk_total = _tcschr(trk_number, _T('/'));
            if(trk_total){
                *trk_total++ = 0;
            }
            SetTrackNumberSI(pFileMP3, trk_number);
            if(trk_total){
                SetTrackTotalSI(pFileMP3, trk_total);
            }
            strTrackNo.ReleaseBuffer();
        }
        {//ディスク番号
         //x/y 形式になっていれば y の方をディスク数として処理
            CString strDiscNo = id3v2.GetDiscNo();
            TCHAR *disc_number = strDiscNo.GetBuffer();
            TCHAR *disc_total = _tcschr(disc_number, _T('/'));
            if(disc_total){
                *disc_total++ = 0;
            }
            SetDiscNumberSI(pFileMP3, disc_number);
            if(disc_total){
                SetDiscTotalSI(pFileMP3, disc_total);
            }
            strDiscNo.ReleaseBuffer();
        }
	} 
    else if (id3v1.IsEnable()) {
		SetTrackNameSI(pFileMP3, id3v1.GetTitle());
		SetArtistNameSI(pFileMP3, id3v1.GetArtist());
		SetAlbumNameSI(pFileMP3, id3v1.GetAlbum());
		SetYearSI(pFileMP3, id3v1.GetYear());
		SetGenreSI(pFileMP3, id3v1.GetGenre());
		SetCommentSI(pFileMP3, id3v1.GetComment());
		SetTrackNumberSI(pFileMP3, id3v1.GetTrackNo());
	}

	tta_info ttainfo;
	if (open_tta_file(GetFullPath(pFileMP3), &ttainfo) == 0) {
		CString strFormat;
		strFormat.Format(_T("True Audio, %dkbps, %dHz, %dbit, %dch, %.02f %%"), 
                         ttainfo.BITRATE, ttainfo.SAMPLERATE, ttainfo.BPS, ttainfo.NCH, ttainfo.COMPRESS);
		SetAudioFormat(pFileMP3, strFormat);
		SetPlayTime(pFileMP3, ttainfo.LENGTH/1000);
	} else {
		SetAudioFormat(pFileMP3, _T("Unknown"));
	} 
    return true;
}

bool WriteFileTTA(FILE_INFO *pFileMP3)
{
	CId3tagv2 id3v2;
	if (id3v2.Load(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
		if (!id3v2.IsEnable()) {
			if (id3v2.MakeTag(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
				return false;
			}
		}
	}
    id3v2.SetUnSynchronization(false);//常に非同期化無効
    if(id3v2.GetCharEncoding() == CId3tagv2::ID3V2CHARENCODING_ISO_8859_1){
    //文字エンコードは基本的に変更しないが、ISO_8859_1 のときは UTF_16 に変更する
        id3v2.SetCharEncoding(CId3tagv2::ID3V2CHARENCODING_UTF_16);
    }

    id3v2.SetTitle(GetTrackNameSI(pFileMP3));           // タイトル
    id3v2.SetArtist(GetArtistNameSI(pFileMP3));         // アーティスト名
    id3v2.SetAlbum(GetAlbumNameSI(pFileMP3));           // アルバム名
    id3v2.SetYear(GetYearSI(pFileMP3));                 // リリース
    id3v2.SetEncoder(GetSoftwareSI(pFileMP3));          // エンコーダ
    {//トラック番号/トラック数
     //トラック数用の ID が ID3v2 にはないので "トラック番号/トラック数" の
     //ような文字列に変換してトラック数に書き込む
        CString strTrackNumber = GetTrackNumberSI(pFileMP3);
        CString strTrackTotal = GetTrackTotalSI(pFileMP3);
        if (!strTrackNumber.IsEmpty() && !strTrackTotal.IsEmpty()) {
            strTrackNumber = strTrackNumber + _T("/") + strTrackTotal;
        }
        id3v2.SetTrackNo(strTrackNumber);
    }
    {//ディスク番号/ディスク数
     //ディスク数用の ID が ID3v2 にはないので "ディスク番号/ディスク数" の
     //ような文字列に変換してディスク数に書き込む
        CString strDiscNumber = GetDiscNumberSI(pFileMP3);
        CString strDiscTotal = GetDiscTotalSI(pFileMP3);
        if (!strDiscNumber.IsEmpty() && !strDiscTotal.IsEmpty()) {
            strDiscNumber = strDiscNumber + _T("/") + strDiscTotal;
        }
        id3v2.SetDiscNo(strDiscNumber);
    }
    id3v2.SetComment(GetCommentSI(pFileMP3));           // コメント
    id3v2.SetCopyright(GetCopyrightSI(pFileMP3));       // 著作権
    id3v2.SetWriter(GetWriterSI(pFileMP3));             // 作詞
    id3v2.SetComposer(GetComposerSI(pFileMP3));         // 作曲
    id3v2.SetAlbumArtist(GetAlbumArtistSI(pFileMP3));   // Albm. アーティスト
    id3v2.SetOrigArtist(GetOrigArtistSI(pFileMP3));     // Orig.アーティスト
    id3v2.SetUrl(GetURLSI(pFileMP3));                   // URL
    id3v2.SetEncodedBy(GetEncodest(pFileMP3));          // エンコードした人
    id3v2.SetEngineer(GetEngineerSI(pFileMP3));         // エンジニア（出版）
    id3v2.SetGenre(GetGenreSI(pFileMP3));

	if (id3v2.Save(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
		return false;
	}

	if (bOptID3TagAutoWrite) {
		STEPConvSiFieldToId3tag(pFileMP3);
	}
	/*
	if (bOptID3TagAutoWrite) {
		SetTrackName(pFileMP3, tagv2.GetTitle());
		SetArtistName(pFileMP3, tagv2.GetArtist());
		SetAlbumName(pFileMP3, tagv2.GetAlbum());
		SetYear(pFileMP3, tagv2.GetYear());
		SetGenre(pFileMP3, tagv2.GetGenre()));
		SetComment(pFileMP3, tagv2.GetComment());
		if (STEPGetIntegerTrackNumber(tagv2.GetTrackNo()) > 0) {
			SetBTrackNumber(pFileMP3, STEPGetIntegerTrackNumber(tagv2.GetTrackNo()));
		}
	}
	*/

	CId3tagv1 id3v1;
	if (id3v1.Load(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
		if  (!bOptID3TagAutoDelete) {
			if (!id3v1.IsEnable()) {
				if (id3v1.MakeTag(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
					return false;
				}
			}
		}
	}
	if  (!bOptID3TagAutoDelete) {
        id3v1.SetTitle(GetTrackName(pFileMP3));
        id3v1.SetArtist(GetArtistName(pFileMP3));
        id3v1.SetAlbum(GetAlbumName(pFileMP3));
        id3v1.SetYear(GetYear(pFileMP3));
        id3v1.SetComment(GetComment(pFileMP3));
        id3v1.SetGenre(STEPGetGenreCode(GetGenre(pFileMP3)));
		if (GetBTrackNumber(pFileMP3) > 0) {
            id3v1.SetTrackNo(GetBTrackNumber(pFileMP3));
		}
		if (id3v1.Save(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
			return false;
		}
	} else if (id3v1.IsEnable()) {
		if (id3v1.DelTag(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
			return false;
		}
		STEPInitDataID3(pFileMP3);
	}

    return true;
}

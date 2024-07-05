#include "stdafx.h"
#include "FileOGG.h"

#include "Tag_Ogg.h"
#include "opus.h"
#include "opusfile.h"
#include "../SuperTagEditor/strcnv.h"

bool LoadFileOGG(FILE_INFO *pFileMP3)
{
    SetFileSpecificInfo(pFileMP3, new CTag_Ogg());
    CTag_Ogg* fileOGG = (CTag_Ogg*)GetFileSpecificInfo(pFileMP3);
    if (fileOGG->Load(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
        return false;
    }

    CString strTrackName;
    CString strArtistName;
    CString strAlbumName;
    CString strAlbumArtist;
    CString strGenre;
    CString strYear;
    CString strComment;
    CString strTrackNumber;
    CString strTrackTotal;
    CString strDiscNumber;
    CString strDiscTotal;
    CString strCopyright;
    CString strLyricist;
    CString strComposer;
    CString strSoftware;
    CString strAlbumSort;
    struct _VORBISCOMMENT_{
        TCHAR *szField;
        CString *pstrContents;
    }VC[] = {
        {_T("TITLE"),       &strTrackName}, //タイトル
        {_T("ARTIST"),      &strArtistName},//アーティスト
        {_T("ALBUM"),       &strAlbumName}, //アルバム
        {_T("ALBUMARTIST"), &strAlbumArtist}, //アルバムアーティスト
        {_T("GENRE"),       &strGenre},     //ジャンル
        {_T("DATE"),        &strYear},      //年
        {_T("COMMENT"),     &strComment},   //コメント
        {_T("TRACKNUMBER"), &strTrackNumber}, //トラック番号
        {_T("TRACKTOTAL"),  &strTrackTotal},  //トラック数
        {_T("DISCNUMBER"),  &strDiscNumber},  //ディスク番号
        {_T("DISCTOTAL"),   &strDiscTotal},   //ディスク数
        {_T("COPYRIGHT"),   &strCopyright},   //著作権
        {_T("LYRICIST"),    &strLyricist},    //作詞者
        {_T("COMPOSER"),    &strComposer},    //作曲者
        {_T("ENCODED BY"),  &strSoftware},    //ソフトウェア
        {_T("ALBUMSORT"),   &strAlbumSort},   //アルバム読み
        {NULL,          NULL}
    };
    int i = 0;
    while(VC[i].szField){
        fileOGG->GetComment(VC[i].szField, 0, *VC[i].pstrContents);
        i++;
    }
    SetTrackNameSI(pFileMP3, strTrackName);
    SetArtistNameSI(pFileMP3, strArtistName);
    SetAlbumNameSI(pFileMP3, strAlbumName);
    SetAlbumArtistSI(pFileMP3, strAlbumArtist);
    SetGenreSI(pFileMP3, strGenre);
    SetYearSI(pFileMP3, strYear);
    SetCommentSI(pFileMP3, strComment);
    SetTrackNumberSI(pFileMP3, strTrackNumber);
    SetTrackTotalSI(pFileMP3, strTrackTotal);
    SetDiscNumberSI(pFileMP3, strDiscNumber);
    SetDiscTotalSI(pFileMP3, strDiscTotal);
    SetCopyrightSI(pFileMP3, strCopyright);
    SetWriterSI(pFileMP3, strLyricist);
    SetComposerSI(pFileMP3, strComposer);
    SetSoftwareSI(pFileMP3, strSoftware);
    SetAlbumSort(pFileMP3, strAlbumSort);

#if 0
    { // 非標準タグ
        CString strDisp;
        CStringArray strArray;
        fileOGG->GetCommentNames(strArray);
        long items = 0;
        int i; for(i=0; i<strArray.GetSize(); i++)
        {
            CString strName = strArray.GetAt(i);
            CString strValue;
            CString _strValue;
            int j = 0;
            while(1)
            {
                if(!fileOGG->GetComment(strName,j++,strValue))
                {
                    break;
                }

                //標準名は除外
                if((j == 1) &&
                    (
                    !strName.Compare(_T("TITLE")) ||
                    !strName.Compare(_T("TRACKNUMBER")) ||
                    !strName.Compare(_T("ARTIST")) ||
                    !strName.Compare(_T("ALBUM")) ||
                    !strName.Compare(_T("DATE")) ||
                    !strName.Compare(_T("GENRE")) ||
                    !strName.Compare(_T("COMMENT")))
                    )
                {
                    continue;
                }
                if (strDisp.IsEmpty()) {
                } else {
                    strDisp += _T("\r\n");
                }
                strDisp += _T("[") + strName + _T("] ") + strValue;
                items++;
            }
        }
        SetOther(pFileMP3, strDisp);
    }
#endif
    SetAudioFormat(pFileMP3, fileOGG->GetAudioFormatString());
    CString strTimeString = fileOGG->GetTimeString();
    TCHAR *time = strTimeString.GetBuffer();
    //szTime = "xx:xx (xxxsec) のようになっている
    TCHAR *pszSec = _tcschr(time, _T('('));
    if(pszSec){
        pszSec++;
        TCHAR *end;
        int sec = _tcstol(pszSec, &end, 10);
        SetPlayTime(pFileMP3, sec);
    }
    strTimeString.ReleaseBuffer();
    return true;
}

bool WriteFileOGG(FILE_INFO *pFileMP3)
{
    CTag_Ogg* fileOGG = (CTag_Ogg*)GetFileSpecificInfo(pFileMP3);

    CString strTrackName = GetTrackNameSI(pFileMP3);
    CString strArtistName = GetArtistNameSI(pFileMP3);
    CString strAlbumName = GetAlbumNameSI(pFileMP3);
    CString strAlbumArtist = GetAlbumArtistSI(pFileMP3);
    CString strGenre = GetGenreSI(pFileMP3);
    CString strYear = GetYearSI(pFileMP3);
    CString strComment = GetCommentSI(pFileMP3);
    CString strTrackNumber = GetTrackNumberSI(pFileMP3);
    CString strTrackTotal = GetTrackTotalSI(pFileMP3);
    CString strDiscNumber = GetDiscNumberSI(pFileMP3);
    CString strDiscTotal = GetDiscTotalSI(pFileMP3);
    CString strCopyright = GetCopyrightSI(pFileMP3);
    CString strLyricist = GetWriterSI(pFileMP3);
    CString strComposer = GetComposerSI(pFileMP3);
    CString strSoftware = GetSoftwareSI(pFileMP3);
    CString strAlbumSort = GetAlbumSort(pFileMP3);
    struct _VORBISCOMMENT_{
        TCHAR *szField;
        CString *pstrContents;
    }VC[] = {
        {_T("TITLE"),       &strTrackName}, //タイトル
        {_T("ARTIST"),      &strArtistName},//アーティスト
        {_T("ALBUM"),       &strAlbumName}, //アルバム
        {_T("ALBUMARTIST"), &strAlbumArtist}, //アルバムアーティスト
        {_T("GENRE"),       &strGenre},     //ジャンル
        {_T("DATE"),        &strYear},      //年
        {_T("COMMENT"),     &strComment},   //コメント
        {_T("TRACKNUMBER"), &strTrackNumber}, //トラック番号
        {_T("TRACKTOTAL"),  &strTrackTotal},  //トラック数
        {_T("DISCNUMBER"),  &strDiscNumber},  //ディスク番号
        {_T("DISCTOTAL"),   &strDiscTotal},   //ディスク数
        {_T("COPYRIGHT"),   &strCopyright},   //著作権
        {_T("LYRICIST"),    &strLyricist},    //作詞者
        {_T("COMPOSER"),    &strComposer},    //作曲者
        {_T("ENCODED BY"),  &strSoftware},    //ソフトウェア
        {_T("ALBUMSORT"),   &strAlbumSort},   //アルバム読み
        {NULL,          NULL}
    };
    int i = 0;
    while(VC[i].szField){
        fileOGG->DelComment(VC[i].szField, 0);
        if (_tcslen(*VC[i].pstrContents) > 0) {
            fileOGG->AddComment(VC[i].szField, *VC[i].pstrContents);
        }
        //fileOGG->ReplaceComment(VC[i].szField, *VC[i].pstrContents, 0);
        i++;
    }
    if (fileOGG->Save(GetFullPath(pFileMP3)) != ERROR_SUCCESS) {
        return false;
    }
    return true;
}

bool LoadFileOPUS(FILE_INFO *pFileMP3)
{
//    return LoadFileOGG(pFileMP3);
    char *fname_utf8 = conv_tstr_to_utf8(GetFullPath(pFileMP3));
    int err = 0;
    OggOpusFile *pOpusFile = op_open_file(fname_utf8, &err);
    free(fname_utf8);
    if(!pOpusFile){
        return false;
    }
    const OpusTags *pTags = op_tags(pOpusFile, -1);
    if(!pTags){
        op_free(pOpusFile);
        return true;
    }
    CString strTrackName;
    CString strArtistName;
    CString strAlbumName;
    CString strAlbumArtist;
    CString strGenre;
    CString strYear;
    CString strComment;
    CString strTrackNumber;
    CString strTrackTotal;
    CString strDiscNumber;
    CString strDiscTotal;
    CString strAlbumSort;
    struct _OPUSCOMMENT_{
        const char *cszField;
        CString *pstrContents;
    }oc[] = {
        {"TITLE",       &strTrackName}, //タイトル
        {"ARTIST",      &strArtistName},//アーティスト
        {"ALBUM",       &strAlbumName}, //アルバム
        {"ALBUMARTIST", &strAlbumArtist}, //アルバムアーティスト
        {"GENRE",       &strGenre},     //ジャンル
        {"DATE",        &strYear},      //年
        {"DESCRIPTION", &strComment},   //コメント(本来がこちらが正しい？)
        {"COMMENT",     &strComment},   //コメント(foobar2000 で書き込むとこちらになる)
        {"TRACKNUMBER", &strTrackNumber}, //トラック番号
        {"TRACKTOTAL",  &strTrackTotal},  //トラック数
        {"DISCNUMBER",  &strDiscNumber},  //ディスク番号
        {"DISCTOTAL",   &strDiscTotal},   //ディスク数
        {"ALBUMSORT",   &strAlbumSort},   //アルバム読み
        {NULL,          NULL}
    };
    int i = 0;
    while(oc[i].cszField){
        const char *value = opus_tags_query(pTags, oc[i].cszField, 0);
        if(value){
            TCHAR *tvalue = conv_utf8_to_tstr(value);
            *oc[i].pstrContents = tvalue;
            free(tvalue);
        }
        i++;
    }
    SetTrackNameSI(pFileMP3, strTrackName);
    SetArtistNameSI(pFileMP3, strArtistName);
    SetAlbumNameSI(pFileMP3, strAlbumName);
    SetAlbumArtistSI(pFileMP3, strAlbumArtist);
    SetGenreSI(pFileMP3, strGenre);
    SetYearSI(pFileMP3, strYear);
    SetCommentSI(pFileMP3, strComment);
    SetTrackNumberSI(pFileMP3, strTrackNumber);
    SetTrackTotalSI(pFileMP3, strTrackTotal);
    SetDiscNumberSI(pFileMP3, strDiscNumber);
    SetDiscTotalSI(pFileMP3, strDiscTotal);
    SetAlbumSort(pFileMP3, strAlbumSort);
    const OpusHead *opusHead = op_head(pOpusFile, -1);
    if(opusHead){
        const int samplerate = opusHead->input_sample_rate;
        const int bitrate = op_bitrate(pOpusFile, -1);
        const int ch = opusHead->channel_count;
        TCHAR format[256];
        _sntprintf_s(format, _TRUNCATE,
                     _T("OggOpus, %dkbps, %dHz(input), %dch"),
                     bitrate/1000, samplerate, ch);
        SetAudioFormat(pFileMP3, format);
    }
    UINT64 qwTotalSample = op_pcm_total(pOpusFile, -1);
    int nTimeSec = qwTotalSample / 48000;//
    SetPlayTime(pFileMP3, nTimeSec);
    op_free(pOpusFile);

    return true;
}
bool WriteFileOPUS(FILE_INFO *pFileMP3)
{
    static DWORD s_dwTime = 0;
    if(GetTickCount() - s_dwTime > 30*1000){
        //前回ダイアログ表示から  30秒以上経過
        MessageBox(NULL, _T("OggOpus のタグ更新は未対応です。"), _T("STEP_ogg"), MB_OK);
        s_dwTime = GetTickCount();
    }
#if 0
    char *fname_utf8 = conv_tstr_to_utf8(GetFullPath(pFileMP3));
    TCHAR szFileName[MAX_PATH*2];
    int err = 0;
    OggOpusFile *pOpusFile = op_open_file(fname_utf8, &err);
    free(fname_utf8);
    if(!pOpusFile){
        return false;
    }
    const OpusTags *pTags = op_tags(pOpusFile, -1);
    OpusTags TagsWrite;
    opus_tags_init(&TagsWrite);
    if(!pTags){
        return true;
    }
    if(pTags){
        opus_tags_copy(&TagsWrite, pTags);
    }
    CString strTrackName = GetTrackNameSI(pFileMP3);
    CString strArtistName = GetArtistNameSI(pFileMP3);
    CString strAlbumName = GetAlbumNameSI(pFileMP3);
    CString strAlbumArtist = GetAlbumArtistSI(pFileMP3);
    CString strGenre = GetGenreSI(pFileMP3);
    CString strYear = GetYearSI(pFileMP3);
    CString strComment = GetCommentSI(pFileMP3);
    CString strTrackNumber = GetTrackNumberSI(pFileMP3);
    CString strTrackTotal = GetTrackTotalSI(pFileMP3);
    CString strDiscNumber = GetDiscNumberSI(pFileMP3);
    CString strDiscTotal = GetDiscTotalSI(pFileMP3);
    CString strAlbumSort = GetAlbumSort(pFileMP3);

    struct _OPUSCOMMENT_{
        const char *cszField;
        CString *pstrContents;
    }oc[] = {
        {"TITLE",       &strTrackName}, //タイトル
        {"ARTIST",      &strArtistName},//アーティスト
        {"ALBUM",       &strAlbumName}, //アルバム
        {"ALBUMARTIST", &strAlbumArtist}, //アルバムアーティスト
        {"GENRE",       &strGenre},     //ジャンル
        {"DATE",        &strYear},      //年
        {"DESCRIPTION", &strComment},   //コメント
        {"TRACKNUMBER", &strTrackNumber}, //トラック番号
        {"TRACKTOTAL",  &strTrackTotal},  //トラック数
        {"DISCNUMBER",  &strDiscNumber},  //ディスク番号
        {"DISCTOTAL",   &strDiscTotal},   //ディスク数
        {"ALBUMSORT",   &strAlbumSort},   //アルバム読み
        {NULL,          NULL}
    };
    int i = 0;
    while(oc[i].cszField){
        char *value_utf8 = conv_tstr_to_utf8(*oc[i].pstrContents);
        opus_tags_add(&TagsWrite, oc[i].cszField, value_utf8);
        i++;
    }
    opus_tags_clear(&TagsWrite);
    op_free(pOpusFile);
#endif
    return true;
}

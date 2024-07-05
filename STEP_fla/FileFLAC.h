#ifndef _FILEFLAC_H_
#define _FILEFLAC_H_

#include "STEPlugin.h"

#include <stdio.h>
#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "flac/all.h"

enum{
    FLA_TITLE,      //トラック名
    FLA_ARTIST,     //アルバム名
    FLA_ALBUMARTIST,//Albm.アーティスト
    FLA_ALBUM,      //アルバム名
    FLA_YEAR,       //寝号
    FLA_TRACKNUMBER,//TrackNo
    FLA_TRACKTOTAL, //Track数
    FLA_DISCNUMBER, //DiscNo
    FLA_DISCTOTAL,  //Disc数
    FLA_GENRE,      //ジャンル
    FLA_COMMENT,    //コメント
    FLA_COPYRIGHT,  //著作権
    FLA_LYRICIST,   //作詞者
    FLA_COMPOSER,   //作曲者
    FLA_PERFORMER,  //演奏者
    FLA_ENCODEDBY,  //ソフトウェア
    FLA_ALBUMSORT,  //アルバム読み
    FLA_ALBUMARTISTSORT, //Albm.アーティスト読み
    FLA_ARTISTSORT, //アーティスト読み
    FLA_LAST
};

typedef struct _File_Tag File_Tag;
struct _File_Tag
{
    unsigned int key;             /* Incremented value */
    boolean saved;        /* Set to TRUE if this tag had been saved */
#if 0
    TCHAR *title;          /* Title of track */
    TCHAR *artist;         /* Artist name */
    TCHAR *album;          /* Album name */
    TCHAR *year;           /* Year of track */
    TCHAR *track;          /* Position of track in the album */
    TCHAR *track_total;    /* The number of tracks for the album (ex: 12/20) */
    TCHAR *str_track;      /* For write tag (ex. 12/20) */
    TCHAR *genre;          /* Genre of song */
    TCHAR *comment;        /* Comment */
    TCHAR *composer;       /* Composer */
    TCHAR *performer;      /* Performer */
#endif
    TCHAR *values[FLA_LAST];
    CPtrArray *other;     /* List of unsupported fields (used for ogg only) */

    int samplerate;       /* Samplerate (Hz) */
    int bps;
    int channels;         /* Stereo, ... or channels for ogg */
    int size;             /* The size of file (in bytes) */
    int duration;         /* The duration of file (in seconds) */
    int bitrate;          /* Bitrate (kb/s) */
};

const char *flac_error_msg;

#ifndef MAX
#    define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#    define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

typedef enum
{
    FMT_U8, FMT_S8, FMT_U16_LE, FMT_U16_BE, FMT_U16_NE, FMT_S16_LE, FMT_S16_BE, FMT_S16_NE, FMT_S24_NE
}
AFormat;

typedef struct {
    FLAC__bool abort_flag;
    FLAC__bool is_playing;
    FLAC__bool eof;
    FLAC__bool play_thread_open; /* if true, is_playing must also be true */
    unsigned total_samples;
    unsigned bits_per_sample;
    unsigned channels;
    unsigned sample_rate;
    unsigned length_in_msec;
    AFormat sample_format; // Note : defined in XMMS devel
    int seek_to_in_sec;
} file_info_struct;

#endif /* _FILEFLAC_H_ */

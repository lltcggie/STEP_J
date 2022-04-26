#include "StdAfx.h"
#include "mpc_format.h"

#include "musepack/libmpcdec/decoder.h"
#include "musepack/libmpcdec/internal.h"
#include "mpc/mpcdec.h"

///////////////////////////////////////////////////////////////////////////
static mpc_int32_t mpc_read(mpc_reader *p_reader, void *ptr, mpc_int32_t size)
{/// Reads size bytes of data into buffer at ptr.
    FILE *fp = (FILE*)p_reader->data;
    return fread(ptr, 1, size, fp);
}
///////////////////////////////////////////////////////////////////////////
static mpc_bool_t mpc_seek(mpc_reader *p_reader, mpc_int32_t offset)
{/// Seeks to byte position offset.
    FILE *fp = (FILE*)p_reader->data;
    return fseek(fp, offset, SEEK_SET) == 0;
}
///////////////////////////////////////////////////////////////////////////
static mpc_int32_t mpc_tell(mpc_reader *p_reader)
{/// Returns the current byte offset in the stream.
    FILE *fp = (FILE*)p_reader->data;
    return ftell(fp);
}
///////////////////////////////////////////////////////////////////////////
static mpc_int32_t mpc_get_size(mpc_reader *p_reader)
{/// Returns the total length of the source stream, in bytes.
    FILE *fp = (FILE*)p_reader->data;
    __int64 cur_pos = _ftelli64(fp);
    fseek(fp, 0, SEEK_END);
    __int64 size = _ftelli64(fp);
    _fseeki64(fp, cur_pos, SEEK_SET);
    if(size > 0x7FFFFFFF){
        size = 0;
    }
    return size;
}
///////////////////////////////////////////////////////////////////////////
static mpc_bool_t mpc_canseek(mpc_reader *p_reader)
{
    return MPC_TRUE;
}

void SetAudioFormatMpc(FILE_INFO *pFileMP3)
{
    FILE *fp;
    if(_tfopen_s(&fp, GetFullPath(pFileMP3), _T("rb")) != 0){
        return;
    }
    mpc_reader reader;
    reader.read = mpc_read;
    reader.seek = mpc_seek;
    reader.tell = mpc_tell;
    reader.get_size = mpc_get_size;
    reader.canseek = mpc_canseek;
    reader.data = fp;
	mpc_demux *demux = mpc_demux_init(&reader);
    if(!demux){
        fclose(fp);
        return;
    }
    int average_bitrate = demux->si.average_bitrate;
    int sample_rate = demux->si.sample_freq;
    int ch = demux->si.channels;
    int length_sec = mpc_streaminfo_get_length(&demux->si);
    TCHAR format[256];
    _sntprintf_s(format, _TRUNCATE, 
#ifdef _UNICODE //demux->si.profile_name ‚Íí‚É ansi (%S ‚Ì‚Ü‚Ü‚¾‚Æ Ansi ”Å‚Å Unicode ‚ÆŒ©‚È‚µ‚Ä‚µ‚Ü‚¤)
                 _T("%dkbps, %dHz, %dch, Profile:%S(SV%d)"), 
#else
                 _T("%dkbps, %dHz, %dch, Profile:%s(SV%d)"), 
#endif
                 average_bitrate/1000, sample_rate, ch, demux->si.profile_name, demux->si.stream_version);
    SetAudioFormat(pFileMP3, format);
    SetPlayTime(pFileMP3, length_sec);
    mpc_demux_exit(demux);
    fclose(fp);
}

/* audiosongps2.c - music streaming state machine (PS2 interface over the GameCube Bink player).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\audiosongps2.c; assembly: build/2049LE/asm/GameSource/src/audiosongps2.s */
#include "rush/rush_protos.h"

static _tStream s_streams;

/* The stream helpers are passed the stream id, which only is_stream_playing names (its DWARF
   drops the unused first parameter); the others are declared without a prototype. */
static void streamparam_init(_tStreamParam *param, int song_id, int is_loop);
static void streamparam_reset(_tStreamParam *param);
static int prepare_stream(_tStreamParam *param);
static int is_stream_ready();
static void start_stream();
static int is_stream_playing(int id, _tStreamParam *param);
static int is_stream_stopped();
static void stop_stream();
static void set_stream_vol();

/* 0x8009e8ac, size 0x54 */
static void streamparam_init(_tStreamParam *param, int song_id, int is_loop)
{
    memset(param, 0, sizeof(*param));
    param->_songID = song_id;
    param->_isLoop = is_loop;
}

/* 0x8009e900, size 0x3c */
static void streamparam_reset(_tStreamParam *param)
{
    memset(param, 0, sizeof(*param));
    param->_songID = -1;
}

/* 0x8009e93c, size 0x60 */
void AudioSongInit(void)
{
    memset(&s_streams, 0, sizeof(s_streams));
    s_streams._state = 0;
    s_streams._streamID = -1;
    streamparam_reset(&s_streams._playingStream);
    streamparam_reset(&s_streams._pendingStream);
}

/* 0x8009e99c, size 0x84 */
void AudioStreamStopAllAndWait(void)
{
    int stopped; /* r29 */
    _tStream *stream; /* r0 */
    stream = &s_streams;
    for (;;) {
        stopped = 0;
        if (stream->_state == 0) {
            stopped = 1;
        } else if (stream->_state != 3) {
            streamparam_reset(&stream->_pendingStream);
            stream->_pendingStream._songID = -2;
        }
        if (stopped) {
            break;
        }
        AudioStreamServer();
        sceGsSyncV(0);
    }
}

/* 0x8009ea20, size 0x50 */
void AudioSongPlay(int song, int loop)
{
    if (!GSettings.bNoStream) {
        streamparam_init(&s_streams._pendingStream, song, loop);
    }
    AudioStreamServer();
    sndTick();
}

/* 0x8009ea70, size 0x58 */
void AudioSongStop(void)
{
    _tStream *stream; /* r0 */
    stream = &s_streams;
    if (stream->_state != 3 && stream->_state != 0) {
        streamparam_reset(&stream->_pendingStream);
        stream->_pendingStream._songID = -2;
    }
    AudioStreamServer();
    sndTick();
}

/* 0x8009eac8, size 0x24 */
int get_song_volume(void)
{
    return 16383.0f * gMusicMasterVol;
}

/* 0x8009eaec, size 0x94 */
static int prepare_stream(_tStreamParam *param)
{
    char buf[64]; /* stack */
    sprintf(buf, "rush2049_data/NGC_MUSIC/%s.bik", gSongNames[param->_songID]);
    if (param->_songID >= 18) {
        sndPlayerMusicPrepare(buf, 36000, 1, 1);
    } else {
        sndPlayerMusicPrepare(buf, 44100, 1, 0);
    }
    return 110;
}

/* 0x8009eb80, size 0x20 */
static int is_stream_ready()
{
    return sndPlayerMusicIsReady();
}

/* 0x8009eba0, size 0x24 */
static void start_stream()
{
    sndPlayerMusicStart(get_song_volume());
}

/* 0x8009ebc4, size 0x3c */
static int is_stream_playing(int id, _tStreamParam *param)
{
    if (param->_isLoop) {
        return 1;
    }
    return !sndPlayerMusicIsEnd();
}

/* 0x8009ec00, size 0x8 */
static int is_stream_stopped()
{
    return 1;
}

/* 0x8009ec08, size 0x20 */
static void stop_stream()
{
    sndPlayerMusicStop();
}

/* 0x8009ec28, size 0x24 */
static void set_stream_vol()
{
    sndPlayerMusicSetVol(get_song_volume());
}

/* 0x8009ec4c, size 0x144 */
void AudioStreamServer(void)
{
    _tStream *stream; /* r0 */
    stream = &s_streams;
    switch (stream->_state) {
    case 0:
        break;
    case 1:
        if (is_stream_ready(stream->_streamID)) {
            start_stream(stream->_streamID);
            stream->_state = 2;
        }
        break;
    case 2:
        if (!is_stream_playing(stream->_streamID, &stream->_playingStream)) {
            stream->_state = 0;
        } else if (gForceUpdateMusic) {
            set_stream_vol(stream->_streamID);
        }
        break;
    case 3:
        if (is_stream_stopped(stream->_streamID)) {
            stream->_state = 0;
        }
        break;
    }
    if (stream->_pendingStream._songID != -1) {
        switch (stream->_state) {
        case 0:
            if (stream->_pendingStream._songID != -2) {
                stream->_playingStream = stream->_pendingStream;
                stream->_streamID = prepare_stream(&stream->_playingStream);
                stream->_state = 1;
            }
            streamparam_reset(&stream->_pendingStream);
            break;
        case 1:
            break;
        case 2:
            stop_stream(stream->_streamID);
            stream->_streamID = -1;
            stream->_state = 3;
            break;
        case 3:
            break;
        }
    }
}


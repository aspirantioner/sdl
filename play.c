#include "lio_camera.h"
#include "lio_soundcard.h"
#include "lio_encoder.h"
#include "format_convert.h"
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <lame/lame.h>

#define WIDTH 1280
#define HEIGHT 720
#define FPSNUM 10

#define SMAPLE_RATE 44100
#define CHANNEL_NUM 2
#define PERIOD 1024

bool flag = true;

const int MP3_SIZE = 2 * sizeof(short) * PERIOD > 7200 ? 2 * sizeof(int) * PERIOD : 7200;
unsigned char *mp3_buffer;

FILE *audio_fp = NULL;
lame_t lame;

void audioCallback(void *userdata, Uint8 *stream, int len)
{
    LioSoundCard *lio_soundcard_p = (LioSoundCard *)userdata;
    int write;
    if (flag)
    {
        LioSoundCardFetchFrame(lio_soundcard_p);
        memcpy(stream, lio_soundcard_p->read_buffer, len);
        write = lame_encode_buffer_interleaved(lame, lio_soundcard_p->read_buffer, PERIOD, mp3_buffer, MP3_SIZE);
    }
    else
    {
        write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        lame_close(lame);
    }
    fwrite(mp3_buffer, write, 1, audio_fp);
}

int main(void)
{
    /*open video device*/
    LioCamera lio_camera;
    LioCameraOpen(&lio_camera, "/dev/video0");
    LioCameraSetFormat(&lio_camera, V4L2_PIX_FMT_YUYV, WIDTH, HEIGHT);
    LioCameraSetFps(&lio_camera, FPSNUM, 1);
    LioCameraBufRequest(&lio_camera, 4);

    /*open voice device*/
    LioSoundCard sound_card_capture;
    LioSoundCardInit(&sound_card_capture, SND_PCM_STREAM_CAPTURE, SMAPLE_RATE, PERIOD, SND_PCM_ACCESS_RW_INTERLEAVED, SND_PCM_FORMAT_S16_LE, CHANNEL_NUM, 2);

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec desiredSpec;
    desiredSpec.callback = audioCallback;
    desiredSpec.userdata = &sound_card_capture;
    desiredSpec.channels = 2;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.freq = 44100;
    desiredSpec.samples = 1024;
    if (SDL_OpenAudio(&desiredSpec, NULL) < 0)
    {
        perror("SDL open error");
        return -1;
    }

    // 初始化SDL库
    SDL_Init(SDL_INIT_VIDEO);
    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("YUV Frame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    // 创建纹理
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    // 锁定纹理
    void *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    LioCameraStartStream(&lio_camera);
    SDL_Event event;

    LioEncoder lio_encoder;
    LioEncoderInit(&lio_encoder, "baseline", WIDTH, HEIGHT, X264_CSP_I420);
    FILE *video_fp = fopen("output.h264", "wb");

    audio_fp = fopen("output.mp3", "wb");
    lame = lame_init();
    lame_set_in_samplerate(lame, 44100);  // 输入采样率
    lame_set_out_samplerate(lame, 44100); // 输出采样率
    lame_set_num_channels(lame, 2);       // 声道数
    lame_set_brate(lame, 128);            // 比特率（以kbps为单位）
    if (lame_init_params(lame) == -1)
    {
        perror("lame set error");
        exit(0);
    };

    // 初始化LAME库
    if (lame_init_params(lame) < 0)
    {
        printf("无法初始化LAME库\n");
        return 1;
    }
    mp3_buffer = malloc(MP3_SIZE);

    SDL_PauseAudio(0);
    while (flag)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                flag = false;
                break;
            }
        }
        yuyv422_to_yuv420(LioCameraFetchStream(&lio_camera), pixels, WIDTH, HEIGHT);
        LioCameraPutStream(&lio_camera);
        LioEncoderOutput(&lio_encoder, pixels, video_fp);
        // 解锁纹理
        SDL_UnlockTexture(texture);
        // 清空渲染器
        SDL_RenderClear(renderer);
        // 将纹理渲染到窗口中
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        // 刷新渲染器
        SDL_RenderPresent(renderer);
    }

    LioCameraStopStream(&lio_camera);
    LioEncoderFlush(&lio_encoder, video_fp);
    LioEncoderDestroy(&lio_encoder);
    LioCameraDestroy(&lio_camera);

    LioSoundCardClose(&sound_card_capture);
    // 释放资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // 退出SDL库
    SDL_Quit();
    free(mp3_buffer);
    exit(0);
}
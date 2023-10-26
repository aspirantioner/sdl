#include "lio_camera.h"
#include "lio_encoder.h"
#include "format_convert.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#define WIDTH 1280
#define HEIGHT 720
#define FPSNUM 10
int main(int argc, char **argv)
{
    LioCamera lio_camera;
    LioCameraOpen(&lio_camera, "/dev/video0");
    LioCameraSetFormat(&lio_camera, V4L2_PIX_FMT_YUYV, WIDTH, HEIGHT);
    LioCameraSetFps(&lio_camera, FPSNUM, 1);
    LioCameraBufRequest(&lio_camera, 4);

    // LioEncoder lio_encoder;
    // LioEncoderInit(&lio_encoder, "baseline", WIDTH, HEIGHT, X264_CSP_I420, FPSNUM);

    FILE *fp = fopen("yu.yuv", "wb");
    LioCameraStartStream(&lio_camera);
    int yuv_buff_size = WIDTH * HEIGHT * 3 / 2;
    unsigned char yuv_buff[yuv_buff_size];
    for (int i = 0; i < 50; i++)
    {
        yuyv422_to_yuv420(LioCameraFetchStream(&lio_camera), yuv_buff, WIDTH, HEIGHT);
        LioCameraPutStream(&lio_camera);
        fwrite(yuv_buff, yuv_buff_size, 1, fp);
        // LioEncoderOutput(&lio_encoder, yuv_buff, fp);
    }
    LioCameraStopStream(&lio_camera);
    // LioEncoderFlush(&lio_encoder, fp);
    // LioEncoderDestroy(&lio_encoder);
    LioCameraDestroy(&lio_camera);
    return 1;
    // // 初始化SDL库
    // SDL_Init(SDL_INIT_VIDEO);
    // // 创建窗口
    // SDL_Window *window = SDL_CreateWindow("YUV Frame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, 0);
    // // 创建渲染器
    // SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    // // 创建纹理
    // SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    // // 锁定纹理
    // void *pixels;
    // int pitch;
    // SDL_LockTexture(texture, NULL, &pixels, &pitch);
    // // LioEncoder lio_encode;
    // // LioEncoderInit(&lio_encode, "high", WIDTH, HEIGHT, X264_CSP_I420);

    // LioCameraStartStream(&lio_camera);
    // SDL_Event event;
    // bool flag = true;
    // while (flag)
    // {
    //     while (SDL_PollEvent(&event))
    //     {
    //         if (event.type == SDL_QUIT)
    //         {
    //             flag = false;
    //             break;
    //         }
    //     }
    //     yuyv422_to_yuv420(LioCameraFetchStream(&lio_camera), pixels, WIDTH, HEIGHT);
    //     // 解锁纹理
    //     SDL_UnlockTexture(texture);
    //     // 清空渲染器
    //     SDL_RenderClear(renderer);
    //     // 将纹理渲染到窗口中
    //     SDL_RenderCopy(renderer, texture, NULL, NULL);
    //     // 刷新渲染器
    //     SDL_RenderPresent(renderer);

    //     LioCameraPutStream(&lio_camera);
    // }
    // LioCameraStopStream(&lio_camera);
    // LioCameraDestroy(&lio_camera);

    // // 释放资源
    // SDL_DestroyTexture(texture);
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);

    // // 退出SDL库
    // SDL_Quit();
    // LioEncoderFlush(&lio_encode, file_p);
    // LioEncoderDestroy(&lio_encode);
}
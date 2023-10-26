#include <stdio.h>
#include <SDL2/SDL.h>

#define FILE_PATH "test.pcm"

void audioCallback(void *userdata, Uint8 *stream, int len)
{
    printf("len is %d\n",len);
    FILE *file = (FILE *)userdata;
    memset(stream, 0, len);
    int ret = fread(stream, 1, len, file);
    printf("%d\n", ret);
    if (ret == 0)
    {
        printf("end\n");
        exit(0);
    }
}

int main()
{
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec desiredSpec;

    FILE *audioFile = fopen(FILE_PATH, "rb");
    if (!audioFile)
    {
        printf("Failed to open audio file: %s\n", FILE_PATH);
        return 1;
    }

    desiredSpec.callback = audioCallback;
    desiredSpec.userdata = audioFile;
    desiredSpec.channels = 2;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.freq = 44100;
    desiredSpec.samples = 1024;
    if (SDL_OpenAudio(&desiredSpec, NULL) < 0)
    {
        perror("SDL open error");
        return -1;
    }
    printf("buff size %d\n", desiredSpec.size);

    SDL_PauseAudio(0);
    while (1)
    {
        SDL_Delay(100);
    }
    SDL_Quit();

    return 0;
}

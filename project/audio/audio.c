#include <stdio.h>
#include <SDL2/SDL.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file.wav>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_AudioSpec wav_spec;
    Uint8 *audio_buf = NULL;
    Uint32 audio_len = 0;

    if (SDL_LoadWAV(argv[1], &wav_spec, &audio_buf, &audio_len) == NULL)
    {
        fprintf(stderr, "Error loading WAV: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("WAV loaded: freq=%d Hz, channels=%d, format=0x%x, bytes=%u\n",
           wav_spec.freq, wav_spec.channels, wav_spec.format, (unsigned)audio_len);

    // Desired = obtained for simplicity (play the file in its exact format)
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
    if (dev == 0)
    {
        fprintf(stderr, "Error opening audio device: %s\n", SDL_GetError());
        SDL_FreeWAV(audio_buf);
        SDL_Quit();
        return 1;
    }

    if (SDL_QueueAudio(dev, audio_buf, audio_len) != 0)
    {
        fprintf(stderr, "Error queuing audio: %s\n", SDL_GetError());
        SDL_CloseAudioDevice(dev);
        SDL_FreeWAV(audio_buf);
        SDL_Quit();
        return 1;
    }

    // Start playback
    SDL_PauseAudioDevice(dev, 0);
    printf("Playback started.\n");

    // Wait until the audio queue is empty
    while (SDL_GetQueuedAudioSize(dev) > 0)
    {
        SDL_Delay(50); // 50 ms polling
    }

    printf("Playback finished.\n");

    SDL_CloseAudioDevice(dev);
    SDL_FreeWAV(audio_buf);
    SDL_Quit();
    return 0;
}
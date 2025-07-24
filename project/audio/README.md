# SDL2 WAV Player

A tiny command‑line program that loads a **WAV** file and plays it once using **SDL2's queueing audio API**. No threads, no callbacks: just `SDL_QueueAudio` + polling until playback finishes.

---

## 1. Features

* Loads a PCM (or ADPCM, etc., anything SDL2 can parse) `.wav` file via `SDL_LoadWAV`.
* Opens the default output device using the *exact* source format (no conversion step).
* Queues the whole buffer (`SDL_QueueAudio`) and starts playback with `SDL_PauseAudioDevice(dev, 0)`.
* Polls `SDL_GetQueuedAudioSize` until the buffer drains, then cleans up.

---

## 2. Build Instructions

### macOS (Homebrew SDL2)

```bash
clang main.c -I"$(brew --prefix)/include" -L"$(brew --prefix)/lib" -lSDL2 -o sdlplay
```

### Linux (Debian/Ubuntu)

```bash
sudo apt install libsdl2-dev
cc main.c -lSDL2 -o sdlplay
```

### Windows (MSYS2 MinGW64)

```bash
pacman -S mingw-w64-x86_64-SDL2
x86_64-w64-mingw32-gcc main.c -lSDL2 -o sdlplay.exe
```

Ensure the SDL2 *development* package is installed (headers + import libs) — not just runtime DLLs.

---

## 3. Usage

```bash
./sdlplay sound.wav
```

If you forget the argument:

```
Usage: ./sdlplay <file.wav>
```

**Typical output:**

```
WAV loaded: freq=44100 Hz, channels=2, format=0x8010, bytes=352800
Playback started.
Playback finished.
```

`format=0x8010` corresponds to `AUDIO_S16LSB` (signed 16‑bit little endian). SDL formats are bit‑field encoded.

---

## 4. Code

### 4.1 Argument Check

```c
if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.wav>\n", argv[0]);
    return 1;
}
```

Minimal validation: exactly one required argument, the path to a `.wav` file.

### 4.2 SDL Audio Subsystem Initialization

```c
if (SDL_Init(SDL_INIT_AUDIO) != 0) {
    fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    return 1;
}
```

Initializes only the audio subsystem (leaner than `SDL_INIT_EVERYTHING`). On failure, you *must* not call other SDL audio functions.

### 4.3 Loading the WAV

```c
SDL_AudioSpec wav_spec;   // Filled by SDL_LoadWAV
Uint8 *audio_buf = NULL;  // Allocated by SDL
Uint32 audio_len = 0;     // Byte length

if (SDL_LoadWAV(argv[1], &wav_spec, &audio_buf, &audio_len) == NULL) {
    fprintf(stderr, "Error loading WAV: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
}
```

`SDL_LoadWAV` parses the RIFF header, allocates a buffer for the PCM (or decoded) data, and populates `wav_spec` with **the original format** (frequency, format enum, channel count, and *optionally* a callback pointer — unused here).

### 4.4 Opening an Audio Device

```c
SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
```

Parameters:

* `NULL`: default output device name (SDL picks one).
* `0`: `iscapture` flag (we want output, not capture).
* `&wav_spec`: *desired* spec. We do **not** supply a callback, so `SDL_QueueAudio` mode is implied.
* `NULL`: we ignore the *obtained* spec so SDL uses the desired format as is (if supported). If not fully supported, SDL might pick a close format unless `SDL_OPENAUDIO_ALLOW_ANY_CHANGE` (flag not used) restricts it.
* `0`: no allow‑changes flags.

If `dev==0`, it's a failure (sound device missing, unsupported format, etc.).

### 4.5 Queueing Audio Data

```c
if (SDL_QueueAudio(dev, audio_buf, audio_len) != 0) { ... }
```

Copies the entire buffer into the device's internal queue (non‑blocking). You can call this multiple times to append more audio.

### 4.6 Start Playback

```c
SDL_PauseAudioDevice(dev, 0); // 0 = unpause (start)
```

Devices start in a paused state. Passing non‑zero would (re)pause.

### 4.7 Polling Until Completion

```c
while (SDL_GetQueuedAudioSize(dev) > 0) {
    SDL_Delay(50);
}
```

`SDL_GetQueuedAudioSize` returns the number of *unconsumed* bytes still buffered. When it reaches zero, playback of queued data is finished.

### 4.8 Cleanup

```c
SDL_CloseAudioDevice(dev);
SDL_FreeWAV(audio_buf); // Frees buffer allocated by SDL_LoadWAV
SDL_Quit();
```

**Order matters:** free the device after playback ends; always free the WAV buffer to avoid leaks.
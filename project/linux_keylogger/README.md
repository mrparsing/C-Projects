# Linux Keyboard Event Listener

A minimal C program that reads **raw key press events** from the Linux *evdev* interface (`/dev/input/event*`) and prints symbolic key names. Includes a `keymap.h` with a sparse lookup table.

---

## 1. Requirements

* Linux system with the *evdev* input subsystem (virtually all modern distros)
* Headers providing `<linux/input.h>` (usually in `linux-libc-dev` / kernel headers)
* A C compiler (GCC or Clang)
* Access permissions to the keyboard device node (root or proper udev group, often `input`)

---


### keylogger.c (core logic)

Key includes:

* `<linux/input.h>`: definitions for `struct input_event`, event types (`EV_KEY`), key codes like `KEY_A`.
* `<glob.h>`: used to locate a stable symlink to the keyboard.
* `<signal.h>`: clean termination on Ctrl+C.

### keymap.h

Sparse array `key_code_names[KEY_MAX_CODE + 1]` with designated initializers at the indexes of interest. Undefined indexes remain `NULL`.

---

## 2. Permissions & Running

Keyboard event devices are typically owned by `root:input` and mode `0600` or `0640`.

Check device(s):

```bash
ls -l /dev/input/by-path/*-event-kbd
```

Run with sudo if necessary:

```bash
sudo ./keylogger
```

Better: add your user to the `input` group (log out/in required):

```bash
sudo usermod -aG input $USER
```

Output example (pressing `A`, `B`, `ENTER`):

```
Uso device: /dev/input/by-path/pci-0000:00:14.0-usb-0:1:1.0-event-kbd
A B ENTER
```


Stop with `Ctrl+C` (SIGINT) — the program installs a handler that closes the file descriptor and exits gracefully.

---

## 3. Detailed Walkthrough

### 3.1 Device Discovery: `find_keyboard_by_path()`

```c
glob("/dev/input/by-path/*-event-kbd", 0, NULL, &g);
```

* Pattern matches symlinks naming the *keyboard* event device (distinguishes from mice, etc.).
* Returns the **first** match. If multiple keyboards exist (internal + USB), you always get the first in the glob order.
* Copies path into a static buffer `result` so its lifetime extends beyond the function.

**Why by-path?** Symlinks in `/dev/input/by-path` are stable across reboots compared to raw `/dev/input/eventX` indices.

### 3.2 Signal Handling: `handle_sigint`

```c
struct sigaction sa = { .sa_handler = handle_sigint, .sa_flags = SA_RESTART };
```

* On `SIGINT` (Ctrl+C), closes `fd` and calls `_exit(EXIT_SUCCESS)`.
* `SA_RESTART` causes interrupted system calls (like `read`) to automatically restart, reducing EINTR handling complexity (you still check `errno == EINTR`).

### 3.3 Opening the Device

```c
fd = open(device, O_RDONLY);
```

* Read-only suffices for listening.
* Could also use `O_NONBLOCK` to avoid blocking on `read`; current code is blocking (simpler).

### 3.4 Event Loop

```c
while (1) {
    ssize_t bytes = read(fd, &ev, sizeof ev);
    ...
    if (ev.type == EV_KEY && ev.value == 1) { /* press */ }
}
```

* Each `read` returns exactly one `struct input_event` (24 bytes on most architectures). The code validates that with `bytes != sizeof ev`.
* `ev.type == EV_KEY` filters only key events.
* `ev.value` semantics for `EV_KEY`:

  * `0` = release
  * `1` = press
  * `2` = auto-repeat (held key)

Current code prints only on **press** (`value == 1`). If you want repeats, include `2`.

### 3.5 Key Code Translation

```c
if (ev.code < KEY_CNT && key_code_names[ev.code])
    printf("%s ", key_code_names[ev.code]);
else
    printf("KEY_%u ", ev.code);
```

* Bounds check (`ev.code < KEY_CNT`) prevents out-of-range array access.
* Sparse array lookup returns `NULL` for codes without a mapped string.
* Fallback prints numeric code.

**`KEY_CNT` vs `KEY_MAX_CODE`:** `KEY_CNT` comes from `<linux/input-event-codes.h>` (included indirectly) and represents the kernel-defined count; `KEY_MAX_CODE` in your header sets array size to a safe constant (0x2ff) larger than current normal range, providing future‑proof slack.

### 3.6 keymap.h Sparse Initialization

```c
const char *key_code_names[KEY_MAX_CODE + 1] = {
    [KEY_ESC] = "ESC",
    [KEY_A]   = "A",
    ...
};
```

* Designated initializers assign only the indexes you care about.
* Unassigned entries default to `NULL` (C static storage zero-initialization), enabling the simple null check.

### 3.7 Exit Paths

* Normal interactive quit: Ctrl+C → handler → `_exit(EXIT_SUCCESS)`.
* Error paths: print diagnostic and `return EXIT_FAILURE` / `break` loop.
* After loop break: `close(fd); return EXIT_FAILURE;` (You might optionally return success depending on context.)

---

## 4. Data Structures: `struct input_event`

From `<linux/input.h>` (simplified):

```c
struct input_event {
    struct timeval time; /* seconds + microseconds timestamp */
    __u16 type;          /* event type (EV_KEY, EV_REL, etc.) */
    __u16 code;          /* key code or axis */
    __s32 value;         /* value (press=1, release=0, repeat=2 for EV_KEY) */
};
```

You ignore `time`, but you could use it for latency or multi-key timing analysis.

---
## 5. Security Note

Reading raw keyboard events can capture passwords typed into terminals. Use responsibly and avoid running such tools on multi-user systems without consent.
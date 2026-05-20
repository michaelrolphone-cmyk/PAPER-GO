# PAPER-GO Arduino IDE Implementation Requirements

This file is the implementation contract for `PAPER-GO` when building with the **Arduino IDE**, not PlatformIO.

The firmware target is the **LILYGO T5S3 4.7-inch E-Paper Pro / H752-01** class board. The project is a handheld e-paper field OS with a launcher, touch apps, GPS, SD storage, radio utilities, weather/cache functions, web server tools, and offline utility/game apps.

This document intentionally avoids PlatformIO-only requirements. Do not require `pio run`, `pio test`, `platformio.ini`, PlatformIO environments, or PlatformIO-only library dependency declarations as the definition of done.

---

## 0. Non-negotiable development rules

These rules apply to every task.

1. Build and test through the Arduino IDE workflow first.
2. Do not satisfy a requirement by adding a stub, fake class, placeholder screen, TODO-only implementation, mock-only behavior, or "not implemented" notice.
3. Do not replace existing working behavior with serial-print-only scaffolding.
4. Do not remove existing features to make a compile error disappear.
5. Do not add PlatformIO as a required build system.
6. Do not require the user to manually edit library source files.
7. Use installed Arduino libraries and real board APIs where available.
8. Hardware-facing requirements must call the real hardware library or real chip interface.
9. Every user-facing app task must connect rendering, touch handling, state, and service behavior.
10. A task is not complete until the acceptance checks for that task pass in Arduino IDE.
11. If a feature cannot be implemented in the current turn, leave the existing behavior intact and document the exact blocking compile/runtime error in the commit notes.
12. Do not mark checkboxes as complete unless the firmware compiles and the feature is reachable on-device.

---

## 1. Arduino IDE build policy

### 1.1 Required build workflow

The project must compile in Arduino IDE using an Arduino sketch entry point.

Required workflow:

1. Open the project as an Arduino sketch.
2. Select an ESP32-S3 board profile compatible with the LILYGO T5S3 4.7-inch e-paper Pro.
3. Enable PSRAM.
4. Use 16 MB flash.
5. Enable USB CDC on boot when needed for serial logs.
6. Build and upload from Arduino IDE.
7. Verify visible screen output on the e-paper display.

The project may keep a `platformio.ini` file for users who want it, but it must not be the required path.

### 1.2 Arduino sketch layout requirement

Codex must ensure the repository contains an Arduino IDE-compatible sketch layout.

Acceptable Arduino layout:

```text
PAPER-GO/
  PAPER-GO.ino
  src/
    BoardHAL.cpp
    BoardHAL.h
    BoardConfig.h
    Services.cpp
    Services.h
    AppManager.cpp
    AppManager.h
    Apps.cpp
    Apps.h
    ...
```

Rules:

1. The root `.ino` file must be named to match the sketch folder or be placed in a properly named Arduino sketch folder.
2. Project headers used by `src/*.cpp` must be available through Arduino's normal include search path.
3. Do not rely on PlatformIO's root `include/` folder behavior.
4. If headers are currently under `include/`, either move them into `src/` or add Arduino-compatible forwarding headers in `src/`.
5. Keep all implementation code in real `.cpp/.h` files where practical.
6. Do not flatten the whole project into one giant `.ino` unless there is no other working Arduino-compatible option.
7. Do not create a second duplicate firmware tree that drifts from the real source.

Acceptance checks:

- Arduino IDE can open the sketch without needing PlatformIO.
- A clean Arduino compile finds all project headers.
- No source file fails because `BoardConfig.h`, `BoardHAL.h`, `Services.h`, `Apps.h`, or any other project header is hidden in a PlatformIO-only include directory.

---

## 2. Library policy for Arduino IDE

### 2.1 Display library rule

The installed display library is **LilyGo-EPD47**.

That library exposes:

```cpp
#include "epd_driver.h"
```

It does **not** expose:

```cpp
#include <epdiy.h>
```

Therefore, for the normal Arduino IDE build, Codex must not include `epdiy.h` unless the repository also vendors the actual `epdiy` source and the Arduino build is explicitly configured to compile it.

Required normal path:

```cpp
#include "epd_driver.h"
```

Use the LilyGo-EPD47 style display API:

```cpp
epd_init();
epd_poweron();
epd_clear();
epd_draw_grayscale_image(epd_full_screen(), framebuffer);
epd_poweroff();
```

Do not call epdiy-only APIs in the Arduino IDE default build:

```cpp
epd_init(&epd_board_v7, &ED047TC1, EPD_LUT_64K);
epd_hl_init(...);
epd_hl_get_framebuffer(...);
epd_hl_update_screen(...);
epd_set_rotation(...);
epd_rotated_display_width();
epd_rotated_display_height();
```

Those are only acceptable if `epdiy` is intentionally vendored and proven to compile under Arduino IDE.

### 2.2 Correct display import task

Files to inspect/change:

- `src/BoardHAL.cpp`
- `src/BoardHAL.h`
- `src/BoardConfig.h`
- root `.ino`
- any display helper files

Tasks:

1. Replace `#include <epdiy.h>` with the correct LilyGo-EPD47 Arduino include:
   ```cpp
   #include "epd_driver.h"
   ```
2. Remove hard dependencies on `EpdiyHighlevelState`, `EPD_BUILTIN_WAVEFORM`, `epd_board_v7`, `ED047TC1`, and `epd_hl_*` functions from the Arduino default path.
3. Allocate a 4-bit grayscale framebuffer using PSRAM:
   ```cpp
   framebuffer = (uint8_t*)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
   ```
4. Preserve the existing `BoardHAL` drawing API:
   - `clear`
   - `fillRect`
   - `drawRect`
   - `drawLine`
   - `drawText`
   - `beginFrame`
   - `endFrame`
5. Implement those functions by drawing into the framebuffer.
6. In `endFrame`, update the physical display using:
   ```cpp
   epd_poweron();
   epd_draw_grayscale_image(epd_full_screen(), framebuffer);
   epd_poweroff();
   ```
7. Do not just print draw calls to Serial.
8. Do not return success if framebuffer allocation fails.
9. Show a boot splash that proves the framebuffer was physically sent to the e-paper panel.

Acceptance checks:

- Arduino IDE compile does not report `epdiy.h: No such file or directory`.
- Arduino IDE compile does not report unknown `epd_hl_*` symbols.
- Uploading the firmware shows a visible boot splash.
- Calls to `BoardHAL::drawText`, `drawRect`, `fillRect`, and `drawLine` visibly change the screen.
- `BoardHAL::endFrame(true)` performs a real display refresh.

### 2.3 Touch library rule

Use the installed SensorLib GT911 driver:

```cpp
#include <TouchDrvGT911.hpp>
```

Do not leave touch as injected sample-only state in the normal board build.

Required behavior:

1. Initialize I2C on the board SDA/SCL pins.
2. Probe GT911 at `0x14` and `0x5D`.
3. Initialize touch using `touch.begin(...)`.
4. Set max coordinates to the e-paper dimensions.
5. Apply the same swap/mirror configuration used by the working LilyGo example unless testing proves different orientation is required.
6. Convert physical touches into the existing `TouchEvent` classifier.
7. Support tap, long press, swipe, and drag.

Acceptance checks:

- Touch startup log reports the detected GT911 address.
- Tapping launcher icons opens apps.
- Swipe down returns home.
- Swipe right performs back where applicable.
- Touch coordinates match the visible screen orientation.

### 2.4 Battery and power library rule

Use the real board battery/charger interfaces where available.

Relevant Arduino libraries may include:

```cpp
#include <XPowersLib.h>
#include "bq27220.h"
```

Required behavior:

1. Stop returning fake battery values.
2. Read battery percent or state-of-charge from BQ27220 when available.
3. Read charging/power state from BQ25896/XPowers when available.
4. Fall back to ADC voltage only when the gauge is not detected.
5. Status bar must show real percent or a clear unavailable state.
6. Charging icon must reflect the charger state, not a hard-coded false value.

Acceptance checks:

- `BoardHAL::battery()` never returns the hard-coded placeholder `{percent=-1, charging=false}` when the hardware responds.
- Serial diagnostics identify whether BQ27220, BQ25896, and ADC fallback are being used.
- Status bar updates after plugging/unplugging USB power.

### 2.5 Required Arduino libraries

The Arduino build must explicitly document and use these libraries only when the code actually includes them:

- LilyGo-EPD47
- SensorLib
- XPowersLib
- Button2 if physical button helper logic uses it
- ArduinoJson
- TinyGPSPlus
- RadioLib
- NimBLE-Arduino
- WebSockets if still used by real code
- ESP32 board package

Do not add a library to the requirements unless a real source file uses it.

Acceptance checks:

- README contains an Arduino IDE "Install these libraries" section.
- The list matches actual includes.
- The firmware compiles after installing those libraries through Arduino Library Manager or documented ZIP/GitHub install steps.

---

## 3. Current repository baseline to preserve

The current project already contains foundations that must not be deleted:

1. `BoardHAL` owns display, touch, button, SD, battery, sleep, and frontlight/backlight boundaries.
2. `SystemServices` ties hardware/services to apps.
3. `AppManager` owns the current app, navigation, status bar, power actions, and redraw decisions.
4. App classes exist for:
   - Springboard
   - Lock Screen
   - GPS Map
   - Radio Scanner
   - Meshtastic
   - URL Fetcher
   - Markdown Reader
   - File Explorer
   - Weather
   - Web Server
   - Games
   - Settings
5. `Services.cpp` owns GPS, Wi-Fi, cache layout, radio scanning, and web server behavior.
6. Logic/helper files should stay separated from hardware-specific code where possible.
7. Existing pure logic tests may remain, but they must not define the build workflow.

Incomplete areas that must be implemented rather than hidden:

1. `BoardHAL::pollTouch()` must read the real GT911 controller.
2. `BoardHAL::battery()` must read real battery/charge state.
3. `GpsMapApp` must render actual cached/offline map content or a real local map tile format, not only a placeholder box.
4. `MeshtasticApp` must become an actual messaging/node UI or be clearly scoped as LoRa packet messaging if full Meshtastic compatibility is not implemented.
5. `GamesApp` must open playable game screens.
6. `SettingsApp` must support actual editing of Wi-Fi, power, display, GPS, LoRa, cache, and web server settings.
7. All UI coordinates must fit the real 960x540 panel.
8. The code must compile under Arduino IDE without PlatformIO include path behavior.

---

## 4. Definition of done for Arduino IDE

A feature is done only when all of these are true:

1. Arduino IDE build succeeds from a clean checkout after documented library installation.
2. No compile error references missing `epdiy.h`.
3. No compile error is fixed by deleting a feature.
4. The firmware uploads to the board.
5. The e-paper screen visibly changes.
6. Touch input works on the real GT911 panel.
7. SD card mounts and creates the required folder structure.
8. GPS, Wi-Fi, LoRa, battery, and RTC failures are handled visibly without crashing.
9. UI drawing stays inside the 960x540 display bounds.
10. Features are reachable through touch UI or documented web endpoints.
11. New pure logic has tests where practical, but tests are not a substitute for real on-device behavior.
12. Documentation describes Arduino IDE setup, not PlatformIO as the required path.

---

# Milestone A — Convert the project to a clean Arduino IDE build

## A1. Add/repair the Arduino sketch entry point

Files to inspect/change:

- root `.ino`
- `src/main.cpp`
- `src/*.h`
- `src/*.cpp`
- old `include/*.h`

Tasks:

1. Ensure there is a single Arduino sketch entry point with `setup()` and `loop()`.
2. If `src/main.cpp` already contains `setup()` and `loop()`, either:
   - rename/wrap it so Arduino compiles it correctly, or
   - create a minimal `.ino` that includes the necessary project headers and delegates to firmware setup/loop functions.
3. Avoid duplicate definitions of `setup()` and `loop()`.
4. Move or forward project headers so Arduino can find them without PlatformIO's `include/` behavior.
5. Keep the existing services and app architecture intact.

Acceptance checks:

- Arduino IDE opens the project as a sketch.
- Compile succeeds without manually copying headers.
- There is only one `setup()` and one `loop()` in the final Arduino build.

## A2. Remove PlatformIO requirements from docs and acceptance checks

Files to inspect/change:

- `REQUIREMENTS.MD`
- `README.md`
- any build docs
- comments in source if misleading

Tasks:

1. Replace `pio run` acceptance checks with Arduino IDE compile/upload checks.
2. Do not delete `platformio.ini` unless explicitly requested.
3. State that PlatformIO is optional and unsupported by the required workflow.
4. Document board settings for Arduino IDE:
   - ESP32-S3 compatible board
   - 16 MB flash
   - OPI PSRAM enabled
   - USB CDC on boot enabled when serial logging is needed
   - correct upload mode for the user's machine
5. Document Arduino library installation.

Acceptance checks:

- README has a clear Arduino IDE setup path.
- Requirements do not require PlatformIO.
- PlatformIO is not mentioned as the required build system.

## A3. Fix project include paths for Arduino

Known issue:

PlatformIO automatically handles root `include/`. Arduino IDE generally does not behave like PlatformIO. Any project header needed by `src/*.cpp` must be available in a path Arduino compiles and searches.

Tasks:

1. Identify every project header included as:
   ```cpp
   #include "SomeHeader.h"
   ```
2. Ensure that header is in the same folder as the source file or in Arduino's compiled `src/` tree.
3. Prefer moving project headers from `/include` to `/src`.
4. If moving headers is too invasive, create forwarding headers in `/src`:
   ```cpp
   #pragma once
   #include "../include/BoardConfig.h"
   ```
   This is acceptable only as a transitional step.
5. Do not change includes to absolute Windows paths.
6. Do not require Arduino IDE custom compiler flags.

Acceptance checks:

- Arduino compile does not fail with missing project headers.
- The repo can be cloned to a different folder and still compile.

---

# Milestone B — Real e-paper display using LilyGo-EPD47

## B1. Replace epdiy display dependency in the default Arduino build

Files:

- `src/BoardHAL.cpp`
- `src/BoardHAL.h`
- `src/BoardConfig.h`

Tasks:

1. Remove default-path dependency on:
   ```cpp
   #include <epdiy.h>
   ```
2. Include the installed LilyGo driver:
   ```cpp
   #include "epd_driver.h"
   ```
3. Allocate a global/private framebuffer:
   ```cpp
   static uint8_t* g_framebuffer = nullptr;
   ```
4. Allocate it after PSRAM is available:
   ```cpp
   g_framebuffer = (uint8_t*)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
   ```
5. Fail `BoardHAL::begin()` if allocation fails.
6. Initialize the display with `epd_init()`.
7. Clear the screen with `epd_clear()`.
8. Render the boot splash into the framebuffer.
9. Send the framebuffer with `epd_draw_grayscale_image(epd_full_screen(), g_framebuffer)`.
10. Power off the panel after refresh.

Acceptance checks:

- `epdiy.h not found` is gone.
- `EPD_WIDTH` and `EPD_HEIGHT` compile from the installed display library.
- The screen shows "PAPER GO" or equivalent boot splash after upload.

## B2. Implement 4-bit framebuffer drawing

Tasks:

1. Implement a safe 4-bit pixel writer:
   - one byte stores two pixels
   - gray levels clamp to 0..15
   - no off-screen write
2. Implement:
   - `clear(gray)`
   - `fillRect(x,y,w,h,gray)`
   - `drawRect(x,y,w,h,gray)`
   - `drawLine(x1,y1,x2,y2,gray)`
   - `drawText(x,y,text,gray,size)`
3. Preserve the existing 5x7 font logic or replace it with a real Arduino-compatible font renderer.
4. Do not use Serial output as a substitute for drawing.

Acceptance checks:

- Boot screen has text and at least one rectangle.
- Springboard icons/cards render visibly.
- Drawing functions do not crash on out-of-bounds coordinates.

## B3. Fix full refresh and partial refresh policy

Tasks:

1. Implement `beginFrame()` as a real frame preparation hook.
2. Implement `endFrame(fullRefresh)` as a real display update.
3. For the first working Arduino version, full-screen grayscale refresh is acceptable.
4. Partial refresh may be added later only after full-screen refresh works.
5. Do not call unsupported epdiy partial refresh functions in the LilyGo-EPD47 path.

Acceptance checks:

- Switching apps updates the screen.
- Status bar redraws on a timed update.
- Lock screen renders.
- No refresh code depends on `epd_hl_update_screen`.

---

# Milestone C — Real GT911 touch

## C1. Initialize GT911

Files:

- `src/BoardHAL.cpp`
- `src/BoardHAL.h`
- `src/BoardConfig.h`

Tasks:

1. Include:
   ```cpp
   #include <TouchDrvGT911.hpp>
   ```
2. Add a `TouchDrvGT911` instance.
3. Initialize `Wire` with board SDA/SCL.
4. Probe both likely addresses:
   - `0x14`
   - `0x5D`
5. Call `touch.begin(...)` using the detected address.
6. Set max coordinates to display width/height.
7. Configure swap/mirror to match the real panel orientation.
8. Log whether touch is online.

Acceptance checks:

- Serial log reports GT911 online or failed with address.
- If touch fails, firmware still boots and shows a visible error indicator.

## C2. Convert GT911 points into existing touch events

Tasks:

1. Read touch points in `BoardHAL::pollTouch()`.
2. Feed the first point into the existing tap/long-press/swipe/drag classifier.
3. Feed second point into two-point handling only if that code already works.
4. Preserve test injection methods, but only under test/simulator paths.
5. Do not let injected sample state override real touch in normal firmware.

Acceptance checks:

- Tap opens launcher icons.
- Long press opens app options.
- Swipe left/right changes launcher pages when needed.
- Swipe down returns home.
- Drag scrolls reader/file views where implemented.

---

# Milestone D — Screen layout correctness

Known issue:

Some current screen coordinates exceed the real display height. The display is 960x540, so coordinates like `y=680` and `y=900` are invalid.

Tasks:

1. Audit all render functions for hard-coded coordinates.
2. Replace invalid coordinates with layout derived from:
   ```cpp
   BoardConfig::SCREEN_W
   BoardConfig::SCREEN_H
   BoardConfig::STATUS_BAR_H
   ```
3. Make Springboard fit inside 960x540.
4. Make Lock Screen fit inside 960x540.
5. Make GPS Map, Radio, Markdown, Files, Weather, Web, Games, and Settings fit inside 960x540.
6. Add clipping counters in debug builds.
7. Do not hide content off-screen and call it implemented.

Acceptance checks:

- Every app's title and main content are visible.
- No footer is drawn below y=539.
- Springboard page indicator is visible.
- Games cards are visible and tappable.

---

# Milestone E — SD card and persistent storage

## E1. SD card mount

Tasks:

1. Use the board's SPI pins and SD CS pin.
2. Initialize SPI before SD.
3. Mount SD with Arduino `SD.begin(...)`.
4. Show SD status in the status bar.
5. If SD fails, apps that require SD must show a clear error but firmware must continue.

Acceptance checks:

- Empty SD card gets the required folders.
- Missing SD does not crash.
- SD status changes are visible after reboot.

## E2. Create storage layout

Required layout:

```text
/apps/
/cache/
/cache/maps/
/cache/weather/
/cache/http/
/documents/
/documents/markdown/
/documents/text/
/webroot/
/games/
/gps/
/gps/tracks/
/gps/fixes/
/meshtastic/
/meshtastic/messages/
/meshtastic/nodes/
/meshtastic/config/
/radio/
/radio/scans/
/config/
/logs/
```

Tasks:

1. Create folders on boot if SD is mounted.
2. Log folder creation failures.
3. Do not assume folders already exist.
4. Do not write config files into flash when the requirement says SD, except for explicitly mirrored NVS fallback values.

Acceptance checks:

- First boot on a blank SD creates the folder tree.
- File Explorer can browse the created folders.

---

# Milestone F — Battery, RTC, and power

## F1. Battery

Tasks:

1. Probe BQ27220.
2. Probe BQ25896/XPowers.
3. Return real percent, voltage, current, and charging state where available.
4. Use ADC voltage fallback only if gauge chips fail.
5. Show battery percent in status bar.
6. Show charging state correctly.

Acceptance checks:

- `BoardHAL::battery()` reports real data when hardware is present.
- Status bar changes when USB charging state changes.
- No hard-coded placeholder values remain in the normal build.

## F2. RTC

Tasks:

1. Initialize PCF8563/compatible RTC.
2. Display RTC time on lock screen/status bar.
3. Sync RTC from GPS time when available.
4. Sync RTC from network time when configured.
5. Preserve last known time if no source is available.

Acceptance checks:

- Time appears without Wi-Fi after RTC has been set.
- GPS valid time can update RTC.
- Lock screen time is not a fake constant.

## F3. Power states

Tasks:

1. Implement active, idle, lock screen, and deep sleep behaviors.
2. Save current app/state before deep sleep where practical.
3. Power down Wi-Fi/BLE/GPS when policy requires it.
4. Keep e-paper content visible without unnecessary refresh.
5. Use a physical button or configured wake source.

Acceptance checks:

- Idle timeout enters lock screen.
- Deep sleep can be enabled/disabled in settings.
- Device wakes without losing critical settings.

---

# Milestone G — Settings app

Tasks:

1. Implement editable settings rows.
2. Add an on-screen text input method usable with touch.
3. Support Wi-Fi SSID/password entry.
4. Support GPS enable/disable.
5. Support LoRa frequency/region parameters.
6. Support display refresh mode.
7. Support power timeout settings.
8. Support cache controls.
9. Support web server enable/auth settings.
10. Save settings to SD JSON files.
11. Mask saved passwords on screen.
12. Do not display Wi-Fi passwords after save.

Acceptance checks:

- User can enter Wi-Fi credentials entirely on device.
- Reboot preserves settings.
- Invalid JSON falls back safely and shows an error.
- Password is masked in UI.

---

# Milestone H — File Explorer and document apps

## H1. File Explorer

Tasks:

1. Browse folders.
2. Navigate up/down.
3. Show file name, directory/file marker, and size.
4. Open text files.
5. Open Markdown files in Markdown Reader.
6. Create folder.
7. Rename file/folder.
8. Delete file/folder with confirmation.
9. Protect `/config`, `/cache`, and `/apps` unless developer mode is enabled.
10. Implement scrolling for long directories.

Acceptance checks:

- Tapping a folder enters it.
- Tapping `..` goes up.
- Long directories scroll.
- Delete confirmation prevents accidental deletion.

## H2. Markdown Reader

Tasks:

1. Open selected markdown file from File Explorer.
2. Parse headings, paragraphs, bold/italic markers, inline code, code blocks, lists, blockquotes, rules, and links as visible text.
3. Reflow text to 960px width.
4. Support page up/down and drag scroll.
5. Save last reading position per file.
6. Support font size setting.

Acceptance checks:

- Opening `/documents/markdown/readme.md` displays formatted content.
- Reboot resumes last read position.
- Swipe/tap scroll works on real touch.

---

# Milestone I — Network, URL Fetcher, Weather, and Web Server

## I1. Wi-Fi manager

Tasks:

1. Scan networks.
2. Display SSID, RSSI, channel, and security.
3. Select network by touch.
4. Enter password.
5. Save/forget credentials.
6. Connect/disconnect.
7. Show IP address.

Acceptance checks:

- User can join Wi-Fi without editing files by hand.
- Offline state is visible.
- Apps do not block forever while Wi-Fi connects.

## I2. URL Fetcher

Tasks:

1. Enter/edit URL on device.
2. Fetch over Wi-Fi.
3. Show HTTP status.
4. Show response headers optionally.
5. Render plain text.
6. Strip basic HTML to text.
7. Pretty-print JSON.
8. Save response under `/cache/http`.
9. Use cache while offline.

Acceptance checks:

- Valid text URL displays content.
- Offline cached URL displays saved content and stale/offline label.
- Timeout shows a clear error.

## I3. Weather

Tasks:

1. Use GPS fix, last known fix, or manual location.
2. Fetch weather from configured provider.
3. Cache response under `/cache/weather`.
4. Show current condition, temperature, high/low, wind, precipitation chance, and forecast.
5. Mark stale cached data clearly.
6. Do not pretend stale data is live.

Acceptance checks:

- Weather works with live Wi-Fi.
- Weather works from cache offline.
- Missing location shows actionable message.

## I4. Web Server

Tasks:

1. Serve `/webroot`.
2. Provide directory index.
3. Provide device status API.
4. Provide optional file upload if enabled.
5. Provide optional authentication.
6. Show server running state and IP on device.
7. Start/stop from UI.

Acceptance checks:

- Browser can open device IP and list `/webroot`.
- Device status endpoint returns JSON.
- Server can be disabled from UI.

---

# Milestone J — GPS and maps

## J1. GPS service

Tasks:

1. Initialize GPS UART.
2. Parse NMEA with TinyGPSPlus.
3. Maintain current fix, last valid fix, fix quality, satellites, HDOP, altitude, speed, course, and timestamp.
4. Compute heading from movement only.
5. Reject stationary false heading.
6. Maintain best-fit location from recent fixes.
7. Log tracks when enabled.

Acceptance checks:

- GPS status changes from searching to fix.
- Lat/lon display updates.
- Heading shows unavailable when stationary.
- Track CSV writes to `/gps/tracks`.

## J2. Map cache and renderer

Tasks:

1. Convert lat/lon to slippy tile z/x/y.
2. Define a realistic tile format for ESP32:
   - preprocessed grayscale raw tile, or
   - decoded image only if memory/performance is proven.
3. Cache tiles under `/cache/maps/{provider}/{z}/{x}/{y}.tile`.
4. Render cached tiles into the map view.
5. Draw current GPS position over map.
6. Draw missing tile placeholders.
7. Show cache coverage state.
8. Support pan/zoom by touch.
9. Respect provider terms and do not bulk-download from restricted providers.

Acceptance checks:

- Cached tile appears on screen.
- Missing tile placeholder appears when tile absent.
- GPS marker appears over map.
- Offline map works with cached tiles.

---

# Milestone K — Radio scanner and LoRa utilities

Tasks:

1. Initialize SX1262 with RadioLib.
2. Scan Wi-Fi networks.
3. Scan BLE advertisements.
4. Receive LoRa packets for configured frequency/spreading/bandwidth/coding parameters.
5. Distinguish:
   - Wi-Fi APs
   - BLE ads
   - LoRa packets
   - unknown/unsupported RF
6. Log scans under `/radio/scans`.
7. Show RSSI, SNR, channel/frequency, packet count, and last seen time.
8. Do not claim the SX1262 is a universal radio scanner.

Acceptance checks:

- Wi-Fi scan shows nearby networks.
- BLE scan shows advertisements.
- LoRa packet receive works with a known transmitter.
- Scan log is written to SD.

---

# Milestone L — Meshtastic / mesh messaging

Important boundary:

Full Meshtastic compatibility is a major integration task. Do not fake it with a screen that only counts files.

Tasks:

1. Choose one real implementation path:
   - integrate a Meshtastic-compatible protocol layer, or
   - implement a clearly named "LoRa Mesh Lite" app that does not claim full Meshtastic compatibility.
2. Implement node list.
3. Implement message list.
4. Implement send message.
5. Implement receive message.
6. Store messages under `/meshtastic/messages`.
7. Store node data under `/meshtastic/nodes`.
8. Support GPS position sharing on/off.
9. Show last-heard time and signal quality.

Acceptance checks:

- Two devices can exchange at least one text message, or the app is renamed/scoped honestly as an incomplete non-Meshtastic LoRa messenger.
- Received messages appear in UI.
- Messages persist after reboot.
- GPS sharing can be disabled.

---

# Milestone M — Games

Tasks:

1. Games folder opens individual game apps.
2. Tic-tac-toe has complete touch play, win/draw detection, reset, save, and resume.
3. Minesweeper has reveal, flag, mine count, win/loss, difficulty, save, and resume.
4. Chess has board rendering, piece selection, legal move validation, check detection, save, and resume.
5. Go has 9x9 board, stone placement, captures, pass, local two-player, save, and resume.
6. Each game must render inside 960x540.
7. Each game must return to Games folder/Home.

Acceptance checks:

- Each game launches from the Games folder.
- Each game accepts touch input.
- Each game can be exited without reboot.
- Save/resume works where required.

---

# Milestone N — Final hardening

Tasks:

1. Add crash/wdt reset logging.
2. Add boot diagnostics screen or log mode.
3. Add visible errors for missing SD, touch failure, GPS failure, battery failure, LoRa failure, and Wi-Fi failure.
4. Audit memory use and PSRAM allocation.
5. Ensure long network operations do not freeze the UI.
6. Ensure all blocking scans have cancel/timeout behavior.
7. Document known hardware limitations.
8. Update README with Arduino-only quick start.

Final release acceptance checks:

- Clean Arduino IDE compile.
- Upload succeeds.
- Boot splash visible.
- Touch works.
- Launcher works.
- Status bar displays real SD/GPS/Wi-Fi/battery states.
- File Explorer can browse SD.
- Settings can edit at least Wi-Fi and power settings.
- No required feature is represented only by a stub or placeholder notice.
- No PlatformIO command is required to build or verify the firmware.


Below is a clean project-specific requirements list for the **LILYGO T5-4.7-S3 E-Paper Pro ESP32-S3 SX1262 LoRa 915 MHz TF Card RTC GPS board**. This is **not** the water valve project and **not** the e-ink schedule display project.

The board target should be treated as a small **touch e-paper handheld operating shell** with multiple apps, persistent SD-card storage, GPS/location awareness, radio tools, offline map/weather caching, and light utility/game functions.

The hardware baseline I found for this board is: ESP32-S3, 16 MB flash, 8 MB PSRAM, SX1262 LoRa, MIA-M10Q GPS/GNSS, 4.7-inch ED047TC1 e-paper display at **960×540**, 16 grayscale levels, GT911 capacitive two-point touch, TF card, PCF8563/PCF85063-class RTC, and BQ25896/BQ27220 battery/charging hardware. ([LILYGO®][1])

---

# LILYGO T5-4.7-S3 E-Paper Pro Firmware Requirements

## 1. Project Identity

### 1.1 Device Purpose

The firmware shall turn the LILYGO T5-4.7-S3 E-Paper Pro into a standalone e-paper handheld with:

1. A touch-driven springboard launcher.
2. Multiple independent applications.
3. GPS-aware local services.
4. Offline-first map and weather caching.
5. LoRa / Wi-Fi / Bluetooth radio utilities.
6. SD-card file browsing and document reading.
7. A lightweight onboard web server.
8. Lock screen and persistent status bar.
9. Simple offline games.

### 1.2 Explicit Non-Goals

This firmware shall not be mixed with:

1. The garden watering valve controller.
2. The e-ink irrigation schedule display.
3. Any relay-host or watering-zone firmware.
4. Any automatic irrigation decision logic.

This is a separate handheld e-paper application platform.

---

# 2. Hardware Target Requirements

## 2.1 Board

Target board:

**LILYGO T5-4.7-S3 E-Paper Pro ESP32-S3 SX1262 LoRa 915 MHz TF Card RTC TTGO Development Board with 4.7-inch Ultra-Low Power Touchable E-Paper Display with GPS**

## 2.2 Required Hardware Support

Firmware shall support:

1. ESP32-S3 MCU.
2. 16 MB flash.
3. 8 MB PSRAM.
4. 4.7-inch ED047TC1 e-paper display.
5. 960×540 display resolution.
6. 16-level grayscale rendering.
7. GT911 capacitive touch input.
8. SX1262 LoRa transceiver at 915 MHz.
9. MIA-M10Q GPS/GNSS module.
10. TF / microSD card storage.
11. RTC timekeeping.
12. Wi-Fi station mode.
13. Wi-Fi access point mode where needed.
14. Bluetooth Low Energy scanning.
15. Battery gauge.
16. Charging status detection.
17. Deep sleep / low-power state.
18. Wake from button, touch, RTC alarm, or configured event where hardware supports it.

The display and touch stack should be based on the board’s actual ED047TC1 / GT911 hardware, not an abstract simulator layer. The LILYGO documentation identifies the display as ED047TC1 960×540 with 16 grayscale levels and the touch controller as GT911. ([wiki.lilygo.cc][2])

---

# 3. Firmware Architecture Requirements

## 3.1 Application Shell

The firmware shall implement a lightweight application shell with:

1. Boot screen.
2. Lock screen.
3. Persistent status bar.
4. Springboard app launcher.
5. App lifecycle manager.
6. Shared services layer.
7. Shared rendering system.
8. Shared touch input dispatcher.
9. Shared SD-card storage layer.
10. Shared network manager.
11. Shared GPS manager.
12. Shared cache manager.
13. Shared power manager.

## 3.2 Recommended Development Stack

Preferred stack:

1. ESP-IDF or Arduino-as-component under ESP-IDF.
2. Direct board-library support from LILYGO examples where practical.
3. LVGL only if it can be made performant and visually appropriate for grayscale e-paper.
4. Custom e-paper UI renderer acceptable and likely preferable for performance and control.

The board is advertised as supporting Arduino IDE and VS Code development, and the LILYGO material references sample code and GitHub resources. ([LILYGO®][1])

## 3.3 Storage Layout

The SD card shall contain a predictable directory structure:

```text
/sd/
  apps/
  cache/
    maps/
    weather/
    http/
  documents/
    markdown/
    text/
  webroot/
  games/
  gps/
    tracks/
    fixes/
  meshtastic/
    messages/
    nodes/
  radio/
    scans/
  config/
    wifi.json
    device.json
    apps.json
    map.json
    weather.json
    power.json
  logs/
```

---

# 4. Display and UI Requirements

## 4.1 Display Rules

The UI shall be designed specifically for a **960×540 16-grayscale e-paper display**.

The UI shall:

1. Use high-contrast grayscale.
2. Avoid thin low-contrast text.
3. Avoid animations that require repeated full refreshes.
4. Prefer page transitions, modal overlays, and partial refresh zones.
5. Use large touch targets.
6. Use strong icon silhouettes.
7. Use simple outlined UI cards.
8. Use dithered grayscale only where useful.
9. Avoid heavy full-screen refresh unless changing major app views.
10. Use partial refresh for cursor, selection, status bar, scroll indicators, and small changes where supported.

## 4.2 Visual Style

The device UI should feel like a small field-grade e-paper computer:

1. Clean monochrome operating shell.
2. Strong icon grid.
3. Technical but readable layout.
4. Minimal decoration.
5. No glossy phone-style UI.
6. No cluttered dense dashboards.
7. App icons should be simple, bold, and legible in grayscale.

## 4.3 Touch Requirements

The firmware shall support:

1. Tap.
2. Long press.
3. Swipe up/down.
4. Swipe left/right.
5. Drag for scrolling.
6. Two-point touch where useful, but not required for every app.

## 4.4 Shared Navigation

### 4.5 Implementation Checklist

- [x] Tap classification implemented.
- [x] Long-press classification implemented.
- [x] Swipe up/down/left/right classification implemented.
- [x] Drag classification implemented for scrolling flows.
- [x] Two-point touch gesture handling integrated with GT911 driver.


Every app shall provide:

1. Back gesture or back button.
2. Home gesture or home button.
3. App menu.
4. Scroll support where content exceeds screen.
5. Low-power idle timeout behavior.
6. Safe return to springboard.

---

# 5. Persistent Status Bar Requirements

## 5.1 Status Bar

A persistent status bar shall appear at the top of the device.

It shall show:

1. Current time.
2. Wi-Fi status.
3. LoRa status.
4. Bluetooth status where relevant.
5. GPS status.
6. Battery percentage.
7. Charging status.
8. SD-card mounted status.
9. Optional unread message indicator.
10. Optional cache/network activity indicator.

## 5.2 Time Source Priority

Time shall be resolved in this order:

1. RTC.
2. GPS time.
3. Network time over Wi-Fi.
4. Last known saved time if no better source is available.

## 5.3 GPS Status States

GPS indicator shall show at least:

1. Off.
2. Searching.
3. 2D fix.
4. 3D fix.
5. Degraded / stale fix.
6. No antenna / no data if detectable.

---

# 6. Lock Screen Requirements

## 6.1 Lock Screen Contents

The lock screen shall show:

1. Large current time.
2. Date.
3. Current location summary.
4. GPS status.
5. Battery status.
6. Charging status.
7. Small local map tile or cached map preview if available.
8. Last known coordinates if GPS is unavailable.
9. Unlock gesture.

## 6.2 Location Display

The lock screen shall display one or more of:

1. Latitude / longitude.
2. Nearest cached place name if available.
3. Map tile centered on last known or current location.
4. Heading indicator if moving.

## 6.3 Low-Power Lock Mode

The lock screen shall be optimized for low-power operation:

1. Minimal refresh.
2. RTC wake for time update if enabled.
3. GPS off by default unless user configures live tracking.
4. Wi-Fi off by default unless required for background task.
5. LoRa behavior configurable.

---

# 7. Springboard App Launcher Requirements

## 7.1 Launcher Layout

The springboard shall show app icons in a touch-friendly grid.

Required apps:

1. GPS Map.
2. Radio Scanner.
3. Meshtastic.
4. URL Fetcher.
5. Markdown Reader.
6. File Explorer.
7. Weather.
8. Web Server.
9. Games Folder.
10. Settings.

## 7.2 App Icon Requirements

Each app icon shall have:

1. Simple grayscale icon.
2. Short label.
3. Large touch target.
4. Optional status badge.
5. Disabled/offline visual state if required service is unavailable.

## 7.3 Launcher Behavior

### 7.4 Implementation Checklist

- [x] Persistent app ordering stored in `/config/apps.json` and loaded at launcher start.
- [x] Unknown IDs are rejected and duplicate IDs are deduplicated when loading ordering.
- [x] Long-press app options implemented.
- [x] Multi-page swipe navigation implemented when icon count exceeds one screen.
- [x] Connectivity-dependent app icons display an online-required status badge.
- [x] Connectivity-dependent app icons render a distinct unavailable visual state.


The launcher shall support:

1. Tap to open app.
2. Long press for app options.
3. Swipe between pages if icons exceed one screen.
4. Return from app without reboot.
5. Persistent app ordering stored on SD or internal config.

---

# 8. GPS Map App Requirements

## 8.1 Core Map Function

The GPS Map app shall:

1. Use current GPS location.
2. Pull map tiles for the current location when network is available.
3. Render cached map tiles from SD card.
4. Show current position.
5. Show best-fit location.
6. Show heading derived from GPS differentials.
7. Show speed if available.
8. Show fix accuracy if available.
9. Show stale-location warning when GPS data is old.
10. Allow panning around the map.
11. Allow zooming between supported tile zoom levels.

## 8.2 GPS Heading Requirement

Heading shall be computed from GPS movement differentials.

Important constraint:

GPS-derived heading is only reliable while the device is moving. When stationary or moving too slowly, the app shall show heading as unavailable, stale, or low-confidence rather than pretending it has a compass heading.

## 8.3 Best-Fit Location

The app shall compute a best-fit location using:

1. Recent GPS fixes.
2. HDOP / accuracy estimates where available.
3. Movement continuity.
4. Rejection of obvious outlier points.
5. Last known valid position.
6. Optional smoothing window.

## 8.4 Map Tile Cache

Map tiles shall be cached to SD card.

The cache shall support:

1. Long-term tile retention.
2. Folder structure by provider / zoom / x / y.
3. Metadata file for cache index.
4. Cache hit/miss tracking.
5. Configurable max cache size.
6. Configurable never-delete/favorite regions.
7. Manual cache cleanup.
8. Automatic offline use.
9. Progressive accumulation of areas the user frequents.

## 8.5 Offline Map Behavior

When offline, the map app shall:

1. Render any available cached tiles.
2. Show missing tile placeholders.
3. Continue showing GPS position over cached map.
4. Allow browsing cached regions.
5. Show whether the current view is fully cached, partially cached, or uncached.

## 8.6 Tile Source Requirements

The firmware shall allow configurable tile providers.

Each provider config shall include:

1. Name.
2. URL template.
3. Attribution text.
4. Max zoom.
5. Min zoom.
6. Cache policy.
7. User agent.
8. API key field if required.

Do not hard-code a tile provider in a way that violates provider caching or bulk-download rules.

---

# 9. Radio Scanner App Requirements

## 9.1 Radio Scanner Scope

The Radio Scanner app shall scan and display local signals for:

1. LoRa / SX1262-supported radio activity.
2. Wi-Fi access points.
3. Bluetooth Low Energy advertisements.

## 9.2 Wi-Fi Scan

The Wi-Fi scanner shall show:

1. SSID.
2. BSSID / MAC.
3. RSSI.
4. Channel.
5. Security type.
6. Hidden network flag where detectable.
7. Last seen time.
8. Scan refresh control.

## 9.3 Bluetooth Scan

The Bluetooth scanner shall show:

1. Device name where advertised.
2. MAC / address.
3. RSSI.
4. Advertisement data summary.
5. Service UUIDs where available.
6. Manufacturer data where available.
7. Last seen time.

## 9.4 LoRa Scan

The LoRa scanner shall show:

1. Frequency.
2. RSSI.
3. SNR where available.
4. Packet count.
5. Last packet time.
6. Decoded payload only when protocol parameters are known.
7. Unknown packet marker where payload cannot be decoded.
8. Configurable LoRa parameters.

## 9.5 Radio Scanner Limitation

The SX1262 cannot generically decode “all local radio signals.” It can scan and receive within its supported RF range and configured modulation/protocol settings. Therefore the app shall distinguish between:

1. Energy detection.
2. LoRa packet detection.
3. Decoded LoRa packets.
4. Wi-Fi networks.
5. BLE advertisements.
6. Unknown or unsupported signals.

## 9.6 Scan Log

The app shall optionally save scans to:

```text
/sd/radio/scans/
```

Each saved scan shall include:

1. Timestamp.
2. Location if GPS is available.
3. Radio type.
4. Signal information.
5. Raw advertisement or packet summary where safe and available.

---

# 10. Meshtastic LoRa App Requirements

## 10.1 App Purpose

The Meshtastic app shall provide LoRa mesh messaging and GPS tracking behavior.

## 10.2 Required Functions

The Meshtastic app shall support:

1. Node list.
2. Message list.
3. Send message.
4. Receive message.
5. GPS position broadcast.
6. Display nearby nodes.
7. Show node signal quality.
8. Show last-heard time.
9. Store message history on SD card.
10. Store known node history on SD card.

## 10.3 Implementation Boundary

There are two possible implementation approaches:

1. Port/adapt Meshtastic firmware support for this hardware.
2. Implement a simplified Meshtastic-compatible client layer if full compatibility is too heavy.

A Meshtastic issue exists discussing the T5-S3 E-Paper Pro with SX1262 and InkHUD-related work, so this should be treated as a known but board-specific integration area rather than assumed finished. ([GitHub][3])

## 10.4 Meshtastic Storage

Meshtastic data shall be stored under:

```text
/sd/meshtastic/
  messages/
  nodes/
  config/
```

## 10.5 GPS Tracking

GPS tracking shall support:

1. Manual position send.
2. Periodic position send.
3. Position sharing on/off.
4. Last sent location.
5. Last received node locations.
6. Map view integration where practical.

---

# 11. URL Fetcher App Requirements

## 11.1 Purpose

The URL Fetcher app shall fetch a URL over Wi-Fi and render the response as a scrollable text page.

## 11.2 Required Functions

The app shall support:

1. Enter URL.
2. Fetch URL.
3. Display HTTP status.
4. Display headers optionally.
5. Render text response.
6. Basic HTML stripping or plain-text mode.
7. Scroll through response.
8. Save response to SD.
9. Open saved response.
10. Show fetch errors clearly.

## 11.3 Response Handling

The fetcher shall support:

1. `text/plain`
2. `text/html` as simplified text
3. `application/json` as formatted text
4. Markdown if served as text
5. Error page display

## 11.4 Cache

Fetched pages may be cached under:

```text
/sd/cache/http/
```

---

# 12. Markdown Reader App Requirements

## 12.1 Purpose

The Markdown Reader shall read Markdown files from the SD card and display formatted content on the e-paper screen.

## 12.2 Supported Markdown Features

The renderer shall support:

1. Headings.
2. Paragraphs.
3. Bold.
4. Italic.
5. Inline code.
6. Code blocks.
7. Blockquotes.
8. Ordered lists.
9. Unordered lists.
10. Horizontal rules.
11. Links as text.
12. Tables if feasible.
13. Images omitted or shown as placeholders unless later supported.

## 12.3 Reading Behavior

The app shall support:

1. Open file from SD.
2. Recent files.
3. Scroll page.
4. Page up/down.
5. Font size options.
6. Reflow text to screen width.
7. Remember last read position.
8. Show filename and path.
9. Return to File Explorer.

---

# 13. File Explorer App Requirements

## 13.1 Purpose

The File Explorer shall allow browsing the SD card.

## 13.2 Required Functions

The File Explorer shall support:

1. Browse folders.
2. Open files.
3. Navigate up one level.
4. Show file size.
5. Show file type.
6. Show modified time where available.
7. Create folder.
8. Delete file or folder with confirmation.
9. Rename file or folder.
10. Copy file.
11. Move file.
12. View text files.
13. Open Markdown files in Markdown Reader.
14. Open cached HTTP text.
15. Open game saves where appropriate.
16. Open map/weather cache metadata where useful.

## 13.3 Safety Requirements

The File Explorer shall protect critical system folders from accidental deletion unless developer mode is enabled.

Protected folders:

```text
/sd/config/
/sd/cache/
/sd/apps/
```

---

# 14. Weather App Requirements

## 14.1 Purpose

The Weather app shall show current weather and a 5-day forecast for the current GPS location.

## 14.2 Required Display

The Weather app shall show:

1. Current temperature.
2. Current condition.
3. High / low.
4. Wind.
5. Humidity if available.
6. Precipitation chance.
7. Forecast summary.
8. 5-day forecast.
9. Last update time.
10. Whether data is live or cached.

## 14.3 Location Source

Weather location shall come from:

1. Current GPS fix.
2. Last known GPS fix.
3. Manually configured location.

## 14.4 Weather Cache

Weather data shall cache to SD card under:

```text
/sd/cache/weather/
```

The cache shall store:

1. Forecast response.
2. Current conditions response.
3. Timestamp.
4. Location.
5. Provider metadata.
6. Expiration time.

## 14.5 Offline Weather Behavior

When offline, the Weather app shall:

1. Show cached weather if available.
2. Clearly mark cached/stale data.
3. Show the age of cached data.
4. Avoid pretending stale weather is current.

---

# 15. Onboard Web Server Requirements

## 15.1 Purpose

The device shall host a folder from the SD card over Wi-Fi station mode.

## 15.2 Webroot

The hosted folder shall be:

```text
/sd/webroot/
```

## 15.3 Required Server Functions

The web server shall support:

1. Static file hosting.
2. Directory index page.
3. Download files.
4. Optional upload files.
5. Optional delete files.
6. Optional simple authentication.
7. Display device status page.
8. Show IP address on device.
9. Start/stop server from UI.

## 15.4 Wi-Fi Mode

The web server shall operate over Wi-Fi station mode.

Optional later feature:

1. Wi-Fi AP fallback for direct connection.
2. Captive portal for setup.

---

# 16. Game App Folder Requirements

## 16.1 Games Folder

The springboard shall include a Games folder.

Required games:

1. Chess.
2. Go.
3. Tic-tac-toe.
4. Minesweeper.

## 16.2 Shared Game Requirements

### 16.2 Implementation Checklist

- [x] New game core logic implemented for Chess, Go, Tic-tac-toe, and Minesweeper.
- [x] Save/resume core serialization implemented for Chess and Tic-tac-toe.
- [x] Save/resume serialization implemented for Go and Minesweeper.

Each game shall support:

1. Touch controls.
2. New game.
3. Save game.
4. Resume game.
5. Exit to Games folder.
6. Clear e-paper-friendly board rendering.
7. Minimal refresh redraw strategy.

## 16.3 Chess Requirements

Chess shall support:

1. Legal board rendering.
2. Piece movement by touch.
3. Legal move validation.
4. Check detection.
5. Checkmate detection if feasible.
6. Local two-player mode.
7. Optional simple AI later.

## 16.4 Go Requirements

Go shall support:

1. Board size selection, at minimum 9×9.
2. Optional 13×13 and 19×19 if usable on screen.
3. Stone placement.
4. Capture rules.
5. Pass.
6. Local two-player play.
7. Basic scoring helper if feasible.

## 16.5 Tic-Tac-Toe Requirements

### 16.5 Implementation Checklist

- [x] Local two-player mode.
- [x] Optional simple AI.
- [x] Win/draw detection.
- [x] Reset.
- [x] New game/save game/resume game core logic implemented.

Tic-tac-toe shall support:

1. Local two-player mode.
2. Optional simple AI.
3. Win/draw detection.
4. Reset.

## 16.6 Minesweeper Requirements

Minesweeper shall support:

1. Grid rendering.
2. Reveal tile.
3. Flag tile.
4. Mine count.
5. Win/loss detection.
6. Difficulty levels.

---

# 17. Network Manager Requirements

## 17.1 Wi-Fi Configuration

The device shall support:

1. Scan Wi-Fi networks.
2. Select network.
3. Enter password.
4. Save credentials.
5. Connect as station.
6. Disconnect.
7. Forget network.
8. Show current IP.
9. Show connection status.

## 17.2 Network Use

Network shall be used for:

1. Map tile download.
2. Weather fetch.
3. URL fetch.
4. Time sync.
5. Web server hosting.
6. Optional OTA updates.

## 17.3 Offline-First Behavior

Every app that uses network data shall:

1. Work without network where cached data exists.
2. Clearly show offline state.
3. Avoid blocking UI while waiting for network.
4. Store failed fetch state where useful.

---

# 18. GPS Manager Requirements

## 18.1 Shared GPS Service

A shared GPS service shall provide:

1. Current fix.
2. Last known fix.
3. Fix quality.
4. Timestamp.
5. Speed.
6. Course over ground if available.
7. Computed heading from recent fixes.
8. Raw coordinates.
9. Accuracy estimate where available.

## 18.2 GPS Logging

The device shall optionally log GPS tracks under:

```text
/sd/gps/tracks/
```

Track logs shall include:

1. Timestamp.
2. Latitude.
3. Longitude.
4. Altitude if available.
5. Speed if available.
6. Heading if available.
7. Accuracy/fix quality.

## 18.3 GPS Privacy

The user shall be able to:

1. Disable GPS.
2. Disable track logging.
3. Disable Meshtastic position broadcasting.
4. Clear location history.
5. Clear cached location metadata.

---

# 19. Cache Manager Requirements

## 19.1 Shared Cache System

The firmware shall implement a shared SD-backed cache manager for:

1. Map tiles.
2. Weather data.
3. HTTP fetches.
4. Optional rendered document pages.

## 19.2 Map Tile Cache Priority

Map tiles shall be treated as long-term cache, not short-lived temporary data.

The user wants to gradually accumulate offline map coverage for places they frequently visit.

## 19.3 Cache Controls

Settings shall include:

1. View cache usage.
2. Clear HTTP cache.
3. Clear weather cache.
4. Clear map cache.
5. Clear selected map region.
6. Protect selected map regions from deletion.
7. Set max cache size.
8. Export cache metadata.

---

# 20. Power Management Requirements

## 20.1 Power Modes

The device shall support:

1. Active mode.
2. Idle mode.
3. Lock-screen mode.
4. Deep sleep mode.
5. Charging mode.
6. Background LoRa mode if feasible.

## 20.2 Low-Power Rules

The firmware shall:

1. Turn off Wi-Fi when not needed.
2. Turn off Bluetooth after scans.
3. Allow GPS duty cycling.
4. Avoid unnecessary e-paper refreshes.
5. Use RTC wake where appropriate.
6. Store state before sleep.
7. Restore previous app or lock screen after wake.

## 20.3 Battery Display

Battery status shall include:

1. Percent.
2. Charging / not charging.
3. Optional voltage.
4. Optional current.
5. Optional low-battery warning.
6. Optional estimated runtime if feasible.

The board is documented with BQ25896 and BQ27220 battery/charging hardware, so the battery subsystem should use the real gauge/charger chips rather than a guessed voltage-only indicator. ([LILYGO®][1])

---

# 21. Settings App Requirements

## 21.1 Device Settings

Settings shall include:

1. Wi-Fi networks.
2. Time and RTC sync.
3. GPS on/off.
4. LoRa region/frequency.
5. Display refresh mode.
6. Font size.
7. Touch calibration if needed.
8. Battery/power settings.
9. Cache settings.
10. Web server settings.
11. Meshtastic settings.
12. About/device info.
13. Factory reset.
14. SD-card status.

## 21.2 Developer Settings

Developer mode may include:

1. Raw GPS data view.
2. Raw LoRa packet view.
3. Display test.
4. Touch test.
5. Battery chip test.
6. RTC test.
7. SD-card test.
8. Wi-Fi diagnostic.
9. BLE diagnostic.
10. Log viewer.

---

# 22. Data Persistence Requirements

## 22.1 Config Storage

Configuration shall be stored in JSON or another simple readable format.

Required files:

```text
/sd/config/device.json
/sd/config/wifi.json
/sd/config/apps.json
/sd/config/map.json
/sd/config/weather.json
/sd/config/power.json
/sd/config/meshtastic.json
```

## 22.2 Internal Fallback Storage

Critical settings shall also be mirrored to ESP32 NVS where appropriate:

1. Boot mode.
2. Last known good config.
3. Wi-Fi fallback state.
4. Device ID.
5. Crash counter.
6. Last selected app.

---

# 23. Logging Requirements

The firmware shall log:

1. Boot events.
2. Crashes or watchdog resets.
3. Wi-Fi connection failures.
4. GPS fix state changes.
5. Map tile download failures.
6. Weather fetch failures.
7. LoRa receive/transmit errors.
8. SD-card mount failures.
9. Battery warnings.
10. Web server start/stop events.

Logs shall be stored under:

```text
/sd/logs/
```

---

# 24. Security Requirements

## 24.1 Wi-Fi Credentials

Wi-Fi credentials shall not be displayed in plain text after saving.

## 24.2 Web Server

The web server shall support at minimum:

1. Disabled by default or user-confirmed enable.
2. Optional password.
3. Visible status indicator while running.
4. Clear warning if unauthenticated access is enabled.

## 24.3 Location Data

GPS history and Meshtastic location data shall be user-clearable.

---

# 25. Feasibility Notes / Important Constraints

## 25.1 GPS Heading

GPS heading from differentials is feasible, but only while moving. The firmware must not show false precision while stationary.

## 25.2 Radio Scanning

Wi-Fi and BLE scanning are straightforward on ESP32-S3.

LoRa scanning is more limited. The SX1262 can detect LoRa activity under configured parameters, but it is not a universal RF scanner and cannot identify every arbitrary protocol in the environment.

## 25.3 Map Tiles

Map tile caching is feasible, but the firmware must respect the selected map provider’s tile usage and caching rules. Long-term offline caching should ideally use a provider or tile package format that allows it.

## 25.4 Meshtastic

Meshtastic integration is likely the hardest app because full compatibility may require significant firmware integration. Treat it as a major subsystem, not a small UI widget.

## 25.5 E-Paper UI

This must be designed around slow refresh and low-power behavior. The UI should avoid phone-style constant animation.

---

# 26. Suggested Implementation Phases

## Phase 1 — Board Bring-Up

1. Display test.
2. Touch test.
3. SD-card mount.
4. RTC read/write.
5. Battery gauge read.
6. GPS read.
7. Wi-Fi scan/connect.
8. BLE scan.
9. SX1262 basic test.

## Phase 2 — Shell

1. Boot screen.
2. Status bar.
3. Lock screen.
4. Springboard launcher.
5. App lifecycle.
6. Settings app.

## Phase 3 — Storage and Files

1. SD directory structure.
2. File Explorer.
3. Text viewer.
4. Markdown Reader.
5. Config persistence.
6. Logging.

## Phase 4 — Network Apps

1. Wi-Fi manager.
2. URL Fetcher.
3. Web server.
4. Weather app.
5. Weather cache.

## Phase 5 — GPS and Maps

1. GPS manager.
2. Location display.
3. Map tile downloader.
4. Map tile renderer.
5. Offline tile cache.
6. Heading and best-fit location.

## Phase 6 — Radio Apps

1. Wi-Fi scanner.
2. BLE scanner.
3. LoRa scanner.
4. Scan logging.

## Phase 7 — Meshtastic

1. Determine whether to integrate existing Meshtastic/InkHUD work or implement limited compatibility.
2. Node list.
3. Messaging.
4. GPS position sharing.
5. Message/node storage.

## Phase 8 — Games

1. Tic-tac-toe.
2. Minesweeper.
3. Chess.
4. Go.

---

# 27. Initial App List

The first firmware release should include these apps:

1. **GPS Map**
2. **Radio Scanner**
3. **Meshtastic**
4. **URL Fetcher**
5. **Markdown Reader**
6. **File Explorer**
7. **Weather**
8. **Web Server**
9. **Games**
10. **Settings**

This should be treated as a miniature e-paper field computer OS, with the springboard as the main interaction model and the SD card as the long-term data/cache backbone.

[1]: https://lilygo.cc/en-us/products/t5-e-paper-s3-pro?srsltid=AfmBOoofcnSonjX3RjfaPmReKQc9Vl08gY_-lu0pc5AmIPZDFCd11Ca8&utm_source=chatgpt.com "T5 E-Paper S3 Pro"
[2]: https://wiki.lilygo.cc/get_started/en/Wearable/T5-E-Paper-S3-Pro/T5-E-Paper-S3-Pro.html?utm_source=chatgpt.com "LILYGO T5 E-Paper S3 Pro"
[3]: https://github.com/meshtastic/firmware/issues/6607?utm_source=chatgpt.com "[Board]: T5S3 aka T5 E-Paper S3 Pro · Issue #6607"


### 5.4 Implementation Checklist

- [x] Status bar GPS indicator includes explicit states: Off, Searching, 2D fix, 3D fix, Degraded/stale, and No data.
- [x] Time source priority is implemented in code using RTC -> GPS -> Network -> Saved.
- [x] Lock screen displays resolved location summary/map preview from live cache pipeline.
- [x] Status bar unread-message and cache-activity indicators.
- [x] Status bar rendering now skips unchanged frames using snapshot comparison to support e-paper partial-refresh behavior.
- [x] Status bar redraw cache is scoped to each `AppManager` instance (no function-static cross-instance state).
- [x] Board pin mapping aligned to T5-4.7-S3 Pro for display timing/I2C/SD/LoRa/GPS with display control on IO48.

- [x] Display rendering primitives now clamp grayscale values to 0-15 and clip draw rectangles to the 960x540 panel bounds.
- [x] Display line primitives are clipped to screen bounds before rendering to prevent off-panel refresh work.
- [x] AppManager forces a periodic full e-paper refresh after bounded partial refresh cycles to reduce ghosting.
- [x] Periodic refresh loop now skips frame submission when status/app state is unchanged, reducing unnecessary e-paper refreshes.


### 19.4 Implementation Checklist

- [x] Lightweight onboard HTTP server serves static content from `/webroot` with MIME detection.
- [x] URI path traversal rejection (`..` and backslash) for static content requests.
- [x] JSON health endpoint implemented (`GET /api/health`).
- [x] Full remote app management API (install/remove/update apps).

- [x] Offline map behavior exposes cached/partial/uncached view state.
- [x] Cache lookup hit/miss tracking implemented for map tile checks.

- [x] Global Home navigation via swipe-down gesture.
- [x] Global Back navigation via swipe-right gesture using app history stack.

- [x] Radio scans are persisted under `/radio/scans/` with timestamped entries.
- [x] Saved scan entries include signal information and GPS location when available.

- [x] Weather cache is labeled stale/fresh in the Weather app UI.
- [x] Weather cache data is loaded from `/cache/weather/current.json`.

- [x] URL Fetcher reads URL config and caches HTTP responses under `/cache/http`.
- [x] URL Fetcher supports offline cached-response fallback when Wi-Fi is unavailable.

- [x] Markdown Reader loads markdown content from SD (`/documents/markdown/readme.md`).
- [x] Markdown Reader renders a bounded preview view suitable for e-paper updates.
- [x] Markdown Reader preview applies e-paper-friendly formatting for headings, lists, blockquotes, and code blocks.
- [x] Markdown Reader supports swipe-based vertical preview scrolling with bounded window rendering.
- [x] Markdown preview supports ordered lists, link-text rendering, table-line markers, and image placeholders.
- [x] Markdown preview renders task-list checkboxes and horizontal-rule variants with high-contrast e-paper glyphs.
- [x] Markdown Reader persists and restores last read position for `/documents/markdown/readme.md`.

- [x] Web API exposes `/api/status` for core device status snapshot.
- [x] OpenAPI specification added for `/api/health`, `/api/status`, and `/api/apps/order`.

- [x] Web API exposes cached weather payload endpoint (`/api/weather/cache`).

- [x] BLE scan output includes service UUID and manufacturer-data summary fields.

- [x] Lock screen renders large time/date and coordinate summary from available GPS epoch/fix.
- [x] Lock screen displays GPS and battery status fields.

- [x] File Explorer displays directory-first sorted listings for SD browsing.

- [x] GPS best-fit location computed from recent fixes with basic outlier rejection.

- [x] GPS track logging persists movement-filtered fixes to `/gps/tracks/current_track.csv` including time, lat/lon, altitude, speed, heading, HDOP, and sats.

- [x] GPS track logging can be disabled by default and enabled via `/config/device.json` (`gpsTrackLogging`).

- [x] Meshtastic storage includes `/meshtastic/config` in addition to messages/nodes.
- [x] Meshtastic app surfaces SD storage counts for messages and nodes.

- [x] Web API exposes cache hit/miss counters via `/api/cache/stats`.

- [x] Web API exposes radio scan log listing endpoint (`/api/radio/scans`).

- [x] Web API exposes Meshtastic storage counters endpoint (`/api/meshtastic/stats`).

- [x] Wi-Fi station boot-connect now loads persisted credentials from `/config/wifi.json` and supports save/forget flows in `NetworkService`.

- [x] Boot serial diagnostics now report board/cache/gps/network/radio/web init results plus periodic Wi-Fi state snapshots.

---

# 28. Actionable Execution Backlog (Authoritative Task List)

Use this section as the implementation queue. Execute tasks in order, and only mark a task complete after its acceptance criteria in this document have passed on-device with the Arduino IDE workflow.

## 28.1 Program rules and build gate tasks

- [ ] Enforce non-negotiable rules 0.1–0.12 across all new changes.
- [ ] Verify Arduino IDE build workflow (board profile, PSRAM enabled, 16MB flash, USB CDC where needed).
- [ ] Verify repository opens as a valid Arduino sketch with `PAPER-GO.ino` as entrypoint.
- [ ] Verify include resolution works without PlatformIO-only include behavior.
- [ ] Verify compile and runtime acceptance are evaluated in Arduino IDE (not PlatformIO as the definition of done).

## 28.2 Milestone A — Arduino IDE conversion

- [ ] A1.1 Ensure `PAPER-GO.ino` is the canonical sketch entrypoint and compiles.
- [ ] A1.2 Ensure setup/loop handoff invokes real app lifecycle and services.
- [ ] A1.3 Confirm no duplicate/parallel firmware tree is introduced.
- [ ] A2.1 Remove PlatformIO-only statements from requirements of completion.
- [ ] A2.2 Keep `platformio.ini` optional-only, not required by docs/checks.
- [ ] A2.3 Update docs to reflect Arduino IDE as primary workflow.
- [ ] A3.1 Fix project header visibility for Arduino include search path.
- [ ] A3.2 Remove compile dependence on hidden PlatformIO `include/` behavior.
- [ ] A3.3 Confirm clean Arduino compile resolves `BoardConfig.h`, `BoardHAL.h`, `Services.h`, `Apps.h`, and peers.

## 28.3 Milestone B — E-paper display implementation

- [ ] B1.1 Replace default-build `epdiy.h` include path with LilyGo-EPD47 `epd_driver.h` path.
- [ ] B1.2 Remove default-path dependency on `EpdiyHighlevelState`, waveform structs, and `epd_hl_*` APIs.
- [ ] B1.3 Keep any optional epdiy path isolated and non-default.
- [ ] B2.1 Allocate 4-bit framebuffer in PSRAM.
- [ ] B2.2 Implement `BoardHAL::clear` against framebuffer.
- [ ] B2.3 Implement `BoardHAL::fillRect` against framebuffer.
- [ ] B2.4 Implement `BoardHAL::drawRect` against framebuffer.
- [ ] B2.5 Implement `BoardHAL::drawLine` against framebuffer.
- [ ] B2.6 Implement `BoardHAL::drawText` against framebuffer.
- [ ] B2.7 Implement `BoardHAL::beginFrame` lifecycle behavior.
- [ ] B2.8 Implement `BoardHAL::endFrame` to push framebuffer to panel.
- [ ] B2.9 Ensure framebuffer alloc failure is surfaced as a real failure.
- [ ] B3.1 Use full refresh path when required by policy.
- [ ] B3.2 Use partial refresh path only when allowed by dirty/refresh cadence logic.
- [ ] B3.3 Render visible boot splash proving physical panel update.

## 28.4 Milestone C — GT911 touch implementation

- [ ] C1.1 Initialize I2C pins for touch bus.
- [ ] C1.2 Probe GT911 at `0x14`.
- [ ] C1.3 Probe GT911 at `0x5D` as fallback.
- [ ] C1.4 Initialize touch driver and coordinate bounds to display geometry.
- [ ] C1.5 Apply proven swap/mirror orientation for this board.
- [ ] C2.1 Convert raw points into `TouchEvent` tap classification.
- [ ] C2.2 Convert raw points into long-press classification.
- [ ] C2.3 Convert raw points into swipe-up/down/left/right classification.
- [ ] C2.4 Convert raw points into drag classification.
- [ ] C2.5 Integrate touch with launcher/app navigation flows.

## 28.5 Milestones D–N delivery tasks

- [ ] D.1 Validate app layout, status bar geometry, and safe drawing bounds for 960x540.
- [ ] E.1 Implement SD mount and failure handling.
- [ ] E.2 Create/validate storage directory contract from section 3.3.
- [ ] F.1 Integrate battery/charger readout via real board interfaces.
- [ ] F.2 Integrate RTC time source and sync behavior.
- [ ] F.3 Implement low-power/lock power transitions.
- [ ] G.1 Deliver Settings app controls for display/network/GPS/cache/power toggles.
- [ ] H.1 Deliver File Explorer browsing/sorting/open behavior.
- [ ] H.2 Deliver Markdown Reader preview, scrolling, and saved position.
- [ ] I.1 Deliver Wi-Fi manager credential flows and connection state handling.
- [ ] I.2 Deliver URL Fetcher request/config/cache/offline fallback behavior.
- [ ] I.3 Deliver Weather app rendering and stale/fresh cache behavior.
- [ ] I.4 Deliver onboard Web Server static/API behavior.
- [ ] J.1 Deliver GPS service, fix state, and app-facing location pipeline.
- [ ] J.2 Deliver map tile renderer/cache/offline map state behavior.
- [ ] K.1 Deliver Wi-Fi/BLE/LoRa scan views and scan logging.
- [ ] L.1 Deliver Meshtastic-compatible node/message flows within implementation boundary.
- [ ] M.1 Deliver Chess, Go, Tic-Tac-Toe, and Minesweeper app behavior.
- [ ] N.1 Perform final hardening (performance, stability, power, storage, and UX checks).

## 28.6 Requirements section taskization (Sections 1–27)

- [ ] R1 Project identity and non-goals codified in docs and app behavior.
- [ ] R2 Hardware target support verified for board, display, touch, storage, radio, and power.
- [ ] R3 Firmware architecture implemented: app shell, stack, storage layout.
- [ ] R4 Display/UI rules implemented: e-paper visual style, touch, shared gestures.
- [ ] R5 Persistent status bar implemented with required indicators and time-source priority.
- [ ] R6 Lock screen implemented with time/date/power/GPS/location preview and low-power behavior.
- [ ] R7 Springboard launcher implemented with layout, ordering, and app-state affordances.
- [ ] R8 GPS Map app implemented with heading, best-fit location, caching, and offline behavior.
- [ ] R9 Radio Scanner app implemented for Wi-Fi/BLE/LoRa and scan-log persistence.
- [ ] R10 Meshtastic app implemented for node/message/storage requirements.
- [ ] R11 URL Fetcher app implemented for configured fetch, render, cache, and fallback.
- [ ] R12 Markdown Reader implemented with feature support and bounded rendering.
- [ ] R13 File Explorer implemented with browsing, sort, and safety constraints.
- [ ] R14 Weather app implemented with required display fields and cache semantics.
- [ ] R15 Web Server implemented with webroot serving and required API surface.
- [ ] R16 Games folder and shared game requirements implemented.
- [ ] R17 Network manager implemented with config, runtime use, and offline-first behavior.
- [ ] R18 GPS manager implemented with shared service, logging, and privacy gates.
- [ ] R19 Cache manager implemented with map priority and cache controls.
- [ ] R20 Power management implemented with power modes and battery display.
- [ ] R21 Settings app implemented with device + developer settings.
- [ ] R22 Data persistence implemented with config storage + internal fallback.
- [ ] R23 Logging requirements implemented for boot/services/app/network/runtime diagnostics.
- [ ] R24 Security requirements implemented for credentials, web server, and location data.
- [ ] R25 Feasibility constraints reflected in implementation decisions and docs.
- [ ] R26 Suggested phases mapped to concrete execution order in tracker.
- [ ] R27 Initial app list verified in shipped firmware launcher.

## 28.7 API, spec, and test execution tasks

- [ ] T1 Add/update unit tests for each new feature before marking feature task complete.
- [ ] T2 Update existing tests when behavior changes; add regression tests for fixed defects.
- [ ] T3 Keep `docs/openapi.yaml` synchronized for every API contract change.
- [ ] T4 Verify every documented API endpoint in automated tests.
- [ ] T5 Keep README API/CLI usage examples aligned with implemented behavior.

## 28.8 Release completion gate (must all be true)

- [ ] GATE.1 Arduino IDE clean compile succeeds on target board profile.
- [ ] GATE.2 Core interaction loop works on-device (launcher, touch, nav, app open/close).
- [ ] GATE.3 Display updates visibly and correctly with e-paper refresh policy.
- [ ] GATE.4 SD, network, GPS, radio, and cache subsystems pass runtime checks.
- [ ] GATE.5 Unit/integration/API tests pass for modified scope.
- [ ] GATE.6 OpenAPI, README, and requirements tracker reflect final delivered behavior.


---

## 3. GPS service + DGPS rover integration requirements (full implementation)

This section is the authoritative implementation contract for turning the current GPS service and GPS map app into real, field-usable features (not scaffolds/stubs).

### 3.1 Core rule: GPS must always be DGPS-aware

When GPS is enabled, firmware must always poll for DGPS correction packets in parallel.

Required runtime behavior:

1. GPS parsing must run continuously (TinyGPSPlus feed loop).
2. LoRa DGPS receiver must run continuously while GPS is active.
3. GPS-only mode and DGPS mode must auto-switch seamlessly based on DGPS packet availability/quality.
4. User must not need to reboot or manually toggle modes when DGPS appears/disappears.
5. UI must explicitly show whether output position is RAW GPS or DGPS-corrected.

### 3.2 Required DGPS data path

Implement and wire all of the following paths end-to-end:

1. Radio receive bytes -> `handleReceivedCorrection(...)` validation path.
2. Valid packet state -> correction state update (`RoverCorrectionState`).
3. Rover GPS fix -> ENU conversion relative to base origin.
4. Correction-age prediction -> corrected E/N/U output.
5. Optional corrected lat/lon back-projection for map marker display.
6. Quality-state evaluation -> UI and logging labels.

Do not keep DGPS logic disconnected in a helper file only; it must be used by `GPSService` and exposed to app/UI layers through real service fields/APIs.

### 3.3 GPSService required fields and APIs

`GPSService` must expose concrete runtime state for both raw GPS and DGPS outputs.

Minimum required service state:

- `rawFix` (live TinyGPSPlus-derived fix)
- `correctedFix` (DGPS-corrected fix when available)
- `usingDgps` (true only when corrected output is currently active)
- `dgpsQualityState` (`NO_ROVER_FIX`, `NO_BASE`, `BASE_STALE`, `BASE_BAD`, `DGPS_DEGRADED`, `DGPS_GOOD`, `DGPS_BEST`)
- `dgpsAgeMs`
- `dgpsLastSeq`
- `dgpsPacketsReceived`
- `dgpsPacketsMissed`
- `dgpsPacketsDuplicate`
- `dgpsPacketsOutOfOrder`
- `dgpsPacketsCrcFail`
- `dgpsRssi`
- `dgpsSnr`

Minimum required API surface:

- getter for active navigation fix (returns corrected fix when `usingDgps`, else raw fix)
- getter for raw fix
- getter for corrected fix validity
- getter for DGPS mode/quality/age
- getter for DGPS packet stats

### 3.4 Seamless mode-switch policy

Implement this exact switching policy:

1. If rover GPS fix is invalid/stale -> `NO_ROVER_FIX`, no corrected output.
2. If rover GPS is valid and no recent base packets -> `NO_BASE`, use raw GPS.
3. If base packets are stale (>5000 ms) -> `BASE_STALE`, use raw GPS.
4. If base packets decode but fail quality -> `BASE_BAD`, use raw GPS.
5. If base quality passes and correction computes:
   - `DGPS_DEGRADED` for age 2001..5000 ms
   - `DGPS_GOOD` for age 0..2000 ms and normal quality
   - `DGPS_BEST` when preferred thresholds are met
6. In `DGPS_*` states, navigation output must use corrected position.
7. Transition between raw and corrected modes must not clear track history or crash map rendering.

### 3.5 GPS Map app requirements

`GPS Map` must stop using placeholder-only labels and must render real service outputs.

Required map/app fields on-screen:

1. Mode label: `GPS` or `DGPS` (prominent, always visible).
2. Quality label from `DgpsQualityState`.
3. Raw lat/lon and active lat/lon (active == corrected when DGPS enabled).
4. DGPS age in seconds.
5. DGPS LoRa RSSI/SNR.
6. Base sats/HDOP from last packet when available.
7. Rover sats/HDOP from live fix.
8. Packet counters: received/missed/duplicate/CRC fail (diagnostic screen acceptable).

Acceptance criteria:

- Walking in/out of DGPS coverage updates mode label automatically.
- App clearly indicates when it falls back to raw GPS.
- No fake "DGPS" indicator may appear without active correction.

### 3.6 Logging requirements (must be implemented, not planned)

Implement real CSV logging for DGPS workflows.

Required files:

1. `/logs/corrections_YYYYMMDD.csv`
2. `/logs/rover_positions_YYYYMMDD.csv`
3. `/logs/events_YYYYMMDD.csv`

Required correction log columns:

`rover_ms,base_seq,origin_seq,base_utc,age_ms,rssi,snr,flags,base_hdop,base_fix_age_ms,base_sats_used,base_sats_view,raw_e_cm,raw_n_cm,raw_u_cm,filt_e_cm,filt_n_cm,filt_u_cm,rate_e_cm_s,rate_n_cm_s,rate_u_cm_s,origin_lat_e7,origin_lon_e7,origin_alt_cm,origin_samples,filter_samples,crc_ok`

Required rover position log columns:

`rover_ms,rover_utc,quality_state,raw_lat,raw_lon,raw_alt_m,raw_hdop,raw_sats,raw_fix_age_ms,base_origin_lat,base_origin_lon,base_origin_alt_m,live_e_cm,live_n_cm,live_u_cm,corr_e_cm,corr_n_cm,corr_u_cm,corr_lat,corr_lon,corr_alt_m,base_seq,base_age_ms,rssi,snr`

Required event log events:

`BOOT,BASE_ACQUIRED,BASE_LOST,BASE_ORIGIN_CHANGED,ROVER_ORIGIN_LOCK,ROVER_ORIGIN_CLEAR,LOG_START,LOG_STOP,GPS_FIX_GAINED,GPS_FIX_LOST`

### 3.7 API and web endpoint requirements for GPS/DGPS status

If web APIs are enabled in firmware, these endpoints are required and must be real (not fixed JSON):

1. `GET /api/gps/status`
   - raw fix validity + fields
   - active fix fields
   - `usingDgps`
   - DGPS quality state
   - DGPS age and radio metrics
   - packet counters
2. `GET /api/gps/dgps`
   - last correction packet header/quality subset
   - origin coordinates
   - ENU correction vector and predicted vector

If either endpoint is added/changed, update `docs/openapi.yaml` in the same change and add/update API unit tests to validate response shapes and required fields.

### 3.8 Failure handling requirements

Required behavior under failures:

1. CRC failure increments counters and must not update active correction.
2. Bad header/version/type must be rejected and counted.
3. Quality-failed packets may update recent-base diagnostics but must not drive corrected navigation output.
4. Rover GPS stale invalidates corrected output immediately.
5. Base origin change clears rover-origin lock and emits `BASE_ORIGIN_CHANGED` event.

### 3.9 Required acceptance checks

Implementation is complete only when all checks pass:

1. Device can run in raw GPS mode with no base present.
2. Device auto-switches to DGPS when valid corrections arrive.
3. Device auto-falls back to raw GPS when corrections are stale/invalid.
4. GPS map visibly indicates mode and quality transitions in real time.
5. CSV logs contain real runtime values and rotate by date.
6. Web API endpoints report live GPS/DGPS state (if web server enabled).
7. Unit tests cover:
   - packet validation and CRC
   - sequence/rollover/missed/duplicate/out-of-order
   - corrected ENU computation
   - mode switching logic
   - quality-state transitions
   - API response fields (if API endpoints exist)


---

## 4. Offline maps full implementation specification (GPS Map feature contract)

This section defines the complete requirements to realize Offline Maps as a production feature.

### 4.1 Scope and goals

Offline Maps must provide usable field navigation when network is unavailable.

Required outcomes:

1. GPS Map renders real local map imagery or vector-derived raster tiles from SD cache.
2. Current position marker updates in real time from active fix (`DGPS` when active, raw GPS otherwise).
3. Map remains usable without Wi-Fi for previously cached regions.
4. UI clearly indicates cache coverage quality and stale/missing map data.

### 4.2 Supported map source and storage model

Firmware must support SD-backed tile cache under:

- `/cache/maps/<provider>/<z>/<x>/<y>.tile`

Provider requirements:

1. `default` provider must be supported.
2. Provider identifier must be configurable from `/config/maps.json`.
3. Unknown providers must fallback to `default` and log warning.

Tile file requirements:

1. `.tile` files must be real renderable payloads (PNG/JPEG/raw-gray agreed format), not placeholders.
2. If compressed image tiles are used, decode path must be implemented on-device.
3. Corrupt tile reads must be detected and counted as cache miss.

### 4.3 Required `/config/maps.json` schema

`/config/maps.json` must support:

- `provider` (string)
- `zoom` (integer default zoom)
- `minZoom` (integer)
- `maxZoom` (integer)
- `prefetchRadius` (integer tile radius)
- `offlineOnly` (bool)
- `tileSizePx` (integer, default 256)

Validation rules:

1. `minZoom <= zoom <= maxZoom`
2. zoom range must be clamped to implementation-supported levels.
3. Invalid config must fallback to safe defaults and emit config error log.

### 4.4 Coordinate and tile math requirements

Implement deterministic conversion pipeline:

1. WGS84 lat/lon -> Slippy-map tile x/y at zoom.
2. Active viewport center tile from active fix.
3. Neighbor tiles sampled by configured radius.
4. Sub-tile pixel offset for smooth marker placement.

Accuracy requirements:

1. Marker must remain spatially consistent while panning due to movement.
2. Crossing tile boundaries must not cause marker jumps > 1 tile error.

### 4.5 Rendering requirements (e-paper aware)

Required rendering stack:

1. Assemble viewport from multiple cached tiles around center.
2. Draw current-position marker and heading arrow (when heading reliable).
3. Draw DGPS status overlay (Mode/Quality/Age) without obscuring marker.
4. Draw scale indicator and north reference.

E-paper refresh policy:

1. Partial update for marker/overlay changes (target 1 Hz) where reliable.
2. Full refresh every configurable interval (default 10s) to control ghosting.
3. Force full refresh when tile set changes significantly.

### 4.6 Cache coverage and fallback behavior

Coverage states must be computed from requested tile set:

- `FULL` (all required tiles present)
- `PARTIAL` (some present)
- `MISS` (none present)

Required behavior:

1. `FULL`: normal render.
2. `PARTIAL`: render available tiles + hatched/no-data background for misses.
3. `MISS`: render clear offline-missing panel with nearest cached-area hint if available.

Do not draw fake map imagery when no tile data exists.

### 4.7 Online fetch and prefetch rules (when Wi-Fi available)

If `offlineOnly=false` and Wi-Fi connected:

1. Missing visible tiles may be fetched and written to cache.
2. Fetch queue must be bounded and non-blocking for UI thread.
3. Prefetch ring around current tile must honor `prefetchRadius`.
4. HTTP failures must not freeze map rendering.

If `offlineOnly=true`, no network tile fetch may be attempted.

### 4.8 Required map APIs

If web server is enabled, these endpoints are required for map feature realization:

1. `GET /api/maps/status`
   - provider, zoom, center tile
   - coverage state
   - tile requests/hits/misses/corrupt counters
   - offlineOnly mode
2. `POST /api/maps/prefetch`
   - accepts center lat/lon, zoom, radius
   - schedules prefetch job and returns job id/status
3. `GET /api/maps/cache`
   - reports cache size, file count, oldest/newest tile timestamps
4. `POST /api/maps/cache/clear`
   - clears provider cache (with optional provider filter)

Any map API changes require same-change updates to `docs/openapi.yaml` and unit tests.

### 4.9 Logging requirements for offline maps

Required log file:

- `/logs/map_cache_YYYYMMDD.csv`

Columns:

`ms,provider,z,x,y,event,source,result,lat,lon,dgps_mode,quality`

Where:

- `event`: lookup|fetch|decode|render
- `source`: cache|network
- `result`: hit|miss|corrupt|ok|error

### 4.10 GPS/DGPS integration requirements for map mode

Map center and marker must always use GPSService active fix:

1. If DGPS available/valid -> active fix is corrected fix.
2. Else active fix is raw fix.
3. Overlay must show `GPS` or `DGPS` explicitly.
4. Quality state label must match `DgpsQualityState`.

No duplicate/parallel ad-hoc GPS source may be used by map renderer.

### 4.11 Error handling and resilience

Required safeguards:

1. Missing SD card -> map app must show storage error state, not crash.
2. Corrupt tile decode -> drop tile, increment corrupt counter, continue rendering.
3. Invalid map config -> fallback defaults + warning log.
4. Memory pressure while composing viewport -> degrade tile radius first, then show fallback panel.

### 4.12 Acceptance criteria (feature complete)

Offline maps are complete only when:

1. With Wi-Fi disabled, previously cached region renders correctly while moving.
2. With DGPS corrections present, marker follows DGPS-corrected active fix and overlay shows `DGPS`.
3. Coverage transitions FULL/PARTIAL/MISS are visible and accurate.
4. Tile corruption and cache misses are logged and exposed in map status API.
5. App remains responsive during fetch/prefetch operations.
6. OpenAPI spec documents all implemented map endpoints.
7. Unit tests cover:
   - tile math conversion
   - cache coverage classification
   - config validation/fallback
   - API response fields for map endpoints
   - DGPS mode label usage in map status payload

### 4.13 Required implementation tasks

1. Implement map config parser/validator for `/config/maps.json`.
2. Implement tile decode/render path for `.tile` payload format used by firmware.
3. Implement viewport tile compositor + marker overlay.
4. Implement cache/fetch queue manager with bounded work budget per loop.
5. Implement map APIs and OpenAPI spec updates.
6. Implement map cache CSV logging.
7. Add/expand unit tests for all above modules.


---

## 5. Full product feature realization spec (non-scaffold completion contract)

This section defines completion specs for all major shipped features so the firmware can be delivered as a fully working product rather than scaffolding.

### 5.1 Springboard launcher (production spec)

Required behavior:

1. Load app order from `/config/apps.json` with validation and deterministic fallback.
2. Render paged app grid with visible selection focus and page indicators.
3. Support tap-to-open, swipe page navigation, and long-press app options.
4. Persist user app order edits back to `/config/apps.json`.
5. Show app health badges (e.g., unread messages, config errors) when available.

Acceptance:

- Reordering survives reboot.
- Invalid app ids are ignored without crash and logged.

### 5.2 Lock screen (production spec)

Required behavior:

1. Render UTC/local time, date, battery/charging, GPS/DGPS mode, and connectivity indicators.
2. Show real map preview state from offline map cache logic.
3. Auto-enter after lock timeout and recover to prior app on unlock.
4. Support unlock by configured gesture/button.

Acceptance:

- Lock policy follows `/config/power.json`.
- No stale/fake status labels shown when backing services unavailable.

### 5.3 GPS + DGPS service (production spec)

Required behavior:

1. Continuous GPS parse loop and DGPS polling loop.
2. Automatic mode selection between raw GPS and DGPS corrected output.
3. Expose active fix, raw fix, DGPS quality, age, radio, and packet diagnostics.
4. Persist GPS track logs and DGPS correction logs with required schemas.

Acceptance:

- DGPS mode transitions are visible and deterministic.
- Invalid packets never update active correction output.

### 5.4 Offline maps app (production spec)

Required behavior:

1. Render cache-backed tile viewport and marker from active fix.
2. Show FULL/PARTIAL/MISS coverage state and tile diagnostics.
3. Support provider/zoom config from `/config/maps.json`.
4. Support cache inspection and clear via APIs.

Acceptance:

- Moving across cached region updates viewport consistently.
- Missing tiles produce explicit no-data visuals, not fake map imagery.

### 5.5 Radio scanner (production spec)

Required behavior:

1. Perform real Wi-Fi/BLE/LoRa scans from radio config.
2. Render sortable scan list with RSSI/protocol/age fields.
3. Persist scans to `/radio/scans/scan-<ms>.log` with GPS coordinates when available.
4. Expose radio control/status via API with validation.

Acceptance:

- Scan toggles in `/config/radio.json` directly affect runtime scan behavior.

### 5.6 Meshtastic/LoRa messaging boundary (production spec)

Required behavior:

1. Persist nodes/messages/config under `/meshtastic/*` paths.
2. Provide message list UI with unread/read state and timestamps.
3. Expose storage stats and unread count to status API.
4. Implement clear scoped behavior when full Meshtastic interoperability is not enabled.

Acceptance:

- No placeholder-only message screen in release mode.

### 5.7 Weather feature (production spec)

Required behavior:

1. Read weather provider config from `/config/weather.json`.
2. Fetch weather by live GPS location when Wi-Fi is available.
3. Cache parsed response in `/cache/weather/current.json`.
4. Render fresh/stale status and last update timestamp.

Acceptance:

- Network failures fall back to cache with explicit stale labeling.

### 5.8 URL fetcher (production spec)

Required behavior:

1. Read template/config from `/config/url_fetcher.json`.
2. Resolve placeholders (`{lat}`, `{lon}`, optional others) safely.
3. Fetch and cache response under `/cache/http`.
4. Render summarized body with size/error metadata.

Acceptance:

- Invalid templates are rejected with explicit user-facing error.

### 5.9 Markdown/document reader (production spec)

Required behavior:

1. Browse markdown files from `/documents/markdown`.
2. Render headings, lists, links text, code fences, and tables (bounded).
3. Support pagination/scroll and file reload.
4. Protect against oversized file memory exhaustion with graceful truncation.

Acceptance:

- Reader remains responsive on large docs and never crashes on malformed markdown.

### 5.10 File explorer (production spec)

Required behavior:

1. Browse SD filesystem within allowed roots.
2. Sort by name/date/size and show metadata.
3. Open supported file types via associated apps.
4. Prevent unsafe operations outside allowed roots.

Acceptance:

- Explorer actions never escape sandboxed SD roots.

### 5.11 Games suite (production spec)

Required behavior:

1. Ship playable Chess, Go, Tic-Tac-Toe, Minesweeper UIs.
2. Enforce core rules and illegal-move prevention.
3. Support save/resume where defined.
4. Render stable touch interaction and turn indicators.

Acceptance:

- Games are playable end-to-end without placeholder notices.

### 5.12 Settings app (production spec)

Required behavior:

1. Provide editable sections for Wi-Fi, GPS/DGPS, LoRa/radio, maps, power, cache, web server.
2. Persist validated config to `/config/*.json`.
3. Show applied vs pending values and commit result.
4. Expose reset-to-default actions per section.

Acceptance:

- Settings edits take effect without reboot when subsystem supports live reload.

### 5.13 Web server/API feature parity (production spec)

Required behavior:

1. Serve static files from `/webroot` with content-type handling.
2. Serve all documented `/api/*` endpoints with live runtime data.
3. Keep `docs/openapi.yaml` authoritative and synchronized.
4. Return structured error payloads for unavailable contexts.

Acceptance:

- Every path in OpenAPI has automated unit coverage for presence and key response fields.

### 5.14 Data persistence and logging (production spec)

Required behavior:

1. Create and maintain required SD layout on boot.
2. Write append-safe logs for GPS, DGPS, radio, map cache, boot/runtime events.
3. Rotate logs by date and bound file growth.
4. Recover gracefully from SD remove/reinsert conditions.

Acceptance:

- No silent data loss when SD is present and writable.

### 5.15 Release quality gates (must pass together)

1. No critical feature screen shows scaffold/placeholder/not-implemented messaging in release mode.
2. GPS Map, Radio, Weather, URL Fetcher, File Explorer, Settings, and Games are all user-operable end-to-end.
3. DGPS and offline map APIs return live data matching docs.
4. OpenAPI + README + requirements are in sync with shipped behavior.
5. Unit tests cover changed logic and API/spec contracts for every merged feature change.


---

## 6. Completion integrity policy (checked items require full working implementation)

This section is mandatory and supersedes any implied "partial completion" interpretation in checklist sections.

### 6.1 Definition of complete (strict)

A requirement/checklist item may be marked complete (`[x]`) only when all of the following are true:

1. Real working code exists in production paths (not test-only, not mock-only).
2. Feature is reachable through real UI/API flows on device/runtime.
3. Failure paths are handled with explicit user-visible or API-visible error behavior.
4. Persistence/logging required by the requirement is implemented.
5. Required tests exist and pass for changed behavior.
6. Docs/specs (README/OpenAPI/requirements) match shipped behavior.

If any condition is unmet, item must remain `[ ]`.

### 6.2 Forbidden completion patterns

The following must never justify marking an item `[x]`:

1. Placeholder text replacing real feature behavior.
2. Stub implementation that only prints logs/returns constants.
3. Mocked UI showing fake data without connected service logic.
4. API endpoint that returns fixed/dummy payload unrelated to runtime state.
5. TODO comments without implementation.
6. Test-only implementation not wired into main runtime.

### 6.3 Retroactive checklist correctness rule

For every checklist section (including previously checked items):

1. Each `[x]` item must have concrete specification details in this file.
2. Each `[x]` item must map to concrete source files implementing behavior.
3. If implementation is partial/scaffolded, item must be reverted to `[ ]` until complete.

### 6.4 Required spec detail minimum for checked items

Any checked item must document at minimum:

1. Runtime entry points (UI route/API endpoint/service loop).
2. Input/config sources (files, payloads, touch/button actions).
3. Output/side effects (screen changes, logs, persisted files, API fields).
4. Error handling behavior.
5. Verification method (tests and/or on-device acceptance checks).

### 6.5 Merge/review gate

Before merging a task that updates requirements checkboxes:

1. Reviewer must verify no `[x]` item depends on stub/mock/placeholder-only code.
2. Reviewer must verify relevant checklist item includes spec detail minimum (Section 6.4).
3. Reviewer must verify tests exist for changed behavior and docs match code.

If any gate fails, checklist changes must be rejected.

### 6.6 Priority remediation rule for existing checked items

If existing checked items are found to be incomplete:

1. Add missing spec details immediately.
2. Re-open items to `[ ]` when code is not fully implemented.
3. Track remediation tasks until item can be honestly re-checked.

No item may remain checked while known to be scaffold-only.


---

## 7. Requirements execution framework (how to complete requirements end-to-end)

This section defines the required workflow for turning each requirement into delivered code.

### 7.1 Requirement lifecycle states

Each requirement item must explicitly track one of these states:

1. `UNSTARTED`
2. `IN_PROGRESS`
3. `BLOCKED` (with blocker reason and owner)
4. `VERIFYING`
5. `COMPLETE`

Rules:

- `COMPLETE` is allowed only when Section 6 completion integrity conditions are satisfied.
- `BLOCKED` items must include concrete blocker details and next action.

### 7.2 Required implementation packet per feature

For every feature requirement, maintain an implementation packet containing:

1. **Spec slice**: exact behavior contract copied from requirements section.
2. **Code map**: source files/modules that implement behavior.
3. **Runtime path**: how user reaches feature (UI route/API endpoint/background loop).
4. **Persistence path**: files/config/logs read/written.
5. **Failure behavior**: explicit expected user/API response for failures.
6. **Verification plan**: tests + on-device acceptance checks.

A feature cannot move to `VERIFYING` without a complete implementation packet.

### 7.3 Test and API spec synchronization policy

When changing behavior:

1. Add/update unit tests before marking complete.
2. If APIs change, update `docs/openapi.yaml` in the same change.
3. Add/update tests verifying changed API paths and key response fields.
4. Update README API/CLI command examples in same change.

### 7.4 On-device validation minimum for hardware-bound features

For hardware-coupled features (display/touch/GPS/radio/power/storage):

1. Validate startup path on real device.
2. Validate one nominal scenario and one failure scenario.
3. Validate no crash/reset across 10+ minutes of operation under feature load.

Required recorded evidence in notes:

- firmware build identifier,
- scenario steps,
- observed result,
- known limitations.

### 7.5 Regression lock policy

After a requirement reaches `COMPLETE`:

1. Add/retain regression tests for key behavior contracts.
2. Any future bug fix touching feature must include regression test updates.
3. If regression is discovered, item re-enters `IN_PROGRESS` until fixed and re-verified.

### 7.6 Completion checklist template (must be attached per completed item)

- [ ] Requirement behavior implemented in production runtime path.
- [ ] No placeholder/stub/mock behavior remains.
- [ ] Error handling paths implemented and observable.
- [ ] Persistence/logging requirements implemented.
- [ ] Unit tests added/updated and passing.
- [ ] OpenAPI updated (if API touched).
- [ ] README API/CLI docs updated.
- [ ] On-device acceptance checks recorded.

### 7.7 Priority execution order for remaining work

To realize a fully working product, execute remaining requirements in this order:

1. **Platform integrity**: display, touch, storage, power, boot/runtime stability.
2. **Navigation core**: GPS/DGPS correctness, offline maps rendering, map cache/fetch.
3. **Operational tools**: radio scanner, web APIs, logging/diagnostics.
4. **User workflows**: settings editor, file/document flows, weather/url integrations.
5. **App polish**: launcher interactions, lock screen UX, games completeness.
6. **Release hardening**: performance, memory, long-run stability, docs parity.

### 7.8 Non-completion disclosure rule

If any requirement is partially implemented, release notes and requirements status must explicitly state:

1. what is implemented,
2. what is missing,
3. what blocks completion,
4. what test coverage exists today.

Do not represent partially implemented features as complete.


---

## 8. File management feature requirements (full implementation spec)

This section defines complete requirements for file management across File Explorer, document apps, logs, and API-accessible storage operations.

### 8.1 Scope

File management includes:

1. Browsing and inspecting files/directories on SD card.
2. Controlled create/rename/move/delete operations within allowed roots.
3. Opening files in associated apps.
4. Exposing safe file-management APIs for remote tooling.
5. Logging file operations for audit/debugging.

### 8.2 Allowed roots and sandbox boundaries

All file operations must be restricted to approved roots:

- `/apps`
- `/cache`
- `/documents`
- `/games`
- `/gps`
- `/meshtastic`
- `/radio`
- `/config`
- `/logs`
- `/webroot`

Rules:

1. Path normalization must reject traversal attempts (`..`, duplicated separators, hidden parent hops).
2. Absolute paths outside approved roots must be rejected.
3. API and UI must return explicit permission errors for blocked paths.

### 8.3 File Explorer app requirements

Required behavior:

1. Directory listing with file type, size, and last-modified timestamp.
2. Sorting modes: name asc/desc, size asc/desc, modified asc/desc.
3. Filter modes: all/files/dirs and extension filter.
4. Operations menu per entry: open, rename, delete, move/copy (where supported).
5. Breadcrumb navigation and parent traversal within sandbox roots.
6. Create directory and create text file actions.

Acceptance:

- Explorer never freezes on large directories; paginate or chunk listing.
- Operations provide success/failure feedback with reason.

### 8.4 File association requirements

File-open routing must map by extension:

- `.md` -> Markdown Reader
- `.txt`/`.log`/`.csv` -> text viewer mode
- map cache tile extensions -> map diagnostic preview mode
- unknown -> metadata-only inspector with hex/text preview limits

Rules:

1. Unsupported file types must still be inspectable (metadata + bounded preview).
2. Opening large files must not exhaust memory.

### 8.5 Safe mutation requirements

For rename/move/copy/delete:

1. Validate source and destination paths in sandbox.
2. Prevent overwrite unless explicit replace flag is provided.
3. For directory delete, support recursive delete with confirmation.
4. For copy/move, support file-size guard and incremental progress state.
5. Handle partial failure with rollback or explicit partial-result report.

### 8.6 File management web APIs

If web server is enabled, required endpoints:

1. `GET /api/files/list?path=<...>&sort=<...>&order=<...>&filter=<...>`
2. `GET /api/files/meta?path=<...>`
3. `POST /api/files/mkdir`
4. `POST /api/files/write` (bounded text payloads)
5. `POST /api/files/rename`
6. `POST /api/files/move`
7. `POST /api/files/copy`
8. `POST /api/files/delete`

Response requirements:

- consistent `{ ok, error?, data? }` envelope
- explicit error codes: `invalid_path`, `forbidden_path`, `not_found`, `already_exists`, `io_error`, `payload_too_large`

Any added/changed endpoints require same-change updates to `docs/openapi.yaml` and API tests.

### 8.7 Logging and audit requirements

Required file:

- `/logs/file_ops_YYYYMMDD.csv`

Columns:

`ms,op,path,dst_path,result,error_code,size_before,size_after`

Operations logged:

`list,meta,mkdir,write,rename,move,copy,delete,open`

### 8.8 Performance and reliability requirements

1. Large directory listing (>1000 entries) must be chunked/paginated.
2. File preview reads must be bounded by configurable max bytes.
3. Long operations (recursive delete/copy) must yield to main loop between chunks.
4. SD transient failures must produce recoverable errors, not crashes.

### 8.9 Security and privacy requirements

1. Deny access to secret-bearing files when policy requires redaction (e.g., credential files in API list views).
2. Redact sensitive values when returning config file previews (`wifi password`, tokens).
3. Require explicit confirmation token for destructive recursive delete through API.

### 8.10 Acceptance criteria

File management is complete only when:

1. Explorer supports browse/sort/filter/open/create/rename/delete/move/copy inside sandbox roots.
2. Web file APIs enforce sandbox rules and return structured errors.
3. Recursive operations are safe and non-blocking.
4. File operation logs are written with required schema.
5. OpenAPI documents all shipped file APIs.
6. Unit tests cover path normalization/sandbox enforcement and operation result handling.

### 8.11 Required tests

Minimum unit/integration tests:

1. path normalization and traversal rejection tests.
2. allowed-root enforcement tests.
3. rename/move/copy/delete success + failure cases.
4. recursive delete behavior and partial-failure reporting tests.
5. API payload/response shape tests for file endpoints.
6. OpenAPI spec presence tests for all file API paths.


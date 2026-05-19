# T5 Field OS

Firmware scaffold for the **LILYGO T5-4.7-S3 E-Paper Pro ESP32-S3 SX1262 LoRa 915 MHz TF Card RTC GPS board**.

This is a separate handheld e-paper app launcher project. It is not the garden watering valve controller and not the e-ink irrigation schedule display.

## What is included

- Springboard launcher
- Persistent status bar with explicit GPS state labels (off/search/2D/3D/stale/no data) and time source label (RTC/GPS/NET/SAVED)
- Lock screen
- GPS service and GPS map app scaffold
- SD-card cache layout
- Weather app with cache scaffold
- URL fetcher app scaffold
- Markdown reader app scaffold
- File explorer app scaffold
- Radio scanner for Wi-Fi / BLE / LoRa scaffold
- Meshtastic placeholder app with correct storage boundaries
- SD-hosted web server scaffold
- Games folder with Chess, Go, Tic-Tac-Toe, and Minesweeper entries plus playable core game logic modules for all four games (rules validation/state progression), with Tic-Tac-Toe and Chess save/resume serialization
- Settings app
- Board abstraction layer so hardware-specific display/touch/pin details stay isolated

## Current hardware integration status

This project intentionally isolates the board-specific display/touch/power pins in `include/BoardConfig.h` and `src/BoardHAL.*`.

The LILYGO T5-S3 Pro display stack is not a normal SPI e-paper panel. It uses the ED047TC1 960x540 16-gray panel with the board-specific driver stack. Before flashing as final firmware, align `BoardHAL` with the official LILYGO `T5S3-4.7-e-paper-PRO` examples or a proven ED047TC1-compatible library such as epdiy/FastEPD for this board revision.

The app framework, storage layout, network services, radio scanner scaffolding, and app lifecycle are written as the firmware foundation.

## SD card layout

On first boot the firmware creates:

```text
/apps
/cache/maps
/cache/weather
/cache/http
/documents/markdown
/documents/text
/webroot
/games
/gps/tracks
/gps/fixes
/meshtastic/messages
/meshtastic/nodes
/radio/scans
/config
/logs
```

## Build

### PlatformIO

```bash
pio run
pio run -t upload
pio device monitor
```

### Arduino IDE (Arduino Studio)

1. Open the `PAPER-GO` folder as a sketch in Arduino IDE.
2. Select an ESP32-S3 board profile that matches the LILYGO T5 4.7 S3 Pro hardware.
3. Install required board support and libraries from the ESP32 core/library manager as needed by your toolchain.
4. Build/Upload from Arduino IDE.

This repository includes a root `PAPER_GO.ino` sketch entry and root-level header shims so Arduino IDE can resolve headers that are stored under `include/`.

### Boot diagnostics serial output

Use monitor to verify boot-step state transitions from firmware:

```bash
pio device monitor --baud 115200
```

Expected boot log prefixes:
- `[BOOT]` for one-time initialization results (board/cache/gps/net/radio/web).
- `[NET]` for periodic connection-state snapshots (`wifi`, `ssid`, `ip`).

## Notes

- `BoardHAL::drawText()` currently writes to Serial and can be wired to the real e-paper renderer.
- Touch input currently has a stub event provider and is structured for GT911 integration.
- GPS defaults to a secondary UART but pins must be confirmed against the exact board revision.
- LoRa defaults are placeholders; wire to the actual SX1262 pins from the LILYGO schematic/examples.


## Test commands

```bash
pio test -e lilygo_t5_s3_pro_test
```

## Launcher configuration

Set app ordering in `/config/apps.json`:

```json
{
  "order": ["gpsmap", "radio", "weather", "web", "games", "settings"]
}
```

Unknown app IDs are ignored, duplicates are removed, and missing apps are appended automatically.


## Wi-Fi configuration

Create `/config/wifi.json` on SD to enable automatic station connection at boot:

```json
{
  "ssid": "YourNetwork",
  "password": "YourPassword"
}
```

Behavior:
- Boot: `NetworkService::begin()` loads `/config/wifi.json` and starts `WiFi.begin(ssid,password)` when valid config exists.
- Manual connect path: `NetworkService::connect(ssid, pass)` updates the same file for persistence.
- Forget network: `NetworkService::forgetSaved()` clears stored credentials in `/config/wifi.json`.

## Web server API

When Web Server app is running, these HTTP endpoints are available:

- `GET /api/health` → health JSON for web service.
- `GET /api/apps/order` → returns `/config/apps.json` contents if present.
- `POST /api/apps/install` → installs app manifest to `/apps/<id>.json` (`id` required; optional `sourceUrl`, `version`).
- `POST /api/apps/remove` → removes installed app manifest from `/apps/<id>.json`.
- `POST /api/apps/update` → updates existing app manifest in `/apps/<id>.json`.
- `GET /api/status` → current device snapshot (wifi/ip/gps/battery/sd/web flags plus `unreadMessages` and `cacheActivity` indicators).
  - `unreadMessages` is `true` when one or more files are present in `/meshtastic/messages`.
  - `cacheActivity` is `true` when map-cache lookups occurred within the last 5 minutes.
- `GET /api/weather/cache` → returns `/cache/weather/current.json` when cached weather exists.
- `GET /api/cache/stats` → map cache hit/miss counters.
- `GET /api/radio/scans` → lists files under `/radio/scans`.
- `GET /api/meshtastic/stats` → Meshtastic message/node file counters.
- `GET /api/gps/tracks` → lists saved GPS track files under `/gps/tracks` with file sizes (response names are normalized to file basenames).
- `POST /api/gps/tracks/clear` → clears GPS track history by deleting files under `/gps/tracks`.
- `GET /api/power/policy` → effective power policy (`lockTimeoutMs`, `deepSleepTimeoutMs`, `allowDeepSleep`, `deepSleepDurationSec`) and `configPresent`.

SD-backed endpoints return `503` with `{"error":"sd not mounted"}` when the SD card is unavailable.
`GET /api/power/policy` returns `503` with `{"error":"cache context unavailable"}` when cache context is not attached.
- `GET /...` → static files served from `/webroot` with content-type detection.

Example command:

```bash
curl http://<device-ip>/api/health
```

## Power management behavior

Power policy is enforced by the app manager and can be overridden with `/config/power.json` on SD.

Default behavior:
- After 30 seconds of no touch interaction, the active app transitions to the lock screen.
- While on lock screen and not charging, Wi-Fi is disconnected to reduce idle power draw.
- After 120 seconds of no touch interaction on lock screen, the device enters deep sleep for timer wake.

`/config/power.json` fields:
```json
{
  "lockTimeoutMs": 30000,
  "deepSleepTimeoutMs": 120000,
  "allowDeepSleep": true,
  "deepSleepDurationSec": 60
}
```

Example command to write config:
```bash
cat >/media/sdcard/config/power.json <<'JSON'
{"lockTimeoutMs":45000,"deepSleepTimeoutMs":180000,"allowDeepSleep":true,"deepSleepDurationSec":90}
JSON
```

## Touch gesture support

Gesture classification now supports: tap, long-press, drag, swipe-left/right/up/down (via `TouchClassifier`, used by `BoardHAL::pollTouch`).
Two-point touch is integrated with the touch classifier and emits `PinchIn`/`PinchOut` events via `BoardHAL::setTouchSampleTwoPoint(...)` and `BoardHAL::pollTouch()`.

## Swipe navigation

Global gestures: swipe-down returns to springboard (Home), swipe-right returns to previous app (Back stack).

## Hardware buttons

- **Home button**: sends the UI to the Springboard from any app.
- **Home button while already on Springboard**: jumps to the first Springboard page; if already on page 1 it does nothing.
- **Backlight/frontlight enable**: GPIO11 (`PIN_BL_EN`) is used by `BoardHAL` lowlight/backlight control and lock-screen tap toggling.
- **PWR button**:
  - From normal operation: enters low-power lock-screen mode.
  - From lock-screen mode: returns to normal online operation and attempts Wi-Fi reconnect from saved credentials.

Springboard launcher gestures:
- Swipe left/right to move between app pages when there are more than 15 apps.
- Long-press an app tile to open app options, then tap "Move to Front" to pin it to the first slot.
- Online-required apps display an `ONLINE` badge when Wi-Fi is connected, and an `OFFLINE` unavailable badge when Wi-Fi is disconnected.
- Tapping an app in `OFFLINE` unavailable state is blocked until Wi-Fi connectivity is restored.

## Radio scan logs

Each Radio Scanner run writes a timestamped scan log under `/radio/scans/scan-<millis>.log` including signal fields and GPS coordinates when a fix is available.

## Weather cache format

`/cache/weather/current.json` expected schema:

```json
{
  "fetchedEpoch": 1700000000,
  "summary": "Clear skies"
}
```

The Weather app marks cache state as `FRESH` or `STALE` using a 30-minute freshness window.

## URL Fetcher config

Create `/config/url_fetcher.json`:

```json
{
  "url": "https://example.com/api",
  "timeoutMs": 5000
}
```

Behavior:
- Online: fetches URL and caches response under `/cache/http/url-<hash>.txt`.
- Offline: reads last cached response for that URL.

## Markdown Reader input file

Place a document at `/documents/markdown/readme.md`. The app renders a title and a bounded preview window from that file.
Reading progress is persisted to `/config/markdown_progress.json` and restored when reopening the Markdown Reader.

OpenAPI spec: `docs/openapi.yaml`

## BLE scan details

Radio Scanner BLE entries now include advertisement summary with RSSI, service UUID count, and manufacturer-data byte length.

## Lock screen fields

Lock screen now renders UTC time/date from GPS epoch when available, GPS state, battery status, and a live cache-derived map preview summary (tile path resolution + cached/uncached state).

## File Explorer ordering

File Explorer lists directory entries sorted with directories first, then alphabetical names, to improve navigation on e-paper.

## GPS best-fit

GPS Map now computes a best-fit location from recent fixes with outlier rejection and displays it alongside live fix coordinates.

## Meshtastic storage

Created paths: `/meshtastic/messages`, `/meshtastic/nodes`, and `/meshtastic/config`. Meshtastic app shows message/node file counts from SD.

## GPS track logging

`GPSService` now appends movement-filtered fixes (>=5 meters) to `/gps/tracks/current_track.csv`.

CSV fields per row:

`epoch,lat,lon,altM,speedKmph,headingDegOrMinusOne,hdop,sats`

`epoch` is derived from GPS date/time when valid (UTC Unix seconds), otherwise `0`.

`headingDegOrMinusOne` is `-1.0` when heading is unreliable (for example while nearly stationary).

Track logging is disabled by default. Enable it by setting `/config/device.json`:

```json
{
  "gpsTrackLogging": true
}
```


GPS history clear command:

```bash
curl -X POST http://<device-ip>/api/gps/tracks/clear
```

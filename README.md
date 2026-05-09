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
- Games folder with Chess, Go, Tic-Tac-Toe, and Minesweeper placeholders
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

Use PlatformIO:

```bash
pio run
pio run -t upload
pio device monitor
```

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

## Web server API

When Web Server app is running, these HTTP endpoints are available:

- `GET /api/health` → health JSON for web service.
- `GET /api/apps/order` → returns `/config/apps.json` contents if present.
- `GET /api/status` → current device snapshot (wifi/ip/gps/battery/sd/web flags).
- `GET /api/weather/cache` → returns `/cache/weather/current.json` when cached weather exists.
- `GET /api/cache/stats` → map cache hit/miss counters.
- `GET /api/radio/scans` → lists files under `/radio/scans`.
- `GET /api/meshtastic/stats` → Meshtastic message/node file counters.
- `GET /...` → static files served from `/webroot` with content-type detection.

Example command:

```bash
curl http://<device-ip>/api/health
```

## Touch gesture support

Gesture classification now supports: tap, long-press, drag, swipe-left/right/up/down (via `TouchClassifier`, used by `BoardHAL::pollTouch`).

## Swipe navigation

Global gestures: swipe-down returns to springboard (Home), swipe-right returns to previous app (Back stack).

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

OpenAPI spec: `docs/openapi.yaml`

## BLE scan details

Radio Scanner BLE entries now include advertisement summary with RSSI, service UUID count, and manufacturer-data byte length.

## Lock screen fields

Lock screen now renders UTC time/date from GPS epoch when available, GPS state, battery status, and coordinate summary.

## File Explorer ordering

File Explorer lists directory entries sorted with directories first, then alphabetical names, to improve navigation on e-paper.

## GPS best-fit

GPS Map now computes a best-fit location from recent fixes with outlier rejection and displays it alongside live fix coordinates.

## Meshtastic storage

Created paths: `/meshtastic/messages`, `/meshtastic/nodes`, and `/meshtastic/config`. Meshtastic app shows message/node file counts from SD.

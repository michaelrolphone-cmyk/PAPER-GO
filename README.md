# T5 Field OS

Firmware scaffold for the **LILYGO T5-4.7-S3 E-Paper Pro ESP32-S3 SX1262 LoRa 915 MHz TF Card RTC GPS board**.

This is a separate handheld e-paper app launcher project. It is not the garden watering valve controller and not the e-ink irrigation schedule display.

## What is included

- Springboard launcher
- Persistent status bar
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

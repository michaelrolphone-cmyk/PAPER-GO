# Implementation Notes

## Real display integration

`BoardHAL` is deliberately the only place that should know the ED047TC1 display driver. Replace the Serial draw calls with the selected display library calls.

Recommended path:

1. Start with official LILYGO `T5S3-4.7-e-paper-PRO` examples for board power/display init.
2. Confirm ED047TC1 framebuffer format.
3. Implement these functions first:
   - `clear`
   - `drawText`
   - `drawRect`
   - `fillRect`
   - `drawLine`
   - `endFrame`
4. Only after full refresh works, add partial refresh zones.

## Real touch integration

`BoardHAL::pollTouch()` should read the GT911 controller over I2C and translate raw touches into `TouchEvent` values.

## Map tile implementation

The `GpsMapApp` currently marks the rendering area and cache policy. Next implementation step is a tile service:

- Lat/lon to slippy tile z/x/y
- SD path lookup
- HTTP download if missing and Wi-Fi is connected
- Tile decode/render or preprocessed tile format

For ESP32, a custom preprocessed grayscale tile format may be more reliable than decoding PNG/JPEG tiles on-device.

## Weather implementation

Weather provider should be selected in `/config/weather.json`. Data should be cached with a timestamp and stale label.

## Meshtastic implementation

This project has a Meshtastic app boundary and SD storage. Full Meshtastic compatibility should be treated as a major port/integration task, not a small UI-only feature.

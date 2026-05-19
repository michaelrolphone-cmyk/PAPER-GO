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

- [x] Web API exposes `/api/status` for core device status snapshot.
- [x] OpenAPI specification added for `/api/health`, `/api/status`, and `/api/apps/order`.

- [x] Web API exposes cached weather payload endpoint (`/api/weather/cache`).

- [x] BLE scan output includes service UUID and manufacturer-data summary fields.

- [x] Lock screen renders large time/date and coordinate summary from available GPS epoch/fix.
- [x] Lock screen displays GPS and battery status fields.

- [x] File Explorer displays directory-first sorted listings for SD browsing.

- [x] GPS best-fit location computed from recent fixes with basic outlier rejection.

- [x] Meshtastic storage includes `/meshtastic/config` in addition to messages/nodes.
- [x] Meshtastic app surfaces SD storage counts for messages and nodes.

- [x] Web API exposes cache hit/miss counters via `/api/cache/stats`.

- [x] Web API exposes radio scan log listing endpoint (`/api/radio/scans`).

- [x] Web API exposes Meshtastic storage counters endpoint (`/api/meshtastic/stats`).

# LilyGO T5 S3 Paper Pro DGPS Rover Message Handling Specification

## 1. Purpose

This document defines how the LilyGO T5 S3 Paper Pro rover firmware shall receive, validate, interpret, apply, display, and log LoRa differential GPS correction messages transmitted by the Heltec Wireless Tracker V2 base station.

This is **not RTCM RTK**. It is a **position-domain differential GPS correction system**.

The base station computes:

```text
base_error = base_live_position - base_locked_origin
```

The rover applies:

```text
rover_corrected_position = rover_live_position - base_error
```

The goal is to reduce common GPS drift between a fixed base and nearby rover, producing a closer approximation to RTK-like local repeatability without RTCM or carrier-phase processing.

---

## 2. System Roles

### 2.1 Heltec Base Station

The Heltec base station is stationary. It shall:

1. Acquire GNSS lock.
2. Average its live GPS position while stationary.
3. Lock a base origin when the user presses the USER/BOOT button.
4. Compute live GPS error relative to that origin.
5. Filter the error vector.
6. Estimate the error vector rate.
7. Transmit binary correction packets over LoRa 915 MHz.

### 2.2 LilyGO Rover

The LilyGO rover shall:

1. Receive LoRa correction packets.
2. Validate packet structure and CRC.
3. Reject stale or low-quality corrections.
4. Read its own GNSS position.
5. Convert its own live GPS position into a local ENU frame relative to the base origin.
6. Subtract the base error correction.
7. Optionally predict correction drift using the transmitted error-rate fields.
8. Display corrected position/status on the e-paper screen.
9. Support a rover-side origin lock for corrected drift/stakeout display.
10. Log raw rover GPS, received corrections, corrected positions, and user events.

---

## 3. LoRa Radio Parameters

The LilyGO rover receiver shall use the same LoRa settings as the Heltec base transmitter.

```text
Frequency:        915.0 MHz
Bandwidth:        125.0 kHz
Spreading factor: SF7
Coding rate:      4/5
Sync word:        0x12
Preamble:         8 symbols
CRC:              enabled
Payload type:     binary fixed-size struct
```

If link margin is poor, the following may be adjusted later on both devices together:

```text
SF7 -> SF8 or SF9
125 kHz -> 250 kHz if higher throughput is needed and range allows
TX interval 1 Hz -> slower if channel occupancy becomes excessive
```

The receiver must treat all received bytes as binary. It must not assume null termination or printable ASCII.

---

## 4. Correction Packet Format

The Heltec base transmits a packed binary structure named `DgpsCorrectionPayloadV1`.

All multibyte fields are little-endian because the sender and receiver are ESP32-class devices. If this protocol is later consumed by another architecture, explicit little-endian decoding shall be used.

### 4.1 Packet Structure

```cpp
#pragma pack(push, 1)

struct DgpsCorrectionPayloadV1 {
  uint8_t  magic0;        // 'D'
  uint8_t  magic1;        // 'G'
  uint8_t  version;       // 1
  uint8_t  type;          // 1 = correction
  uint8_t  flags;
  uint8_t  reserved0;

  uint32_t seq;
  uint32_t originSeq;
  uint32_t txMillis;

  uint16_t gpsYear;
  uint8_t  gpsMonth;
  uint8_t  gpsDay;
  uint8_t  gpsHour;
  uint8_t  gpsMinute;
  uint8_t  gpsSecond;
  uint8_t  gpsCentisecond;

  int32_t  originLatE7;
  int32_t  originLonE7;
  int32_t  originAltCm;

  int32_t  rawErrECm;
  int32_t  rawErrNCm;
  int32_t  rawErrUCm;

  int32_t  filtErrECm;
  int32_t  filtErrNCm;
  int32_t  filtErrUCm;

  int16_t  rateECmPerSec;
  int16_t  rateNCmPerSec;
  int16_t  rateUCmPerSec;

  uint16_t hdopCenti;
  uint16_t fixAgeMs;

  uint8_t  satsUsed;
  uint8_t  satsView;

  uint8_t  nmeaMask;
  uint8_t  gsvTotal;
  uint8_t  gsvIndex;

  uint16_t avgCn0X10;
  uint16_t cn0Count;

  uint32_t visibleSatSig;
  uint32_t usedSatSig;

  uint16_t originSamples;
  uint16_t filterSamples;

  uint16_t payloadBytes;
  uint32_t crc32;
};

#pragma pack(pop)
```

### 4.2 Packet Size

The rover shall require the received payload length to equal:

```cpp
sizeof(DgpsCorrectionPayloadV1)
```

Packets with shorter or longer lengths shall be rejected unless a future protocol version explicitly supports variable-length messages.

---

## 5. Header Validation

The rover shall reject any packet that fails these checks:

```text
magic0 == 'D'
magic1 == 'G'
version == 1
type == 1
payloadBytes == sizeof(DgpsCorrectionPayloadV1) - sizeof(crc32)
```

The rover shall also reject packets where the LoRa-reported byte count does not equal `sizeof(DgpsCorrectionPayloadV1)`.

---

## 6. CRC Validation

The last field, `crc32`, is computed over every byte of the packet except the `crc32` field itself.

The rover shall compute:

```cpp
computed_crc = crc32Bytes(
  (const uint8_t *)&pkt,
  sizeof(DgpsCorrectionPayloadV1) - sizeof(pkt.crc32)
);
```

The packet is valid only if:

```cpp
computed_crc == pkt.crc32
```

CRC failures shall increment a receiver counter and shall not update the active correction state.

---

## 7. Flag Definitions

The `flags` byte uses the following bits:

```cpp
static const uint8_t FLAG_FIX_VALID       = 1 << 0;
static const uint8_t FLAG_ORIGIN_LOCKED   = 1 << 1;
static const uint8_t FLAG_FILTER_READY    = 1 << 2;
static const uint8_t FLAG_ORIGIN_AVERAGED = 1 << 3;
static const uint8_t FLAG_HDOP_GOOD       = 1 << 4;
static const uint8_t FLAG_UTC_VALID       = 1 << 5;
static const uint8_t FLAG_FIX_FRESH       = 1 << 6;
```

### 7.1 Required Flags

The rover shall only apply a correction if all of these are set:

```text
FLAG_FIX_VALID
FLAG_ORIGIN_LOCKED
FLAG_FILTER_READY
FLAG_HDOP_GOOD
FLAG_UTC_VALID
FLAG_FIX_FRESH
```

If `FLAG_ORIGIN_AVERAGED` is not set, the rover may still apply the correction, but the display shall mark base quality as `QUICK` instead of `AVG`.

---

## 8. Sequence Handling

### 8.1 `seq`

`seq` increments each time the base transmits a correction packet.

The rover shall track:

```text
lastSeq
receivedPacketCount
missedPacketCount
duplicatePacketCount
outOfOrderPacketCount
```

Recommended behavior:

```text
if seq == lastSeq:
    duplicate; ignore

if seq > lastSeq + 1:
    missedPacketCount += seq - lastSeq - 1
    accept if otherwise valid

if seq < lastSeq:
    out-of-order; usually ignore
```

Because `seq` is `uint32_t`, rollover should be handled using unsigned subtraction if the system is expected to run long-term.

### 8.2 `originSeq`

`originSeq` identifies the base-origin lock event.

If `originSeq` changes, the rover shall:

1. Clear the current correction filter state.
2. Clear any correction-age assumptions.
3. Mark the base origin as changed.
4. Recompute local ENU origin from the new base origin.
5. Optionally invalidate the rover-side drift lock until the user re-locks rover origin.

This prevents applying corrections from one base-origin definition to positions computed relative to another.

---

## 9. Time Handling

### 9.1 Transmitted Time Fields

The packet contains two types of time:

```text
txMillis: ESP32 millisecond counter at base transmit time
GPS UTC fields: year/month/day/hour/minute/second/centisecond from GNSS
```

`txMillis` is only meaningful for base-side uptime and sequence diagnostics. It shall not be used as absolute time on the rover because the rover and base clocks are not synchronized.

The GPS UTC fields shall be used for human-readable logs and approximate correction epoch matching.

### 9.2 Correction Age

The rover shall compute a local correction age using receive time:

```cpp
correctionAgeMs = millis() - lastCorrectionReceiveMillis;
```

Recommended correction-age policy:

```text
0-2000 ms:     apply full correction
2000-5000 ms:  apply predicted/faded correction and warn STALE
>5000 ms:      reject correction for navigation/stakeout use
```

Correction age shall be displayed on the e-paper status screen.

---

## 10. Unit Conventions

### 10.1 Coordinate Origin

```text
originLatE7 = latitude degrees * 10,000,000
originLonE7 = longitude degrees * 10,000,000
originAltCm = altitude meters * 100
```

### 10.2 Error Vector

The correction vector is local ENU error from the base locked origin to the base live GPS fix.

```text
rawErrECm   = raw east error, centimeters
rawErrNCm   = raw north error, centimeters
rawErrUCm   = raw up error, centimeters
filtErrECm  = filtered east error, centimeters
filtErrNCm  = filtered north error, centimeters
filtErrUCm  = filtered up error, centimeters
```

Sign convention:

```text
positive east  = live GPS appears east of the true/locked base origin
positive north = live GPS appears north of the true/locked base origin
positive up    = live GPS appears above the true/locked base origin
```

Rover correction:

```text
corrected = live - base_error
```

### 10.3 Error Rate

```text
rateECmPerSec = east correction drift rate, centimeters/second
rateNCmPerSec = north correction drift rate, centimeters/second
rateUCmPerSec = up correction drift rate, centimeters/second
```

These rates allow the rover to predict the correction forward slightly when LoRa receive timing is irregular.

---

## 11. Rover Coordinate Processing

### 11.1 Decode Base Origin

The rover shall decode the base origin:

```cpp
baseOriginLatDeg = pkt.originLatE7 / 10000000.0;
baseOriginLonDeg = pkt.originLonE7 / 10000000.0;
baseOriginAltM   = pkt.originAltCm / 100.0;
```

### 11.2 Convert Rover Live GPS to ENU Relative to Base Origin

Given the rover live GPS position:

```text
roverLatDeg
roverLonDeg
roverAltM
```

Compute local position relative to the base origin:

```cpp
roverLiveEastM, roverLiveNorthM = latLonDeltaMeters(
  baseOriginLatDeg,
  baseOriginLonDeg,
  roverLatDeg,
  roverLonDeg
);

roverLiveUpM = roverAltM - baseOriginAltM;
```

Convert to centimeters:

```cpp
roverLiveECm = round(roverLiveEastM * 100.0);
roverLiveNCm = round(roverLiveNorthM * 100.0);
roverLiveUCm = round(roverLiveUpM * 100.0);
```

### 11.3 Predict Correction for Age

If correction age is small, use the transmitted filtered correction directly:

```cpp
predErrE = pkt.filtErrECm;
predErrN = pkt.filtErrNCm;
predErrU = pkt.filtErrUCm;
```

If correction age is greater than zero and still within the accepted age window, predict forward:

```cpp
ageSec = correctionAgeMs / 1000.0;

predErrE = pkt.filtErrECm + pkt.rateECmPerSec * ageSec;
predErrN = pkt.filtErrNCm + pkt.rateNCmPerSec * ageSec;
predErrU = pkt.filtErrUCm + pkt.rateUCmPerSec * ageSec;
```

Recommended maximum prediction age:

```text
2 seconds for full-confidence use
5 seconds for degraded/stale display only
```

### 11.4 Apply Differential Correction

```cpp
correctedECm = roverLiveECm - predErrE;
correctedNCm = roverLiveNCm - predErrN;
correctedUCm = roverLiveUCm - predErrU;
```

### 11.5 Vertical Correction Weighting

Vertical GPS error is usually less stable than horizontal GPS error. The rover shall support a vertical correction scale factor:

```cpp
verticalCorrectionWeight = 0.50; // default
```

Apply vertical correction as:

```cpp
correctedUCm = roverLiveUCm - round(predErrU * verticalCorrectionWeight);
```

Valid configuration range:

```text
0.00 = no vertical correction
0.25 = conservative vertical correction
0.50 = default vertical correction
1.00 = full vertical correction
```

---

## 12. Converting Corrected ENU Back to Latitude/Longitude

The rover may keep corrected coordinates in ENU for drift/stakeout display. If corrected latitude/longitude is required, approximate inverse projection may be used for local distances:

```cpp
latRad = baseOriginLatDeg * PI / 180.0;

correctedLatDeg = baseOriginLatDeg +
  (correctedNorthM / EARTH_RADIUS_M) * 180.0 / PI;

correctedLonDeg = baseOriginLonDeg +
  (correctedEastM / (EARTH_RADIUS_M * cos(latRad))) * 180.0 / PI;

correctedAltM = baseOriginAltM + correctedUpM;
```

This local tangent-plane approximation is suitable for short baseline distances. For larger baselines, the rover firmware should use a more rigorous geodetic/ECEF conversion.

---

## 13. Quality Gating

The rover shall assign correction quality using both base packet quality and rover GNSS quality.

### 13.1 Base Correction Acceptance

The rover shall reject correction application if any of the following are true:

```text
CRC invalid
bad magic/version/type
required flags missing
correctionAgeMs > 5000
hdopCenti > 300
fixAgeMs > 5000
satsUsed < 5
originSamples < 5
filterSamples < 1
```

### 13.2 Preferred Base Quality

The rover shall display high base quality only when:

```text
FLAG_ORIGIN_AVERAGED set
originSamples >= 30
filterSamples >= 7
hdopCenti <= 150
satsUsed >= 8
correctionAgeMs <= 2000
```

### 13.3 Rover GNSS Quality

The rover shall only compute corrected navigation/stakeout position if its own GPS has:

```text
fresh valid location
valid UTC time
HDOP <= 3.0
satellites >= 5
fix age <= 5000 ms
```

If the rover GPS is valid but weak, the display may show corrected position as degraded.

### 13.4 Quality States

Recommended rover quality states:

```text
NO_ROVER_FIX      rover GPS invalid or stale
NO_BASE           no valid correction received
BASE_STALE        correction age too old
BASE_BAD          correction packet valid but poor quality
DGPS_DEGRADED     correction applied but quality marginal
DGPS_GOOD         correction applied normally
DGPS_BEST         averaged base, stable filter, good HDOP/sats, young correction
```

---

## 14. Satellite Signature Handling

The base packet includes:

```text
visibleSatSig
usedSatSig
satsView
satsUsed
avgCn0X10
cn0Count
```

These are not RTK observables. They are quality diagnostics.

The LilyGO rover should compute its own rough visible/used satellite signatures from NMEA GSV/GSA if possible, using the same FNV-1a hash method. Then it can estimate whether the base and rover are seeing a similar satellite environment.

Recommended behavior:

```text
If base and rover usedSatSig match exactly:
    sky match = strong

If visible signatures differ but sats/HDOP are good:
    sky match = moderate

If rover and base have very different satsUsed/satsView or poor C/N0:
    sky match = weak
```

Because NMEA satellite reporting varies by receiver, signature mismatch shall not by itself reject corrections. It shall only reduce confidence or display a warning.

---

## 15. Rover-Side Origin Lock

The LilyGO shall support a rover-side origin lock independent of the base origin.

### 15.1 Purpose

Rover origin lock is used to display corrected local drift/stakeout movement:

```text
corrected rover current position - corrected rover locked origin
```

### 15.2 Behavior

When the user presses the configured LilyGO button/touch control:

1. Verify current corrected rover position is valid.
2. Store corrected E/N/U as `roverOriginECm`, `roverOriginNCm`, `roverOriginUCm`.
3. Set `roverOriginLocked = true`.
4. Log a `ROVER_ORIGIN_LOCK` event.

Then display:

```cpp
driftECm = correctedECm - roverOriginECm;
driftNCm = correctedNCm - roverOriginNCm;
driftUCm = correctedUCm - roverOriginUCm;
```

Also compute:

```cpp
horizontalDistanceCm = sqrt(driftECm^2 + driftNCm^2);
bearingDeg = atan2(driftECm, driftNCm) * 180 / PI;
```

Bearing convention:

```text
0 degrees = north
90 degrees = east
180 degrees = south
270 degrees = west
```

---

## 16. E-Paper Display Requirements

The LilyGO e-paper display shall show two levels of status: compact field screen and diagnostic screen.

### 16.1 Field Screen

Required fields:

```text
DGPS ROVER
State: DGPS_GOOD / DGPS_DEGRADED / NO_BASE / NO_FIX
Base age: 0.8 s
LoRa RSSI: -94 dBm
LoRa SNR: 8.5 dB
Base sats/HDOP: 12 / 0.82
Rover sats/HDOP: 14 / 0.95
Corrected lat/lon or ENU
E drift: +0.12 m
N drift: -0.04 m
U drift: +0.08 m
Distance: 0.13 m
Bearing: 108.4 deg
Battery
Logging status
```

### 16.2 Diagnostic Screen

Required fields:

```text
Base ID / originSeq
Packet seq
Packets received
Packets missed
CRC errors
Duplicate packets
Correction raw E/N/U
Correction filtered E/N/U
Correction rate E/N/U
Base origin samples
Base filter samples
Base average C/N0
Satellite signature status
Rover raw position
Rover corrected position
Rover origin lock state
```

### 16.3 Refresh Policy

Because the LilyGO display is e-paper, the firmware should avoid full-screen refreshes at 1 Hz unless necessary.

Recommended strategy:

```text
1 Hz: update small numeric/status regions if partial refresh is reliable
5-10 s: full refresh
Immediate refresh: state changes, origin lock, LoRa link lost/recovered
```

If ghosting becomes unacceptable, use a periodic full black/white clear cycle.

---

## 17. Logging Requirements

The LilyGO rover shall log enough data to diagnose field performance.

### 17.1 Correction Log

File:

```text
/logs/corrections_YYYYMMDD.csv
```

Columns:

```text
rover_ms,base_seq,origin_seq,base_utc,age_ms,rssi,snr,
flags,base_hdop,base_fix_age_ms,base_sats_used,base_sats_view,
raw_e_cm,raw_n_cm,raw_u_cm,
filt_e_cm,filt_n_cm,filt_u_cm,
rate_e_cm_s,rate_n_cm_s,rate_u_cm_s,
origin_lat_e7,origin_lon_e7,origin_alt_cm,
origin_samples,filter_samples,crc_ok
```

### 17.2 Rover Position Log

File:

```text
/logs/rover_positions_YYYYMMDD.csv
```

Columns:

```text
rover_ms,rover_utc,quality_state,
raw_lat,raw_lon,raw_alt_m,
raw_hdop,raw_sats,raw_fix_age_ms,
base_origin_lat,base_origin_lon,base_origin_alt_m,
live_e_cm,live_n_cm,live_u_cm,
corr_e_cm,corr_n_cm,corr_u_cm,
corr_lat,corr_lon,corr_alt_m,
base_seq,base_age_ms,rssi,snr
```

### 17.3 Event Log

File:

```text
/logs/events_YYYYMMDD.csv
```

Columns:

```text
rover_ms,utc,event,detail,lat,lon,alt_m,corr_e_cm,corr_n_cm,corr_u_cm
```

Events:

```text
BOOT
BASE_ACQUIRED
BASE_LOST
BASE_ORIGIN_CHANGED
ROVER_ORIGIN_LOCK
ROVER_ORIGIN_CLEAR
LOG_START
LOG_STOP
GPS_FIX_GAINED
GPS_FIX_LOST
```

---

## 18. Runtime State Model

The LilyGO rover shall maintain a state object equivalent to:

```cpp
struct RoverCorrectionState {
  bool hasValidCorrection;
  bool baseOriginKnown;
  bool baseOriginChanged;

  DgpsCorrectionPayloadV1 lastPacket;

  uint32_t lastReceiveMillis;
  uint32_t lastSeq;
  uint32_t packetsReceived;
  uint32_t packetsMissed;
  uint32_t packetsDuplicate;
  uint32_t packetsOutOfOrder;
  uint32_t packetsCrcFail;

  float lastRssi;
  float lastSnr;

  double baseOriginLatDeg;
  double baseOriginLonDeg;
  double baseOriginAltM;

  int32_t predictedErrECm;
  int32_t predictedErrNCm;
  int32_t predictedErrUCm;

  int32_t roverLiveECm;
  int32_t roverLiveNCm;
  int32_t roverLiveUCm;

  int32_t roverCorrectedECm;
  int32_t roverCorrectedNCm;
  int32_t roverCorrectedUCm;

  bool roverOriginLocked;
  int32_t roverOriginECm;
  int32_t roverOriginNCm;
  int32_t roverOriginUCm;
};
```

---

## 19. Main Rover Loop

Recommended loop order:

```text
1. Feed rover GPS parser.
2. Poll LoRa receive.
3. If a packet arrives:
   a. validate length
   b. validate header
   c. validate CRC
   d. validate flags/quality
   e. update correction state
   f. log correction
4. If rover GPS has fresh fix:
   a. convert rover raw GPS to ENU relative to base origin
   b. predict base correction for correction age
   c. subtract base correction
   d. compute corrected lat/lon if needed
   e. compute rover-origin drift if locked
   f. log corrected position
5. Update display regions.
6. Poll buttons/touch controls.
7. Periodically flush logs.
```

---

## 20. Pseudocode for Packet Handling

```cpp
bool handleReceivedCorrection(uint8_t *buf, size_t len) {
  if (len != sizeof(DgpsCorrectionPayloadV1)) {
    stats.badLength++;
    return false;
  }

  DgpsCorrectionPayloadV1 pkt;
  memcpy(&pkt, buf, sizeof(pkt));

  if (pkt.magic0 != 'D' || pkt.magic1 != 'G') {
    stats.badMagic++;
    return false;
  }

  if (pkt.version != 1 || pkt.type != 1) {
    stats.badVersion++;
    return false;
  }

  if (pkt.payloadBytes != sizeof(DgpsCorrectionPayloadV1) - sizeof(pkt.crc32)) {
    stats.badPayloadSize++;
    return false;
  }

  uint32_t computed = crc32Bytes(
    (const uint8_t *)&pkt,
    sizeof(DgpsCorrectionPayloadV1) - sizeof(pkt.crc32)
  );

  if (computed != pkt.crc32) {
    stats.crcFail++;
    return false;
  }

  uint8_t required =
    FLAG_FIX_VALID |
    FLAG_ORIGIN_LOCKED |
    FLAG_FILTER_READY |
    FLAG_HDOP_GOOD |
    FLAG_UTC_VALID |
    FLAG_FIX_FRESH;

  if ((pkt.flags & required) != required) {
    stats.badFlags++;
    return false;
  }

  if (pkt.hdopCenti > 300) {
    stats.badBaseHdop++;
    return false;
  }

  if (pkt.fixAgeMs > 5000) {
    stats.badBaseAge++;
    return false;
  }

  if (pkt.satsUsed < 5) {
    stats.badBaseSats++;
    return false;
  }

  updateSequenceStats(pkt.seq);

  if (!state.baseOriginKnown || pkt.originSeq != state.lastPacket.originSeq) {
    state.baseOriginChanged = true;
    state.baseOriginKnown = true;
    state.baseOriginLatDeg = pkt.originLatE7 / 10000000.0;
    state.baseOriginLonDeg = pkt.originLonE7 / 10000000.0;
    state.baseOriginAltM = pkt.originAltCm / 100.0;

    state.roverOriginLocked = false;
  }

  state.lastPacket = pkt;
  state.lastReceiveMillis = millis();
  state.hasValidCorrection = true;

  state.lastRssi = radio.getRSSI();
  state.lastSnr = radio.getSNR();

  logCorrection(pkt);

  return true;
}
```

---

## 21. Pseudocode for Correction Application

```cpp
bool computeCorrectedRoverPosition() {
  if (!state.hasValidCorrection) return false;
  if (!state.baseOriginKnown) return false;
  if (!roverGpsUsable()) return false;

  uint32_t ageMs = millis() - state.lastReceiveMillis;

  if (ageMs > 5000) {
    qualityState = BASE_STALE;
    return false;
  }

  double liveEastM = 0.0;
  double liveNorthM = 0.0;

  latLonDeltaMeters(
    state.baseOriginLatDeg,
    state.baseOriginLonDeg,
    roverGpsLatDeg,
    roverGpsLonDeg,
    liveEastM,
    liveNorthM
  );

  int32_t liveECm = round(liveEastM * 100.0);
  int32_t liveNCm = round(liveNorthM * 100.0);
  int32_t liveUCm = round((roverGpsAltM - state.baseOriginAltM) * 100.0);

  double ageSec = ageMs / 1000.0;

  int32_t predErrECm = round(
    state.lastPacket.filtErrECm +
    state.lastPacket.rateECmPerSec * ageSec
  );

  int32_t predErrNCm = round(
    state.lastPacket.filtErrNCm +
    state.lastPacket.rateNCmPerSec * ageSec
  );

  int32_t predErrUCm = round(
    state.lastPacket.filtErrUCm +
    state.lastPacket.rateUCmPerSec * ageSec
  );

  predErrUCm = round(predErrUCm * verticalCorrectionWeight);

  state.roverLiveECm = liveECm;
  state.roverLiveNCm = liveNCm;
  state.roverLiveUCm = liveUCm;

  state.predictedErrECm = predErrECm;
  state.predictedErrNCm = predErrNCm;
  state.predictedErrUCm = predErrUCm;

  state.roverCorrectedECm = liveECm - predErrECm;
  state.roverCorrectedNCm = liveNCm - predErrNCm;
  state.roverCorrectedUCm = liveUCm - predErrUCm;

  updateQualityState();
  logRoverPosition();

  return true;
}
```

---

## 22. Configuration Defaults

Recommended LilyGO defaults:

```text
Correction max age:              5000 ms
Correction full-confidence age:  2000 ms
Base max HDOP:                   3.00
Rover max HDOP:                  3.00
Minimum base satellites:         5
Minimum rover satellites:        5
Minimum preferred origin samples: 30
Vertical correction weight:      0.50
Display partial refresh:         1 Hz
Display full refresh:            10 s
Position log interval:           1 Hz
Correction log:                  every valid received correction
```

---

## 23. Failure Behavior

### 23.1 No Base Corrections

If no valid correction packet has been received:

```text
Display: NO_BASE
Use raw GPS only for map/diagnostic display
Do not claim corrected position
Do not update rover corrected-origin drift
```

### 23.2 Base Correction Stale

If last correction age exceeds 5 seconds:

```text
Display: BASE_STALE
Stop applying correction
Keep last correction visible for diagnostics
Continue logging raw rover GPS
```

### 23.3 Rover GPS Stale

If rover GPS fix is stale:

```text
Display: NO_ROVER_FIX
Keep receiving/logging base corrections
Do not compute corrected rover position
```

### 23.4 Base Origin Changed

If `originSeq` changes:

```text
Display: BASE_ORIGIN_CHANGED
Clear rover-side origin lock
Reset local corrected drift display
Require user to re-lock rover origin if needed
```

---

## 24. Implementation Priority

### Phase 1: Message Receiver

1. Configure LoRa receive with matching settings.
2. Receive binary packet.
3. Validate length/header/CRC.
4. Print decoded fields to serial.
5. Show base correction status on display.

### Phase 2: Correction Application

1. Parse rover GPS.
2. Decode base origin.
3. Convert rover GPS to ENU relative to base origin.
4. Apply filtered correction.
5. Display raw vs corrected position.

### Phase 3: Rover Origin and Field UI

1. Add rover-origin lock.
2. Display corrected drift E/N/U.
3. Display distance and bearing.
4. Add quality state display.

### Phase 4: Logging

1. Log correction packets.
2. Log rover raw/corrected positions.
3. Log user events.
4. Add log rotation by date/session.

### Phase 5: Optimization

1. Add satellite signature comparison.
2. Tune vertical correction weight.
3. Tune correction-age prediction.
4. Tune display refresh behavior.
5. Add persistent settings.

---

## 25. Core Rule

The LilyGO rover shall never silently treat a correction as valid unless it passes:

```text
binary length check
magic/version/type check
CRC32 check
required flags check
base quality check
correction age check
rover GPS quality check
```

When correction quality is degraded, the display and logs must say so explicitly.

This prevents false precision and makes field testing defensible.


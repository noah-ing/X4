# X4 Games and Utilities

Community firmware for classic games and small utilities on the **Xteink X4**
e-paper reader. The project uses the Papyrix partition layout as a compatibility
reference and provides an application image that can be written to the `app0`
offset after the device layout has been verified.

> **Status:** hardware-specific prototype. Back up the full device before
> flashing and review `partitions.csv` against your exact hardware revision.
> Compatibility and recovery are not guaranteed.

## Included Apps

### Games
| Game | Description |
|------|-------------|
| **Chess** | Full chess implementation with AI opponent (minimax with alpha-beta pruning) |
| **Minesweeper** | Classic mine-finding puzzle with 3 difficulty levels |
| **Snake** | Classic snake game adapted for E-Ink (turn-based or auto-move) |
| **2048** | Slide tiles to combine numbers and reach 2048 |
| **Game of Life** | Conway's cellular automaton with preset patterns |

### Utilities
| App | Description |
|-----|-------------|
| **Portfolio Tracker** | Local holdings display using an unofficial Yahoo Finance quote endpoint |

## Hardware

This firmware is designed for the **Xteink X4**:
- **MCU**: ESP32-C3 (16MB Flash, ~380KB RAM)
- **Display**: 4.26" E-Ink (800×480px, GDEQ0426T82)
- **Input**: Physical buttons via ADC resistor ladder

## Safety

The supplied configuration is intended to reduce flashing risk:

- It uses the documented Papyrix partition layout.
- The built application image is intended for the `app0` offset at `0x10000`.
- NVS is outside that application partition and is not rewritten by the manual
  application-only command below.
- Two application partitions are defined for OTA-capable builds.

These properties describe the checked-in configuration; they are not a safety
certification. A wrong device revision, flash size, offset, or interrupted write
can still make the device unbootable.

### Partition Layout

```
Name      Type  SubType  Offset     Size
nvs       data  nvs      0x9000     0x5000    (20KB)  - Preserved
otadata   data  ota      0xe000     0x2000    (8KB)   - OTA metadata
app0      app   ota_0    0x10000    0x640000  (6.25MB) - Primary app
app1      app   ota_1    0x650000   0x640000  (6.25MB) - OTA backup
spiffs    data  spiffs   0xc90000   0x360000  (3.4MB) - File storage
coredump  data  coredump 0xff0000   0x10000   (64KB)  - Crash dumps
```

## Building

### Prerequisites

1. [PlatformIO](https://platformio.org/) (CLI or IDE extension)
2. USB-C cable for flashing

### Build Commands

```bash
# Clone the repository
git clone https://github.com/noah-ing/X4.git
cd X4

# Build the firmware
pio run

# Build release version (optimized)
pio run -e xteink_x4_release

# Build debug version (with serial output)
pio run -e xteink_x4_debug
```

## Flashing

### Option 1: Application-only esptool command

```bash
# Build first, then write only the application image to app0
esptool.py --chip esp32c3 --port /dev/ttyACM0 --baud 460800 \
  write_flash -z 0x10000 .pio/build/xteink_x4/firmware.bin
```

Replace `/dev/ttyACM0` with the device port. This command is deliberately
explicit: PlatformIO's standard ESP32 upload target can also write supporting
images, so it is not the documented application-only path here.

### Option 2: Papyrix Flasher

If you have [papyrix-flasher](https://github.com/bigbag/papyrix-flasher) installed:

```bash
papyrix-flasher flash .pio/build/xteink_x4/firmware.bin
```

Confirm the flasher's offsets and backup behavior for the installed version
before using it.

## ⚠️ Backup First!

Before flashing any custom firmware, **backup your factory firmware**:

```bash
# Read entire 16MB flash to file
esptool.py --chip esp32c3 --port /dev/ttyACM0 read_flash 0 0x1000000 backup.bin
```

To restore:
```bash
esptool.py --chip esp32c3 --port /dev/ttyACM0 write_flash 0 backup.bin
```

## Controls

### Menu
| Button | Action |
|--------|--------|
| UP/DOWN | Navigate |
| CONFIRM (A) | Select game |
| BACK (B) / POWER | Exit to sleep |

### In-Game (varies by game)
| Button | Common Action |
|--------|---------------|
| D-PAD | Move cursor/direction |
| CONFIRM (A) | Select/Action |
| BACK (B) | Cancel/Flag/Pause |
| POWER | Special (varies) |

### Game-Specific Controls

#### Chess
- D-PAD: Move cursor
- A: Select piece / Move
- B: Deselect / Pause

#### Minesweeper
- D-PAD: Move cursor
- A: Reveal cell / Chord reveal
- B: Toggle flag

#### Snake
- D-PAD: Change direction (also moves)
- A: Move forward
- B: Toggle auto-move mode

#### 2048
- D-PAD: Slide tiles
- B: New game

#### Game of Life
- D-PAD: Move cursor (edit mode) / Speed (run mode)
- A: Toggle cell (edit) / Step (run)
- B: Pattern menu (edit) / Edit mode (run)
- POWER: Toggle run/pause

#### Portfolio Tracker
- UP/DOWN: Select stock
- A: Refresh quotes
- B: Exit

## Configuring the Portfolio Tracker

Create the ignored local configuration file, then add only the values you want
compiled into your device firmware:

```bash
cp src/config_local.example.h src/config_local.h
```

```cpp
inline void configureLocalPortfolio(StockTracker& tracker) {
    tracker.setWiFi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
    tracker.addHolding("VTI", 1.0F, 100.0F);  // symbol, shares, cost basis
}
```

The tracker uses an unofficial Yahoo Finance endpoint for delayed or near-real-time
quotes. That endpoint may change or become unavailable without notice. Wi-Fi is
required; no API key is needed. Holdings and cost basis are compiled into the
firmware. `src/config_local.h` is ignored by Git; keep credentials and personal
portfolio values there and never force-add it to a commit.

The tracker is an informational display, not investment advice, a broker, or a
source of guaranteed market data.

HTTPS requests validate the endpoint against the embedded DigiCert Global Root
G2 certificate after the device clock is synchronized over NTP. If Yahoo changes
its certificate chain, update the trusted root deliberately; do not replace this
with an insecure TLS mode. The quote endpoint remains unofficial and should not
be treated as an availability or data-quality guarantee.

## Development

### Project Structure

```
X4/
├── platformio.ini      # Build configuration
├── partitions.csv      # Flash partition table
├── include/
│   ├── x4_hardware.h   # Pin definitions
│   ├── display.h       # Display wrapper
│   ├── input.h         # Button handling
│   ├── yahoo_root_ca.h # Trusted root for the quote client
│   ├── game.h          # Game base class
│   ├── menu.h          # Game launcher
│   ├── games/
│   │   ├── chess.h
│   │   ├── minesweeper.h
│   │   ├── snake.h
│   │   ├── game2048.h
│   │   └── gameoflife.h
│   └── apps/
│       └── stocktracker.h
└── src/
    ├── main.cpp        # Entry point
    ├── config_local.example.h # Ignored local-config template
    ├── display.cpp
    ├── input.cpp
    ├── game.cpp
    ├── menu.cpp
    ├── games/
    │   ├── chess.cpp
    │   ├── minesweeper.cpp
    │   ├── snake.cpp
    │   ├── game2048.cpp
    │   └── gameoflife.cpp
    └── apps/
        └── stocktracker.cpp
```

### Adding a New Game

1. Create header in `include/games/yourgame.h`
2. Inherit from `Game` class
3. Implement required methods: `name()`, `description()`, `init()`, `update()`, `draw()`, `handleInput()`
4. Create implementation in `src/games/yourgame.cpp`
5. Add instance and register in `src/main.cpp`

### Memory Considerations

The ESP32-C3 has limited RAM (~380KB usable). Keep in mind:
- Avoid large buffers
- Use stack allocation carefully
- Consider using PROGMEM for constant data
- E-Ink refresh is slow - minimize full refreshes

## License

MIT License - See LICENSE file for details.

## Credits

- Hardware: [Xteink X4](https://www.xteink.com/products/xteink-x4)
- Display Library: [GxEPD2](https://github.com/ZinggJM/GxEPD2)
- Inspired by: [Papyrix Reader](https://github.com/bigbag/papyrix-reader)

## Disclaimer

This is a community project and is **not affiliated with Xteink, Papyrix, or
Yahoo**. Flash custom firmware at your own risk. Always back up your device
before flashing.

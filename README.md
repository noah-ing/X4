# X4 Games

A collection of classic games for the **Xteink X4** e-paper reader, designed to work safely alongside Papyrix firmware.

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
| **Portfolio Tracker** | Stock/IRA portfolio monitor with real-time quotes via Yahoo Finance API |

## Hardware

This firmware is designed for the **Xteink X4**:
- **MCU**: ESP32-C3 (16MB Flash, ~380KB RAM)
- **Display**: 4.26" E-Ink (800×480px, GDEQ0426T82)
- **Input**: Physical buttons via ADC resistor ladder

## Safety

⚠️ **This firmware is designed to be SAFE:**

- ✅ Uses the **exact same partition layout** as Papyrix
- ✅ Does **NOT** touch the bootloader (0x0-0x8FFF)
- ✅ Preserves NVS data (WiFi credentials, calibration)
- ✅ Supports OTA with dual app partitions (failsafe)
- ✅ Can coexist with Papyrix (flash to app0 partition)

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
git clone https://github.com/your-repo/x4-games.git
cd x4-games

# Build the firmware
pio run

# Build release version (optimized)
pio run -e xteink_x4_release

# Build debug version (with serial output)
pio run -e xteink_x4_debug
```

## Flashing

### Option 1: PlatformIO (Recommended)

```bash
# Connect X4 via USB-C and flash
pio run -t upload

# Monitor serial output (optional, for debugging)
pio device monitor
```

### Option 2: esptool (Manual)

```bash
# Flash only the app partition (safe, preserves bootloader/NVS)
esptool.py --chip esp32c3 --port /dev/ttyACM0 --baud 460800 \
  write_flash -z 0x10000 .pio/build/xteink_x4/firmware.bin
```

### Option 3: Papyrix Flasher

If you have [papyrix-flasher](https://github.com/bigbag/papyrix-flasher) installed:

```bash
papyrix-flasher flash .pio/build/xteink_x4/firmware.bin
```

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

To track your IRA or stock portfolio, edit `src/main.cpp` and uncomment/add your holdings:

```cpp
// In setup(), before gameMenu.addGame(&stockTracker):
stockTracker.addHolding("VTI", 50.0, 10000.00);   // Symbol, shares, cost basis
stockTracker.addHolding("VXUS", 30.0, 5000.00);
```

The tracker uses Yahoo Finance's unofficial API for real-time quotes. WiFi connection is required.

## Development

### Project Structure

```
x4-games/
├── platformio.ini      # Build configuration
├── partitions.csv      # Flash partition table
├── include/
│   ├── x4_hardware.h   # Pin definitions
│   ├── display.h       # Display wrapper
│   ├── input.h         # Button handling
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

This is a community project and is **not affiliated with Xteink**. Flash custom firmware at your own risk. Always backup your device before flashing.

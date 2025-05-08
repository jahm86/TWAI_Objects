# TWAI_Objects - CAN controller for ESP32

[![PlatformIO CI](https://github.com/jahm86/TWAI_Objects/actions/workflows/platformio.yml/badge.svg)](https://github.com/jahm86/TWAI_Objects/actions)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Advanced library for controlling the TWAI peripheral on ESP32 microcontrollers, with support for multiple transceivers and hardware filters.

## Key Features

- ✅ Support for TJA1050, MCP2551 and custom transceivers
- 🚀 Hardware filter management (up to 32 logical filters)
- 🔄 FreeRTOS support (safe queues in ISR)
- 📡 ESP32 support. ESP32-C3, ESP32-C6 (multi-CAN) coming soon

## installation

### PlatformIO
```ini
lib_deps = 
    ...
    https://github.com/jahm86/TWAI_Objects.git
    ...
```

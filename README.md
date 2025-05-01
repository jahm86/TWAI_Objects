# TWAI_Object - Controlador CAN para ESP32

[![PlatformIO CI](https://github.com/tu-usuario/TWAI_Object/actions/workflows/platformio.yml/badge.svg)](https://github.com/tu-usuario/TWAI_Object/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Biblioteca avanzada para el control del periférico TWAI en microcontroladores ESP32, con soporte para múltiples transceivers y filtros hardware.

## Características clave

- ✅ Soporte para TJA1050, MCP2551 y transceivers personalizados
- 🚀 Gestión de filtros hardware (hasta 32 filtros lógicos)
- 🔄 Compatibilidad con FreeRTOS (colas seguras en ISR)
- 📡 Soporte para ESP32. ESP32-C3, ESP32-C6 (multi-CAN), proximamente soportados

## Instalación

### PlatformIO
```ini
lib_deps = 
    https://github.com/tu-usuario/TWAI_Object.git
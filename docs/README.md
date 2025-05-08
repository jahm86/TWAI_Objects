# TWAI_Objects Documentation

[![Documentation Status](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://jahm86.github.io/TWAI_Objects/)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

<img src="https://raw.githubusercontent.com/jahm86/TWAI_Objects/main/assets/canbus-icon.svg" width="120" align="right">

Complete documentation for the TWAI_Objects library, an advanced CAN bus controller implementation for ESP32 family microcontrollers.

## 📚 Browse Documentation

- **[Online HTML Version](https://jahm86.github.io/TWAI_Objects/)**

## 🏗️ Project Structure

```
docs/
├── html/ # Generated HTML docs
├── resources/ # Templates and assets
│ ├── custom.css # Stylesheet
│ └── header.html # Navigation header
└── Doxyfile # Configuration file
```

## 🛠️ Build Locally

### Requirements
- Doxygen 1.9+
- Graphviz (for diagrams)
- Python 3.x (for preview)

### Generate Documentation

```bash
# Install dependencies
sudo apt install doxygen graphviz

# Generate docs
doxygen docs/Doxyfile

# Preview (localhost:8000)
python3 -m http.server --directory docs/html/
```

## 🔍 Key Features

### CAN Controller Features
- Hardware filter management (up to 32 filters)
- Interrupt-driven operation
- Multi-transceiver support
- Error handling and recovery
- FreeRTOS integration

### Transceiver Support

#### ✅ TJA1050
#### ✅ MCP2551
#### ✅ SN65HVD23x
#### ✅ Custom transceivers

## 📝 Code Example

```cpp
#include <TWAI_Object.h>

void setup() {
  // Initialize CAN at 500kbps
  TWAI_Object::twai.begin(GPIO_NUM_5, GPIO_NUM_4, 500000);
  
  // Send test message
  twai_message_t msg = {
    .identifier = 0x123,
    .data_length_code = 1,
    .data = {0xAA}
  };
  TWAI_Object::twai.send(msg);
}
```

## 🤝 Contributing

To contribute to the documentation:

1. Edit source comments in .h/.cpp files

2. Regenerate docs:

 ```bash
 doxygen docs/Doxyfile
 ```

3. Verify changes locally

4. Submit a Pull Request

## 📌 Version History

|Version|Changes|
|-|-|
|v0.5|Multi-transceiver docs|
|v0.4|FreeRTOS integration|
|v0.1|Initial release|

## 📜 License

MIT & LGPL-3 © 2025 JAHM86 & Deepseek. See LICENSE for details.
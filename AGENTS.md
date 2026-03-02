# AGENTS.md for Waveshare LCD Display Node.js Addon

## Project Overview

This is a Node.js native addon project that provides bindings to control a Waveshare 1.44" LCD display hat for Raspberry Pi. The project combines C++ for native hardware control with JavaScript for easy integration into Node.js applications.

## Build/Lint/Test Commands

### Installation and Build

```bash
# Install dependencies and build automatically
npm install

# Build the native addon
npm run build

# Clean and rebuild
npm run rebuild

# Clean build artifacts
npm run clean

# Clean all builds including libgpio
npm run clean:all
```

### Cross-Compilation

```bash
# Build for x64 architecture
npm run build:x64

# Build for ARM 32-bit architecture
npm run build:arm

# Build for ARM 64-bit architecture
npm run build:arm64

# Build for all supported architectures
npm run build:all
```

### Testing

```bash
# Run the complete test suite
npm test

# Run a single test (directly execute the test file)
node test/test_binding.js

# Run test with verbose output (if supported)
DEBUG=1 node test/test_binding.js
```

### Library Preparation

```bash
# Build the libgpio library (required for GPIO operations)
npm run prepare-libs
```

## Code Style Guidelines

### C++ Code (src/ directory)

#### Naming Conventions

- **Functions**: PascalCase (e.g., `DEV_ModuleInit()`, `LCD_1IN44_Init()`)
- **Classes/Structs**: PascalCase (e.g., `Paint`, `POINT`)
- **Variables**: camelCase (e.g., `imageBuffer`, `displayWidth`)
- **Constants**: UPPERCASE with underscores (e.g., `LCD_WIDTH`, `LCD_HEIGHT`)
- **Macros**: UPPERCASE with underscores (e.g., `DEBUG`, `DEV_ERROR`)

#### File Structure

```cpp
// Header files (*.h)
#ifndef FILENAME_H
#define FILENAME_H

// Include guards
#include <iostream>

// Function declarations
void FunctionName();

#endif

// Source files (*.cc)
#include "filename.h"

void FunctionName() {
    // Implementation
}
```

#### Code Style

- Use 4 spaces for indentation
- Opening braces on the same line as control statements
- Closing braces on new lines
- Add comments explaining complex logic
- Handle errors with appropriate error codes and messages
- Use `DEBUG` macro for debugging output

#### Example from src/device.cc

```cpp
int DEV_ModuleInit(void)
{
    if (DEV_ModuleInitFlag) {
        return 0;
    }

    DEBUG("DEV_ModuleInit");

    // Initialize GPIO
    int ret = lgpioInit();
    if (ret < 0) {
        DEBUG("lgpioInit failed: %d", ret);
        return -1;
    }

    DEV_ModuleInitFlag = 1;
    return 0;
}
```

### JavaScript Code

#### Naming Conventions

- **Functions**: camelCase (e.g., `getDeviceInfo()`, `clearDisplay()`)
- **Variables**: camelCase (e.g., `device`, `displayBuffer`)
- **Constants**: UPPERCASE with underscores (e.g., `KEY_UP`, `KEY_DOWN`)

#### File Structure

```javascript
// binding.js - Main entry point
const binding = require('node-gyp-build')(__dirname)

module.exports = {
  // Export functions from native addon
  init: binding.DEV_ModuleInit,
  release: binding.DEV_ModuleRelease,
  // ... other methods
}

// test_binding.js - Test file
const waveshare = require('../binding')

// Test functions
function testDeviceInitialization() {
  const result = waveshare.init()
  console.log('Initialization:', result)
  return result
}
```

#### Code Style

- Use 2 spaces for indentation
- Use semicolons at the end of statements
- Prefer const over let for immutable variables
- Add comments explaining test scenarios and expected behavior
- Handle errors with try-catch blocks

#### Example from test/test_binding.js

```javascript
function test() {
  const waveshare = require('../binding');

  console.log("Testing Waveshare LCD Display...");
  
  // Initialize device
  const initResult = waveshare.init();
  console.log("Initialization:", initResult);
  
  if (initResult === 0) {
    // Run tests
    console.log("Device initialized successfully");
    
    try {
      // Test display functions
      waveshare.clearDisplay();
      waveshare.drawBitmap('demo.bmp');
      waveshare.writeText('Hello, World!', 0, 0);
      
      console.log("Tests completed successfully");
    } catch (error) {
      console.error("Error during tests:", error);
    } finally {
      // Release resources
      waveshare.release();
    }
  } else {
    console.error("Failed to initialize device");
  }
}

test();
```

## Import Guidelines

### C++ Imports

```cpp
// Standard library
#include <iostream>
#include <cstdio>
#include <cstring>

// Node.js addon API
#include <napi.h>

// Project headers
#include "device.h"
#include "display.h"
#include "keys.h"
#include "Config/DEV_Config.h"
#include "GUI/GUI_Paint.h"
```

### JavaScript Imports

```javascript
// Native binding
const binding = require('node-gyp-build')(__dirname)

// Core modules
const fs = require('fs')
const path = require('path')

// External modules
// (none in this project)
```

## Error Handling

### C++ Error Handling

```cpp
int DEV_ModuleInit(void)
{
    int ret = lgpioInit();
    if (ret < 0) {
        DEBUG("lgpioInit failed: %d", ret);
        return -1;  // Return error code
    }
    return 0;  // Success
}
```

### JavaScript Error Handling

```javascript
try {
  const result = waveshare.someFunction();
  if (result < 0) {
    throw new Error(`Function failed with error code: ${result}`);
  }
} catch (error) {
  console.error('Error:', error.message);
  // Cleanup operations
  waveshare.release();
  process.exit(1);
}
```

## Project Structure

```
/home/joe/workspace/web/ws144/
├── src/                           # C++ source code
│   ├── addon.cc/h                 # Node.js addon entry point
│   ├── device.cc/h                # Device control functions
│   ├── display.cc/h               # Display control functions
│   ├── keys.cc/h                  # Key/button input functions
│   ├── Config/                    # Configuration
│   ├── GUI/                       # Graphics library
│   ├── LCD/                       # LCD driver
│   ├── Fonts/                     # Font files
│   └── demo.bmp                   # Sample bitmap
├── test/
│   └── test_binding.js            # Main test script
├── lgpio/                         # GPIO library source
├── prebuilds/                     # Prebuilt binaries
├── build/                         # Build output
├── binding.gyp                    # Node-gyp configuration
├── binding.js                     # JavaScript binding
├── package.json                   # Project metadata
└── README.md                      # Project description
```

## Key Files and Their Purposes

- `src/addon.cc`: Main Node.js addon entry point, exposes C++ functions to JavaScript
- `src/device.cc/h`: Device initialization and resource management
- `src/display.cc/h`: Display control functions (draw bitmap, write text, etc.)
- `src/keys.cc/h`: Key/button input handling
- `src/Config/DEV_Config.h/cc`: Hardware configuration and GPIO setup
- `src/GUI/GUI_Paint.h/cc`: Graphics drawing primitives
- `src/LCD/LCD_1in44.h/cc`: LCD driver implementation
- `binding.js`: JavaScript interface to the native addon
- `test/test_binding.js`: Comprehensive test suite

## Development Workflow

1. Make changes to C++ code in src/ directory
2. Build the native addon: `npm run build`
3. Test changes: `npm test`
4. If cross-compiling, use: `npm run build:all`
5. Commit changes to git

## Debugging Tips

1. Enable debug output by uncommenting `#define DEBUG` in src/Config/Debug.h
2. Check test_binding.js for detailed test output
3. Use Node.js debugging tools for JavaScript code
4. For C++ debugging, use gdb or other native debugging tools

## Hardware Requirements

- Raspberry Pi (any model: Zero, 1, 2, 3, 4, 5)
- Waveshare 1.44" LCD Display Hat
- Proper GPIO connection between Raspberry Pi and display

## Supported Architectures

- x86_64 (Intel/AMD)
- ARM 32-bit (Raspberry Pi 1, 2, Zero)
- ARM 64-bit (Raspberry Pi 3, 4, 5)
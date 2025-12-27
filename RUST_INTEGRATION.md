# Rust Integration Guide

This document describes how to integrate the Rust core library into the QuickerSFV build system.

## Overview

The Rust core library (`rust_core`) provides high-performance CRC32 computation that can be integrated into the existing C++ codebase via two approaches:

### Approach 1: Minimal C++ Shim (Recommended)

Create a small C++ COM DLL (~200-400 lines) that:
1. Implements `IExplorerCommand` for Windows Explorer integration
2. Calls into the Rust library via the C ABI
3. Isolates all COM/Explorer complexity in the shim

**Advantages:**
- Clean separation of concerns
- Minimal changes to existing codebase
- Easy to maintain and debug
- Standard Windows COM implementation in C++

**File Structure:**
```
rust_core/
  ├── quicksfv_core.dll   (Rust library)
  └── quicksfv_core.h     (C header)

com_shim/
  └── quicksfv_explorer.dll  (C++ COM shim)
```

### Approach 2: Direct COM in Rust (Future Enhancement)

Implement `IExplorerCommand` directly in Rust using the `windows` crate.

**Advantages:**
- Single DLL deployment
- No C++ shim needed
- Direct Windows API access

**Disadvantages:**
- More complex Rust code
- Requires learning Windows COM in Rust
- Less standard approach

## CMake Integration

To integrate the Rust library into the CMake build:

### Option A: Cargo as External Project

Add to `CMakeLists.txt`:

```cmake
find_program(CARGO_EXECUTABLE cargo REQUIRED)

add_custom_target(rust_core_build ALL
    COMMAND ${CARGO_EXECUTABLE} build --release --manifest-path ${CMAKE_SOURCE_DIR}/rust_core/Cargo.toml
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/rust_core
    COMMENT "Building Rust core library"
)

# On Windows
if(WIN32)
    set(RUST_LIB_NAME "quicksfv_core.dll")
    set(RUST_LIB_PATH "${CMAKE_SOURCE_DIR}/rust_core/target/release/${RUST_LIB_NAME}")
else()
    set(RUST_LIB_NAME "libquicksfv_core.so")
    set(RUST_LIB_PATH "${CMAKE_SOURCE_DIR}/rust_core/target/release/${RUST_LIB_NAME}")
endif()

# Copy DLL to output directory
add_custom_command(TARGET rust_core_build POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${RUST_LIB_PATH}
        $<TARGET_FILE_DIR:QuickerSFV>
)
```

### Option B: corrosion (Recommended for Complex Builds)

Use the [corrosion](https://github.com/corrosion-rs/corrosion) CMake package:

```cmake
include(FetchContent)

FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.5
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH rust_core/Cargo.toml)

target_link_libraries(YourTarget PRIVATE quicksfv_core)
```

## C++ Shim Implementation

Here's a minimal COM shim example (expand to full implementation):

```cpp
// quicksfv_explorer_command.cpp
#include <windows.h>
#include <shobjidl.h>
#include "quicksfv_core.h"

class QuickerSFVExplorerCommand : public IExplorerCommand {
private:
    ULONG m_refCount;

public:
    QuickerSFVExplorerCommand() : m_refCount(1) {}

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        // Standard COM implementation
        // ...
    }
    
    STDMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&m_refCount);
    }
    
    STDMETHODIMP_(ULONG) Release() {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }

    // IExplorerCommand methods
    STDMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) {
        // Get file path from psiItemArray
        // Call quicksfv_crc32_file()
        // Display results
        return S_OK;
    }
    
    // Other IExplorerCommand methods...
};

// DLL exports for COM registration
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    // Standard COM class factory
    // ...
}

STDAPI DllRegisterServer() {
    // Register COM object and shell extension
    // ...
}
```

## Build Instructions

### Prerequisites

- Rust toolchain (1.70+)
- CMake (3.30+)
- Visual Studio 2022 (on Windows)

### Building

```bash
# Build Rust library
cd rust_core
cargo build --release

# Build C++ project with CMake
cd ..
cmake -S . -B build -G"Visual Studio 17 2022"
cmake --build build --config Release
```

### Testing

```bash
# Test Rust library
cd rust_core
cargo test

# Test C++ integration
cd ..
cmake --build build --config Release --target quicker_sfv_test
ctest --test-dir build -C Release
```

## Deployment

### Development
- Keep Rust DLL in `target/release/`
- Copy to build output directory post-build

### Release
Package both:
- `quicksfv_core.dll` (Rust library)
- `quicksfv_explorer.dll` (C++ COM shim)
- `QuickerSFV.exe` (main application)

## Performance Considerations

The `crc32fast` crate provides:
- **SSE4.2 acceleration** - 3-4x faster than scalar
- **AVX512 acceleration** - 6-8x faster than scalar
- **PCLMUL acceleration** - 8-10x faster than scalar
- Runtime CPU feature detection

No manual optimization needed - the crate automatically selects the best implementation.

## Troubleshooting

### Rust not found
Install Rust from https://rustup.rs/

### Cargo build fails
Check network connection (crates.io access needed)

### DLL not found at runtime
Ensure DLL is in same directory as EXE or in PATH

### COM registration fails
Run `regsvr32 quicksfv_explorer.dll` as administrator

## Future Enhancements

1. **Direct Rust COM implementation** using `windows` crate
2. **Additional checksums** (SHA-256, BLAKE3) in Rust
3. **Parallel processing** for multiple files
4. **Async I/O** for better performance
5. **Cross-platform CLI** tool

## References

- [Rust Windows Programming](https://microsoft.github.io/windows-docs-rs/)
- [windows crate documentation](https://docs.rs/windows/)
- [crc32fast documentation](https://docs.rs/crc32fast/)
- [corrosion CMake integration](https://github.com/corrosion-rs/corrosion)

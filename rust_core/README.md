# QuickSFV Core - Rust Implementation

This directory contains the Rust implementation of the core CRC32 checksum functionality for QuickerSFV.

## Overview

The Rust core library provides high-performance CRC32 computation using the `crc32fast` crate, which automatically selects optimized implementations (SSE/PCLMUL) at runtime when available.

## Building

```bash
# Build the library
cargo build --release

# Run tests
cargo test

# Build and run the CLI tool
cargo build --bin quicksfv_cli --release
```

## Testing the CLI

The CLI tool can be used to verify CRC32 checksums:

```bash
# Test with the canonical test vector "123456789" (should produce CBF43926)
echo -n '123456789' | cargo run --bin quicksfv_cli -- --stdin

# Compute CRC32 for files
cargo run --bin quicksfv_cli -- file1.txt file2.txt
```

## C ABI Interface

The library exports a C ABI that can be called from C++ code:

- `quicksfv_crc32(ptr, len)` - Compute CRC32 for a byte buffer
- `quicksfv_crc32_file(path_ptr)` - Compute CRC32 for a file (UTF-16 path on Windows)
- `quicksfv_version()` - Get library version string

See `quicksfv_core.h` for the full C header.

## Integration Options

### Option 1: Direct COM in Rust (Future)
Implement `IExplorerCommand` directly in Rust using the `windows` crate.

### Option 2: C++ COM Shim (Current Approach)
A minimal C++ COM shim that:
- Implements `IExplorerCommand` interface
- Calls into the Rust library via C ABI
- Isolates COM/Explorer complexity (200-400 lines)

## Performance

The `crc32fast` crate provides:
- SIMD-accelerated implementations (SSE4.2, AVX, PCLMUL)
- Automatic CPU feature detection at runtime
- Fallback to portable implementation when SIMD unavailable
- Excellent performance on modern CPUs

## Test Vectors

The implementation is validated against standard CRC32 test vectors:

- Empty string: `00000000`
- "123456789": `CBF43926`
- "Hello, World!": `EC4AC3D0`

## Cross-Platform Support

The library is designed to work on:
- Windows (primary target)
- Linux (for development and testing)
- Other Unix-like systems

UTF-16 path handling is platform-specific and uses native Windows APIs when available.

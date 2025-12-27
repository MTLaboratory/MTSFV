# MTSFV Rust Core

This directory contains the Rust-only implementation of the checksum functionality for MTSFV. The code builds to a single executable (`mtsfv.exe`) that can be used to compute CRC32 checksums.

## Building

```bash
cd rust_core
cargo build --release --bin mtsfv
```

## Testing

```bash
cd rust_core
cargo test
```

## Using the CLI

```bash
# From source
cargo run --bin mtsfv -- --stdin
cargo run --bin mtsfv -- path/to/file1 path/to/file2

# Using a built binary
./target/release/mtsfv --stdin
./target/release/mtsfv file.txt
```

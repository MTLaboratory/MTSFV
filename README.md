# MTSFV

MTSFV is a lightweight, Rust-only checksum verifier that builds into a single executable (`mtsfv.exe` on Windows). The project has been simplified to remove all non-Rust components.

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

## Usage

```bash
# Read data from stdin
echo -n '123456789' | cargo run --bin mtsfv -- --stdin

# Compute CRC32 for files
cargo run --bin mtsfv -- file1.txt file2.txt
```

The release binary will be available at `rust_core/target/release/mtsfv.exe` on Windows (or `mtsfv` on Unix-like systems). Historical documentation files remain in the repository for reference, but only the Rust CLI is supported going forward.

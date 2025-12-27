Build Instructions
==================

The project has been simplified to a Rust-only CLI that emits a single executable (`mtsfv.exe` on Windows).

Prerequisites
-------------

- [Rust toolchain](https://rustup.rs/) (stable)

Build
-----

```bash
cd rust_core
cargo build --release --bin mtsfv
```

The optimized binary will be available at `rust_core/target/release/mtsfv.exe` on Windows (or `mtsfv` on Unix).

Testing
-------

```bash
cd rust_core
cargo test
```

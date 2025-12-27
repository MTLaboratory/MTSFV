# COM Shim for Windows Explorer Integration

This directory contains a minimal C++ COM shim that integrates MTSFV with Windows Explorer.

## Overview

The COM shim implements the `IExplorerCommand` interface to provide a context menu item in Windows Explorer. When the user right-clicks on files and selects "Compute Hash (MTSFV)", the shim:

1. Receives the list of selected files from Windows Explorer
2. Calls the Rust core library to compute CRC32 for each file
3. Displays the results in a message box

## Architecture

```
┌─────────────────────┐
│  Windows Explorer   │
└──────────┬──────────┘
           │
           │ IExplorerCommand
           ▼
┌─────────────────────┐
│  COM Shim (C++)     │  ~330 lines
│  - IExplorerCommand │
│  - IObjectWithSite  │
│  - Class Factory    │
└──────────┬──────────┘
           │
           │ C ABI
           ▼
┌─────────────────────┐
│  Rust Core Library  │
│  quicksfv_core.dll  │
└─────────────────────┘
```

## Files

- `quicksfv_explorer_command.cpp` - Main COM shim implementation
- `quicksfv_explorer.def` - Module definition file for DLL exports
- `README.md` - This file

## Building (Windows only)

### Prerequisites

- Visual Studio 2022 (or MSVC compiler)
- Windows SDK
- Rust core library (`quicksfv_core.dll`)

### Using CMake (Recommended)

The COM shim will be built automatically when building the full QuickerSFV project with CMake if `BUILD_RUST_CORE` is enabled.

### Manual Build

```cmd
REM Build Rust library first
cd ..\rust_core
cargo build --release --target x86_64-pc-windows-msvc

REM Build COM shim
cd ..\com_shim
cl /LD /EHsc /DUNICODE /D_UNICODE ^
   quicksfv_explorer_command.cpp ^
   /link /DEF:quicksfv_explorer.def ^
   /LIBPATH:..\rust_core\target\x86_64-pc-windows-msvc\release ^
   quicksfv_core.lib ^
   shlwapi.lib shell32.lib ole32.lib
```

## Registration

The COM shim must be registered with Windows to appear in the Explorer context menu.

### Automatic Registration (Installer)

In production, the installer should register the COM object and shell extension.

### Manual Registration

```cmd
REM Register the COM DLL
regsvr32 quicksfv_explorer.dll

REM Unregister
regsvr32 /u quicksfv_explorer.dll
```

### Registry Entries

The following registry entries are required (example):

```reg
Windows Registry Editor Version 5.00

[HKEY_CLASSES_ROOT\CLSID\{A5B8C3D2-1F2E-4D5A-9B3C-7E8F9A1B2C3D}]
@="MTSFV Hash Command"

[HKEY_CLASSES_ROOT\CLSID\{A5B8C3D2-1F2E-4D5A-9B3C-7E8F9A1B2C3D}\InprocServer32]
@="C:\\Path\\To\\quicksfv_explorer.dll"
"ThreadingModel"="Apartment"

[HKEY_CLASSES_ROOT\*\shell\MTSFV.Hash]
@="Compute Hash (MTSFV)"
"SubCommands"=""

[HKEY_CLASSES_ROOT\*\shell\MTSFV.Hash\shell\command]
"DelegateExecute"="{A5B8C3D2-1F2E-4D5A-9B3C-7E8F9A1B2C3D}"
```

## Testing

1. Build both the Rust library and COM shim
2. Register the COM DLL: `regsvr32 quicksfv_explorer.dll`
3. Right-click on any file in Windows Explorer
4. Look for "Compute Hash (MTSFV)" in the context menu
5. Select one or more files and click the menu item
6. A message box should appear showing the CRC32 checksums

## Implementation Details

### COM Interfaces

- **IExplorerCommand**: Provides the context menu item and handles the command
- **IObjectWithSite**: Allows Explorer to set the site object (required for shell extensions)
- **IClassFactory**: Factory for creating command instances
- **IUnknown**: Base interface for reference counting

### Key Functions

- `GetTitle()`: Returns "Compute Hash (MTSFV)"
- `GetToolTip()`: Returns descriptive tooltip
- `GetState()`: Returns ECS_ENABLED for file selections
- `Invoke()`: Called when user clicks the menu item
  - Gets selected files from IShellItemArray
  - Calls `quicksfv_crc32_file()` for each file
  - Displays results in message box

### Error Handling

- Returns 0 if file cannot be read or CRC computation fails
- Displays "ERROR" in results for failed files
- Gracefully handles missing files or permissions issues

## Limitations

- Currently only computes CRC32 (no MD5, SHA1, etc.)
- Results displayed in message box (no file output)
- Windows-only (requires Windows COM APIs)
- Requires both `quicksfv_core.dll` and `quicksfv_explorer.dll`

## Future Enhancements

1. **File Output**: Save results to .sfv file
2. **Additional Checksums**: Support MD5, SHA-256, etc.
3. **Progress Dialog**: Show progress for multiple/large files
4. **Settings**: Allow customization via registry
5. **Verify Mode**: Verify existing .sfv files
6. **Direct Rust Implementation**: Implement IExplorerCommand entirely in Rust

## Troubleshooting

### Context menu item doesn't appear

1. Verify DLL is registered: Check registry entries
2. Restart Explorer: `taskkill /f /im explorer.exe && start explorer`
3. Check event log for errors
4. Verify both DLLs are in the same directory

### "Entry point not found" error

1. Ensure `quicksfv_core.dll` is in the same directory
2. Verify Rust library was built for correct architecture (x64 vs x86)
3. Check DLL dependencies with Dependency Walker

### Access denied errors

1. Run registration as administrator: `regsvr32` requires admin rights
2. Verify file permissions on DLLs
3. Check antivirus is not blocking DLL loading

## License

This file is part of QuickerSFV and is licensed under GPL v3.
See LICENSE file for details.

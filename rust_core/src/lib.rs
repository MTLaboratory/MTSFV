use crc32fast::Hasher;
use std::fs::File;
use std::io::{BufReader, Read};
use std::os::raw::c_uint;
use std::path::{Path, PathBuf};
use std::slice;

/// Compute CRC32 checksum for a byte buffer
/// 
/// # Safety
/// 
/// This function dereferences a raw pointer. The caller must ensure that:
/// - `ptr` points to valid memory of at least `len` bytes
/// - The memory region is readable and lives for the duration of this call
/// - The buffer is not mutated by other threads while the function runs
#[no_mangle]
pub unsafe extern "C" fn mtsfv_crc32(ptr: *const u8, len: usize) -> c_uint {
    if ptr.is_null() {
        return 0u32;
    }

    // Handle empty buffer - return proper CRC32 initial value
    if len == 0 {
        let hasher = Hasher::new();
        return hasher.finalize();
    }

    // SAFETY: Caller guarantees valid pointer/len
    let data = unsafe { slice::from_raw_parts(ptr, len) };
    
    let mut hasher = Hasher::new();
    hasher.update(data);
    hasher.finalize()
}

/// Compute CRC32 checksum for a file given its UTF-16 path
/// 
/// # Safety
/// 
/// This function reads from a raw UTF-16 string pointer. The caller must ensure that:
/// - `path_ptr` points to a valid, null-terminated UTF-16 string
/// - The pointed-to string is readable and lives for the duration of this call
/// - The string length does not exceed 32,768 characters (Windows extended MAX_PATH limit)
/// 
/// # Returns
/// 
/// Returns the CRC32 checksum on success, or 0 on error.
/// In production code, a separate error code mechanism should be used.
#[no_mangle]
pub unsafe extern "C" fn mtsfv_crc32_file(path_ptr: *const u16) -> c_uint {
    if path_ptr.is_null() {
        return 0u32;
    }

    // SAFETY: Caller guarantees valid null-terminated UTF-16 string
    // We limit the search to prevent potential issues with non-terminated strings
    let path = unsafe {
        const MAX_PATH_LEN: usize = 32768; // Windows MAX_PATH extended limit
        let mut len = 0;
        while len < MAX_PATH_LEN && *path_ptr.add(len) != 0 {
            len += 1;
        }
        if len >= MAX_PATH_LEN {
            // Path too long or not null-terminated
            return 0u32;
        }
        slice::from_raw_parts(path_ptr, len)
    };

    // Convert UTF-16 to PathBuf
    #[cfg(target_os = "windows")]
    let path_buf = {
        use std::os::windows::ffi::OsStringExt;
        PathBuf::from(std::ffi::OsString::from_wide(path))
    };
    
    #[cfg(not(target_os = "windows"))]
    let path_buf = PathBuf::from(String::from_utf16_lossy(path));

    // Open file and compute CRC32
    match compute_file_crc32(&path_buf) {
        Ok(crc) => crc,
        Err(_) => 0u32,
    }
}

/// Internal function to compute CRC32 of a file
fn compute_file_crc32(path: &Path) -> std::io::Result<u32> {
    let file = File::open(path)?;
    let mut reader = BufReader::new(file);
    let mut hasher = Hasher::new();
    let mut buffer = [0u8; 65536]; // 64KB buffer

    loop {
        let bytes_read = reader.read(&mut buffer)?;
        if bytes_read == 0 {
            break;
        }
        hasher.update(&buffer[..bytes_read]);
    }

    Ok(hasher.finalize())
}

/// Compute CRC32 checksum for a file path.
///
/// This safe helper is intended for internal callers that already have a `Path`
/// available and want a straightforward result type instead of the FFI-style API.
pub fn crc32_path(path: impl AsRef<Path>) -> std::io::Result<u32> {
    compute_file_crc32(path.as_ref())
}

/// Version information
///
/// # Safety
///
/// The returned pointer references a static, null-terminated string. Callers
/// must treat it as read-only and must not free or mutate the pointed-to memory.
#[no_mangle]
pub unsafe extern "C" fn mtsfv_version() -> *const u8 {
    "0.1.0\0".as_ptr()
}

#[cfg(test)]
mod tests {
    use super::*;

    fn crc(ptr: *const u8, len: usize) -> u32 {
        unsafe { mtsfv_crc32(ptr, len) }
    }

    #[test]
    fn test_crc32_empty() {
        let result = crc(std::ptr::null(), 0);
        assert_eq!(result, 0);
    }

    #[test]
    fn test_crc32_known_vectors() {
        // Test vector: "123456789" should produce 0xCBF43926
        let data = b"123456789";
        let result = crc(data.as_ptr(), data.len());
        assert_eq!(result, 0xCBF43926);
    }

    #[test]
    fn test_crc32_empty_string() {
        let data = b"";
        let result = crc(data.as_ptr(), data.len());
        // Empty buffer should return proper CRC32 initial value (0x00000000)
        assert_eq!(result, 0x00000000);
    }

    #[test]
    fn test_crc32_hello_world() {
        // Test with common string
        let data = b"Hello, World!";
        let result = crc(data.as_ptr(), data.len());
        // Pre-computed CRC32 for "Hello, World!"
        assert_eq!(result, 0xEC4AC3D0);
    }

    #[test]
    fn test_hasher_incremental() {
        // Test that incremental hashing works correctly
        let data = b"123456789";
        
        // Compute in one go
        let mut hasher1 = Hasher::new();
        hasher1.update(data);
        let result1 = hasher1.finalize();
        
        // Compute incrementally
        let mut hasher2 = Hasher::new();
        hasher2.update(&data[..3]);
        hasher2.update(&data[3..6]);
        hasher2.update(&data[6..]);
        let result2 = hasher2.finalize();
        
        assert_eq!(result1, result2);
        assert_eq!(result1, 0xCBF43926);
    }

    #[test]
    fn test_crc32_cpp_test_vectors() {
        // Test vectors from C++ test file (crc32.t.cpp)
        // These are the exact bytes used in the C++ test
        let data: [u8; 9] = [0x1a, 0x2b, 0x3c, 0x4f, 0x5a, 0x6b, 0x7c, 0x8d, 0x9e];
        
        // Full array should produce 0xb0c3bbc7
        let result = crc(data.as_ptr(), data.len());
        assert_eq!(result, 0xb0c3bbc7, 
            "Full array CRC32 mismatch: expected 0xb0c3bbc7, got 0x{:08x}", result);
        
        // First 5 bytes should produce 0x4a6fa7d5
        let result5 = crc(data.as_ptr(), 5);
        assert_eq!(result5, 0x4a6fa7d5,
            "First 5 bytes CRC32 mismatch: expected 0x4a6fa7d5, got 0x{:08x}", result5);
        
        // Incremental: first 5 bytes + last 4 bytes should equal full array
        let mut hasher = Hasher::new();
        hasher.update(&data[..5]);
        hasher.update(&data[5..]);
        let result_incremental = hasher.finalize();
        assert_eq!(result_incremental, 0xb0c3bbc7,
            "Incremental CRC32 mismatch: expected 0xb0c3bbc7, got 0x{:08x}", result_incremental);
    }

    #[test]
    fn test_crc32_zero_initialized() {
        // Test that empty hasher produces 0x00000000
        let hasher = Hasher::new();
        let result = hasher.finalize();
        assert_eq!(result, 0x00000000);
    }

    #[test]
    fn test_crc32_path_helper() {
        use std::fs::File;
        use std::io::Write;
        use std::time::{SystemTime, UNIX_EPOCH};

        let mut tmp_path = std::env::temp_dir();
        let unique = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap()
            .as_nanos();
        tmp_path.push(format!("mtsfv_crc_test_{}.txt", unique));

        struct TempFile(std::path::PathBuf);
        impl Drop for TempFile {
            fn drop(&mut self) {
                let _ = std::fs::remove_file(&self.0);
            }
        }
        let _cleanup = TempFile(tmp_path.clone());

        {
            let mut f = File::create(&tmp_path).expect("create temp file");
            f.write_all(b"123456789").expect("write temp data");
        }

        let crc = crc32_path(&tmp_path).expect("crc32 calculation");
        assert_eq!(crc, 0xCBF43926);
    }
}

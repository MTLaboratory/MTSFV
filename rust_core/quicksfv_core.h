/*
 *   QuickerSFV - A fast checksum verifier
 *   Copyright (C) 2025  Andreas Weis (quickersfv@andreas-weis.net)
 *
 *   This file is part of QuickerSFV.
 *
 *   QuickerSFV is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   QuickerSFV is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef QUICKSFV_CORE_H
#define QUICKSFV_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute CRC32 checksum for a byte buffer.
 * 
 * @param ptr Pointer to data buffer (must not be NULL if len > 0)
 * @param len Length of data buffer in bytes
 * @return CRC32 checksum, or 0 if ptr is NULL or len is 0
 */
uint32_t quicksfv_crc32(const uint8_t* ptr, size_t len);

/**
 * Compute CRC32 checksum for a file given its UTF-16 path.
 * 
 * @param path_ptr Pointer to null-terminated UTF-16 string (Windows path)
 * @return CRC32 checksum on success, or 0 on error
 * 
 * @note On error (file not found, read error, etc.), returns 0.
 *       In production, consider using a separate error code mechanism.
 */
uint32_t quicksfv_crc32_file(const uint16_t* path_ptr);

/**
 * Get version string of the Rust core library.
 * 
 * @return Pointer to null-terminated version string (e.g., "0.1.0")
 */
const uint8_t* quicksfv_version(void);

#ifdef __cplusplus
}
#endif

#endif /* QUICKSFV_CORE_H */

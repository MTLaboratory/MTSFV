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

/*
 * Example usage of the Rust CRC32 library from C++
 * 
 * This demonstrates how to call the Rust library from C++ code.
 * This serves as a foundation for implementing a COM shim for IExplorerCommand.
 */

#include "quicksfv_core.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

// Helper function to convert std::string to std::vector<uint16_t> for Windows paths
std::vector<uint16_t> string_to_utf16(const std::string& str) {
    std::vector<uint16_t> result;
    result.reserve(str.size() + 1);
    
    // Simple ASCII to UTF-16 conversion (for demo purposes)
    // In production, use proper UTF-8 to UTF-16 conversion
    for (char c : str) {
        result.push_back(static_cast<uint16_t>(c));
    }
    result.push_back(0); // null terminator
    
    return result;
}

int main() {
    std::cout << "QuickSFV Core - Rust Library Example\n";
    std::cout << "=====================================\n\n";
    
    // Get version
    const char* version = reinterpret_cast<const char*>(quicksfv_version());
    std::cout << "Library version: " << version << "\n\n";
    
    // Test 1: Compute CRC32 of a string
    std::string test_data = "123456789";
    uint32_t crc = quicksfv_crc32(
        reinterpret_cast<const uint8_t*>(test_data.c_str()),
        test_data.size()
    );
    
    std::cout << "Test 1: CRC32 of \"" << test_data << "\"\n";
    std::cout << "  Expected: CBF43926\n";
    std::cout << "  Got:      " << std::hex << std::uppercase << crc << "\n";
    std::cout << "  Status:   " << (crc == 0xCBF43926 ? "PASS" : "FAIL") << "\n\n";
    
    // Test 2: Compute CRC32 of another string
    test_data = "Hello, World!";
    crc = quicksfv_crc32(
        reinterpret_cast<const uint8_t*>(test_data.c_str()),
        test_data.size()
    );
    
    std::cout << "Test 2: CRC32 of \"" << test_data << "\"\n";
    std::cout << "  Expected: EC4AC3D0\n";
    std::cout << "  Got:      " << std::hex << std::uppercase << crc << "\n";
    std::cout << "  Status:   " << (crc == 0xEC4AC3D0 ? "PASS" : "FAIL") << "\n\n";
    
    // Test 3: Demonstrate file API (would need actual file)
    std::cout << "Test 3: File CRC32 API\n";
    std::cout << "  The quicksfv_crc32_file() function can be used to compute\n";
    std::cout << "  CRC32 of files directly by passing a UTF-16 encoded path.\n\n";
    
    std::cout << "All tests completed!\n";
    
    return 0;
}

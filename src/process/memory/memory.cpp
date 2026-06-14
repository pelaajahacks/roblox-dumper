#include "memory.h"
#include "../process.h"
#include <spdlog/spdlog.h>
#include <sys/ptrace.h>
#include <cstring>
#include <fstream>
#include <sstream>

namespace process {

    auto Memory::read_bytes(uintptr_t address, uint8_t* buffer, size_t size) -> bool {
        if (!g_process.get_pid() || size == 0) {
            return false;
        }

        size_t bytes_read = 0;
        const size_t word_size = sizeof(long);

        while (bytes_read < size) {
            // Calculate how much we can read from this word
            size_t offset_in_word = (address + bytes_read) % word_size;
            size_t to_read = std::min(word_size - offset_in_word, size - bytes_read);
            uintptr_t word_addr = (address + bytes_read) - offset_in_word;

            errno = 0;
            long word = ptrace(PTRACE_PEEKDATA, g_process.get_pid(), word_addr, nullptr);

            if (word == -1 && errno != 0) {
                spdlog::warn("Failed to read memory at 0x{:x}: {}", word_addr, strerror(errno));
                return bytes_read > 0;
            }

            // Copy the relevant bytes
            uint8_t* word_bytes = reinterpret_cast<uint8_t*>(&word);
            std::memcpy(buffer + bytes_read, word_bytes + offset_in_word, to_read);
            bytes_read += to_read;
        }

        return true;
    }

    auto Memory::write_bytes(uintptr_t address, const uint8_t* data, size_t size) -> bool {
        if (!g_process.get_pid() || size == 0) {
            return false;
        }

        size_t bytes_written = 0;
        const size_t word_size = sizeof(long);

        while (bytes_written < size) {
            size_t offset_in_word = (address + bytes_written) % word_size;
            size_t to_write = std::min(word_size - offset_in_word, size - bytes_written);
            uintptr_t word_addr = (address + bytes_written) - offset_in_word;

            // Read the current word value
            errno = 0;
            long word = ptrace(PTRACE_PEEKDATA, g_process.get_pid(), word_addr, nullptr);

            if (word == -1 && errno != 0) {
                spdlog::warn("Failed to read word at 0x{:x} for write: {}", word_addr, strerror(errno));
                return bytes_written > 0;
            }

            // Modify the relevant bytes
            uint8_t* word_bytes = reinterpret_cast<uint8_t*>(&word);
            std::memcpy(word_bytes + offset_in_word, data + bytes_written, to_write);

            // Write the modified word back
            if (ptrace(PTRACE_POKEDATA, g_process.get_pid(), word_addr, word) == -1) {
                spdlog::warn("Failed to write word at 0x{:x}: {}", word_addr, strerror(errno));
                return bytes_written > 0;
            }

            bytes_written += to_write;
        }

        return true;
    }

    auto Memory::read_string(uintptr_t address, size_t max_length) -> std::optional<std::string> {
        std::string result;
        uint8_t buffer[256];
        size_t chunk_size = std::min(sizeof(buffer), max_length);

        if (!read_bytes(address, buffer, chunk_size)) {
            return std::nullopt;
        }

        // Find the null terminator
        size_t str_len = std::strlen(reinterpret_cast<char*>(buffer));
        if (str_len == 0 || str_len > max_length) {
            return std::nullopt;
        }

        result.assign(reinterpret_cast<char*>(buffer), str_len);
        return result;
    }

    auto Memory::read_sso_string(uintptr_t address) -> std::optional<std::string> {
        // SSO (small string optimization) string format varies, but try to read it
        // This is a simplified implementation
        const size_t sso_threshold = 24;  // Typical threshold for 64-bit systems
        uint8_t buffer[sso_threshold];

        if (!read_bytes(address, buffer, sso_threshold)) {
            return std::nullopt;
        }

        // Check if it's using SSO (small string is stored inline)
        // Size is usually stored at offset 0, capacity at offset 8
        size_t* size_ptr = reinterpret_cast<size_t*>(buffer);
        if (*size_ptr < sso_threshold && *size_ptr > 0) {
            // SSO string - data is inline
            std::string result(reinterpret_cast<char*>(buffer + 16), *size_ptr);
            return result;
        }

        return std::nullopt;
    }

    auto Memory::scan_string(const std::string& target, std::string_view section) -> std::vector<uintptr_t> {
        std::vector<uintptr_t> results;
        // This would require more complex implementation with section scanning
        // For now, return empty - would need access to process maps
        spdlog::warn("String scanning not yet fully implemented for Linux");
        return results;
    }

} // namespace process

#pragma once
#include "process/process.h"
#include <optional>
#include <string>
#include <vector>
#include <sys/ptrace.h>
#include <cstring>

namespace process {
    class Memory {
      public:
        template <typename T> static auto read(uintptr_t address) -> std::optional<T> {
            T buffer{};
            if (!read_bytes(address, reinterpret_cast<uint8_t*>(&buffer), sizeof(T))) {
                return std::nullopt;
            }
            return buffer;
        }

        template <typename T> static auto write(uintptr_t address, const T& value) -> bool {
            return write_bytes(address, reinterpret_cast<const uint8_t*>(&value), sizeof(T));
        }

        static auto read_bytes(uintptr_t address, uint8_t* buffer, size_t size) -> bool;
        static auto write_bytes(uintptr_t address, const uint8_t* data, size_t size) -> bool;
        static auto read_string(uintptr_t address, size_t max_length = 256)
            -> std::optional<std::string>;
        static auto read_sso_string(uintptr_t address) -> std::optional<std::string>;
        static auto scan_string(const std::string& target, std::string_view section = "")
            -> std::vector<uintptr_t>;
    };
} // namespace process

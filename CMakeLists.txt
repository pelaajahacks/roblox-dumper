cmake_minimum_required(VERSION 3.20)

project("roblox-dumper" VERSION 2.7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Platform-specific compiler flags
if(MSVC)
    # Windows MSVC build
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elif(UNIX AND NOT APPLE)
    # Linux/Unix build
    add_compile_options(-fPIC -Wno-unknown-pragmas)
    # Enable ptrace debugging
    add_compile_definitions(_GNU_SOURCE)
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

include(FetchContent)

set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_ENABLE_PCH ON CACHE BOOL "" FORCE)

set(BUILD_CPR_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# Platform-specific curl options
if(MSVC OR WIN32)
    # Windows: Use native Windows TLS (SChannel)
    set(CURL_STATICLIB ON CACHE BOOL "" FORCE)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
else()
    # Linux/Unix: Use OpenSSL for TLS
    set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)
    # Find system OpenSSL
    find_package(OpenSSL REQUIRED)
endif()

set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)
set(USE_LIBIDN2 OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAPS ON CACHE BOOL "" FORCE)
set(USE_NGHTTP2 OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5
)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.15.3
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_Declare(
    zydis
    GIT_REPOSITORY https://github.com/zyantific/zydis.git
    GIT_TAG v4.1.1
)

FetchContent_MakeAvailable(spdlog glm cpr httplib json zydis)

include_directories(src ${PROJECT_BINARY_DIR})

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.cxx" "src/*.cc")

add_executable(roblox-dumper ${SOURCES})

# Link dependencies
if(MSVC OR WIN32)
    set_property(TARGET roblox-dumper PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_libraries(roblox-dumper PRIVATE spdlog::spdlog glm::glm cpr::cpr httplib::httplib nlohmann_json::nlohmann_json Zydis)
else()
    # Linux/Unix: Link with system OpenSSL and ptrace support
    target_link_libraries(roblox-dumper PRIVATE 
        spdlog::spdlog 
        glm::glm 
        cpr::cpr 
        httplib::httplib 
        nlohmann_json::nlohmann_json 
        Zydis
        OpenSSL::Crypto
        OpenSSL::SSL
        dl  # For dlopen/dlsym used in process attachment
    )
endif()

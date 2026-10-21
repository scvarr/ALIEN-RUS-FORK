set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/resources/locales/en.json")
file(READ "${PROJECT_SOURCE_DIR}/resources/locales/en.json" ALIEN_DEFAULT_LOCALE)
# Keep each raw literal below the MSVC limit while preserving the complete catalog.
string(REPLACE "\n" "\n)ALIEN_LOCALE\"\nR\"ALIEN_LOCALE(" ALIEN_DEFAULT_LOCALE "${ALIEN_DEFAULT_LOCALE}")
configure_file(DefaultLocale.h.in "${CMAKE_CURRENT_BINARY_DIR}/DefaultLocale.h" @ONLY)


add_library(Base
    AlienExceptions.cpp
    AlienExceptions.h
    Cache.h
    Console.cpp
    Console.h
    Definitions.cpp
    Definitions.h
    DocumentationReader.cpp
    DocumentationReader.h
    ExitScopeGuard.h
    FileLogger.cpp
    FileLogger.h
    GlobalSettings.cpp
    GlobalSettings.h
    Hashes.h
    Ids.h
    ImageFileService.cpp
    ImageFileService.h
    JsonParser.h
    KernelProfiler.cpp
    KernelProfiler.h
    KernelTracer.cpp
    KernelTracer.h
    LocalizationService.cpp
    LocalizationService.h
    LoggingService.cpp
    LoggingService.h
    Macros.h
    MarkdownDocument.h
    MarkdownParser.cpp
    MarkdownParser.h
    Math.cpp
    Math.h
    MathTypes.cpp
    MathTypes.h
    NameGeneratorService.cpp
    NameGeneratorService.h
    NumberGenerator.cpp
    NumberGenerator.h
    Physics.cpp
    Physics.h
    Resources.cpp
    Resources.h
    RgbImage.h
    Singleton.h
    StringHelper.cpp
    StringHelper.h
    UnlockGuard.h
    VersionParserService.cpp
    VersionParserService.h
    WebLinkHelper.cpp
    WebLinkHelper.h)

target_include_directories(Base PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(Base Boost::json)
target_link_libraries(Base Boost::boost)
target_link_libraries(Base imgui::imgui)
target_link_libraries(Base md4c::md4c)

if (WIN32)
    target_link_libraries(Base dbghelp)
endif()

target_precompile_headers(Base PRIVATE
    <algorithm>
    <array>
    <chrono>
    <cstdint>
    <filesystem>
    <functional>
    <initializer_list>
    <list>
    <map>
    <memory>
    <mutex>
    <optional>
    <set>
    <sstream>
    <string>
    <unordered_map>
    <unordered_set>
    <variant>
    <vector>
)

if (MSVC)
    target_compile_options(Base PRIVATE "/MP")
endif()

if (MSVC)
    target_compile_options(Base PRIVATE /utf-8)
endif()

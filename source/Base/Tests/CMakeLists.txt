target_sources(BaseTests
PUBLIC
    DocumentationReaderTests.cpp
    LocalizationServiceTests.cpp
    MarkdownParserTests.cpp
    StringHelperTests.cpp
    Testsuite.cpp)

target_link_libraries(BaseTests Base)
target_compile_definitions(BaseTests PRIVATE ALIEN_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
if (MSVC)
    target_compile_options(BaseTests PRIVATE /utf-8)
endif()

target_link_libraries(BaseTests GTest::GTest GTest::Main)

target_precompile_headers(BaseTests PRIVATE
    <string>
    <vector>
    <gtest/gtest.h>
)

if (MSVC)
    target_compile_options(BaseTests PRIVATE "/MP")
endif()

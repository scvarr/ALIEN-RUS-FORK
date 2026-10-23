target_sources(GuiLocalizationTests PRIVATE LocalizationGuiTests.cpp BrowserLocalizationTests.cpp)
target_link_libraries(GuiLocalizationTests PRIVATE
    GuiImpl RenderingImpl McpToolsImpl PersisterImpl EngineImpl EngineKernels GTest::gtest_main)
target_include_directories(GuiLocalizationTests PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
if (MSVC)
    target_compile_options(GuiLocalizationTests PRIVATE /utf-8)
endif()

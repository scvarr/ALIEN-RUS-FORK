target_sources(GuiLocalizationTests PRIVATE LocalizationGuiTests.cpp BrowserLocalizationTests.cpp)
target_link_libraries(GuiLocalizationTests PRIVATE
    GuiImpl RenderingImpl McpToolsImpl PersisterImpl EngineImpl EngineKernels GTest::gtest_main)
target_include_directories(GuiLocalizationTests PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
if (MSVC)
    target_compile_options(GuiLocalizationTests PRIVATE /utf-8 /bigobj)
endif()

add_executable(AccountLocalizationTests AccountLocalizationTests.cpp AccountNetworkStub.cpp)
set_target_properties(AccountLocalizationTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
add_dependencies(AccountLocalizationTests LocalizationResources)
target_link_libraries(AccountLocalizationTests PRIVATE
    GuiImpl RenderingImpl McpToolsImpl PersisterImpl EngineImpl EngineKernels GTest::gtest_main)
target_include_directories(AccountLocalizationTests PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
if (MSVC)
    target_compile_options(AccountLocalizationTests PRIVATE /utf-8)
endif()

add_executable(McpLocalizationTests McpLocalizationTests.cpp McpServerStub.cpp)
set_target_properties(McpLocalizationTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
add_dependencies(McpLocalizationTests LocalizationResources)
target_link_libraries(McpLocalizationTests PRIVATE
    GuiImpl RenderingImpl McpToolsImpl PersisterImpl EngineImpl EngineKernels GTest::gtest_main)
target_include_directories(McpLocalizationTests PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
if (MSVC)
    target_compile_options(McpLocalizationTests PRIVATE /utf-8)
endif()

#pragma once

#ifdef _WIN32
#include <Windows.h>

#include <stdexcept>
#include <string>

// Redirect only this test process; an independently running ALIEN keeps its real profile.
class SettingsTestScope
{
public:
    SettingsTestScope()
    {
        _path = L"Software\\alien-localization-tests-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64());
        if (RegCreateKeyExW(HKEY_CURRENT_USER, _path.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &_key, nullptr) != ERROR_SUCCESS) {
            throw std::runtime_error("Cannot create isolated settings registry");
        }
        if (RegOverridePredefKey(HKEY_CURRENT_USER, _key) != ERROR_SUCCESS) {
            RegCloseKey(_key);
            RegDeleteTreeW(HKEY_CURRENT_USER, _path.c_str());
            throw std::runtime_error("Cannot isolate test settings");
        }
    }

    ~SettingsTestScope()
    {
        RegOverridePredefKey(HKEY_CURRENT_USER, nullptr);
        RegCloseKey(_key);
        RegDeleteTreeW(HKEY_CURRENT_USER, _path.c_str());
    }

    SettingsTestScope(SettingsTestScope const&) = delete;
    SettingsTestScope& operator=(SettingsTestScope const&) = delete;

private:
    HKEY _key = nullptr;
    std::wstring _path;
};
#else
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

class SettingsTestScope
{
public:
    SettingsTestScope()
    {
        // This directory outlives GlobalSettings, whose destructor also saves settings.
        static ProfileDirectory profile;
    }

private:
    struct ProfileDirectory
    {
        ProfileDirectory()
        {
            path = std::filesystem::temp_directory_path()
                / ("alien-settings-tests-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
            std::filesystem::create_directory(path);
            if (setenv("XDG_CONFIG_HOME", path.c_str(), 1) != 0) {
                throw std::runtime_error("Cannot isolate test settings");
            }
        }

        ~ProfileDirectory() { std::filesystem::remove_all(path); }
        std::filesystem::path path;
    };
};
#endif

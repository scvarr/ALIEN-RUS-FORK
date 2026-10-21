#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <system_error>

namespace PathHelper
{
    inline std::string toUtf8(std::filesystem::path const& path)
    {
        auto text = path.u8string();
        return {reinterpret_cast<char const*>(text.data()), text.size()};
    }

    inline std::filesystem::path fromUtf8(std::string const& text)
    {
        return std::filesystem::path(std::u8string(text.begin(), text.end()));
    }

    // The legacy file dialog accepts locale-encoded starting directories.
    inline std::optional<std::string> toLegacyString(std::filesystem::path const& path)
    {
        try {
            auto text = path.string();
            if (std::filesystem::path(text) == path) {
                return text;
            }
        } catch (std::system_error const&) {
        }
        return std::nullopt;
    }
}

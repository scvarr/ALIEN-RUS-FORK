#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Utf8Search
{
    struct Character
    {
        uint32_t folded;
        size_t begin;
        size_t end;
    };

    struct Match
    {
        size_t begin;
        size_t end;
    };

    // Strict decoding keeps matching and highlighting on complete UTF-8 characters.
    inline std::optional<std::vector<Character>> decode(std::string const& text)
    {
        std::vector<Character> result;
        for (size_t i = 0; i < text.size();) {
            auto begin = i;
            auto lead = static_cast<unsigned char>(text.at(i++));
            uint32_t code = lead;
            int remaining = 0;
            uint32_t minimum = 0;
            if (lead >= 0xc2 && lead <= 0xdf) {
                code = lead & 0x1f;
                remaining = 1;
                minimum = 0x80;
            } else if (lead >= 0xe0 && lead <= 0xef) {
                code = lead & 0x0f;
                remaining = 2;
                minimum = 0x800;
            } else if (lead >= 0xf0 && lead <= 0xf4) {
                code = lead & 0x07;
                remaining = 3;
                minimum = 0x10000;
            } else if (lead >= 0x80) {
                return std::nullopt;
            }
            while (remaining-- > 0) {
                if (i == text.size()) {
                    return std::nullopt;
                }
                auto next = static_cast<unsigned char>(text.at(i++));
                if ((next & 0xc0) != 0x80) {
                    return std::nullopt;
                }
                code = (code << 6) | (next & 0x3f);
            }
            if (code < minimum || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff)) {
                return std::nullopt;
            }
            if ((code >= 'A' && code <= 'Z') || (code >= 0x0410 && code <= 0x042f)) {
                code += 0x20;
            } else if (code == 0x0401) {
                code = 0x0451;
            }
            result.push_back({code, begin, i});
        }
        return result;
    }

    inline std::optional<Match> find(std::string const& text, std::string const& query)
    {
        auto characters = decode(text);
        auto needle = decode(query);
        if (!characters || !needle || needle->empty() || needle->size() > characters->size()) {
            return std::nullopt;
        }
        for (size_t i = 0; i <= characters->size() - needle->size(); ++i) {
            size_t j = 0;
            while (j < needle->size() && characters->at(i + j).folded == needle->at(j).folded) {
                ++j;
            }
            if (j == needle->size()) {
                return Match{characters->at(i).begin, characters->at(i + j - 1).end};
            }
        }
        return std::nullopt;
    }
}

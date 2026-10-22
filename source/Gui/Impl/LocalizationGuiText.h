#pragma once

#include <map>
#include <string>

#include <Base/LocalizationService.h>

namespace LocalizationGuiText
{
    inline std::string format(std::string const& key, std::map<std::string, std::string> const& arguments)
    {
        auto text = tr(key);
        std::string result;
        for (size_t position = 0; position < text.size();) {
            auto end = text.find('}', position);
            if (text.at(position) == '{' && end != std::string::npos) {
                auto argument = arguments.find(text.substr(position + 1, end - position - 1));
                if (argument != arguments.end()) {
                    result += argument->second;
                    position = end + 1;
                    continue;
                }
            }
            result += text.at(position++);
        }
        return result;
    }
}

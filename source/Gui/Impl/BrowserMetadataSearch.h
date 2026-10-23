#pragma once

#include <Network/NetworkResourceRawTO.h>

#include "Utf8Search.h"

namespace BrowserMetadataSearch
{
    inline bool matches(_NetworkResourceRawTO const& resource, std::string const& filter)
    {
        if (filter.empty()) {
            return true;
        }
        auto contains = [&](std::string const& field) { return Utf8Search::find(field, filter).has_value(); };
        return contains(resource.timestamp) || contains(resource.userName) || contains(resource.resourceName) || contains(std::to_string(resource.numDownloads))
            || contains(std::to_string(resource.width)) || contains(std::to_string(resource.height)) || contains(std::to_string(resource.particles))
            || contains(std::to_string(resource.contentSize)) || contains(resource.description) || contains(resource.version);
    }
}

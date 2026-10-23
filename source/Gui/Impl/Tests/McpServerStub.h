#pragma once

#include <Network/McpServer.h>

namespace McpServerStub
{
    inline bool available = true;
    inline std::vector<int> starts;
    inline int stops = 0;
    inline std::string name;
    inline std::string instructions;
    inline std::vector<McpTool> tools;
}

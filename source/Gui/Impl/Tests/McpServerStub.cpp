#include "McpServerStub.h"

#include <gtest/gtest.h>

// Link-time isolation: this executable never creates an HTTP listener.
namespace httplib
{
    class Server
    {};
}

McpServer::McpServer(std::string const& serverName, std::string const& serverVersion, std::string const& instructions, std::vector<McpTool> tools)
{
    McpServerStub::name = serverName;
    McpServerStub::instructions = instructions;
    McpServerStub::tools = std::move(tools);
}

McpServer::~McpServer() = default;

bool McpServer::start(int port)
{
    McpServerStub::starts.push_back(port);
    _port = port;
    return McpServerStub::available;
}

void McpServer::stop()
{
    ++McpServerStub::stops;
}

bool McpServer::isRunning() const
{
    return McpServerStub::available;
}

int McpServer::getPort() const
{
    return _port;
}

std::optional<std::string> McpServer::handleMessage(std::string const& message) const
{
    ADD_FAILURE() << "Unexpected transport call in isolated GUI test";
    return std::nullopt;
}

#include "AccountNetworkStub.h"

#include <gtest/gtest.h>
#include <Network/NetworkService.h>

void NetworkService::setup()
{
    ADD_FAILURE() << "Unexpected isolated network operation: setup";
}

void NetworkService::shutdown()
{
    ADD_FAILURE() << "Unexpected isolated network operation: shutdown";
}

std::string NetworkService::getServerAddress()
{
    ADD_FAILURE() << "Unexpected isolated network operation: getServerAddress";
    return {};
}

void NetworkService::setServerAddress(std::string const& value)
{
    ADD_FAILURE() << "Unexpected isolated network operation: setServerAddress";
}

bool NetworkService::isCountAsActive()
{
    ADD_FAILURE() << "Unexpected isolated network operation: isCountAsActive";
    return {};
}

void NetworkService::setCountAsActive(bool value)
{
    ADD_FAILURE() << "Unexpected isolated network operation: setCountAsActive";
}

bool NetworkService::isLoggedIn()
{
    return AccountNetworkStub::loggedIn;
}

std::optional<std::string> NetworkService::getLoggedInUserName()
{
    return AccountNetworkStub::loggedIn ? std::optional<std::string>(AccountNetworkStub::user) : std::nullopt;
}

std::optional<std::string> NetworkService::getPassword()
{
    return AccountNetworkStub::password;
}

bool NetworkService::createUser(std::string const& userName, std::string const& password, std::string const& email)
{
    AccountNetworkStub::calls.emplace_back("create");
    AccountNetworkStub::arguments = {userName, password, email};
    return AccountNetworkStub::success;
}

bool NetworkService::activateUser(std::string const& userName, std::string const& password, UserInfo const& userInfo, std::string const& confirmationCode)
{
    AccountNetworkStub::calls.emplace_back("activate");
    AccountNetworkStub::arguments = {userName, password, confirmationCode};
    return AccountNetworkStub::success;
}

bool NetworkService::login(LoginErrorCode& errorCode, std::string const& userName, std::string const& password, UserInfo const& userInfo)
{
    AccountNetworkStub::calls.emplace_back("login");
    AccountNetworkStub::arguments = {userName, password};
    AccountNetworkStub::loggedIn = AccountNetworkStub::success;
    return AccountNetworkStub::success;
}

bool NetworkService::logout()
{
    AccountNetworkStub::calls.emplace_back("logout");
    AccountNetworkStub::loggedIn = false;
    return true;
}

void NetworkService::refreshLogin()
{
    ADD_FAILURE() << "Unexpected isolated network operation: refreshLogin";
}

void NetworkService::setUserInfo(UserInfo const& value)
{
    AccountNetworkStub::calls.emplace_back("info");
}

bool NetworkService::deleteUser()
{
    AccountNetworkStub::calls.emplace_back("delete");
    if (AccountNetworkStub::success) {
        AccountNetworkStub::loggedIn = false;
    }
    return AccountNetworkStub::success;
}

bool NetworkService::resetPassword(std::string const& userName, std::string const& email)
{
    AccountNetworkStub::calls.emplace_back("reset");
    AccountNetworkStub::arguments = {userName, email};
    return AccountNetworkStub::success;
}

bool NetworkService::setNewPassword(std::string const& userName, std::string const& newPassword, std::string const& confirmationCode)
{
    AccountNetworkStub::calls.emplace_back("new");
    AccountNetworkStub::arguments = {userName, newPassword, confirmationCode};
    return AccountNetworkStub::success;
}

bool NetworkService::getNetworkResourceList(std::vector<NetworkResourceRawTO>& result, bool withRetry)
{
    ADD_FAILURE() << "Unexpected isolated network operation: getNetworkResourceList";
    return {};
}

bool NetworkService::getResourcePictures(std::unordered_map<std::string, std::string>& jpgBySimId, std::vector<std::string> const& simIds)
{
    ADD_FAILURE() << "Unexpected isolated network operation: getResourcePictures";
    return {};
}

bool NetworkService::getUserList(std::vector<UserTO>& result, bool withRetry)
{
    ADD_FAILURE() << "Unexpected isolated network operation: getUserList";
    return {};
}

std::optional<int> NetworkService::getNumActiveSimulators()
{
    ADD_FAILURE() << "Unexpected isolated network operation: getNumActiveSimulators";
    return {};
}

bool NetworkService::getEmojiTypeByResourceId(std::unordered_map<std::string, int>& result)
{
    ADD_FAILURE() << "Unexpected isolated network operation: getEmojiTypeByResourceId";
    return {};
}

bool NetworkService::getUserNamesForResourceAndEmojiType(std::set<std::string>& result, std::string const& simId, int likeType)
{
    ADD_FAILURE() << "Unexpected isolated network operation: getUserNamesForResourceAndEmojiType";
    return {};
}

bool NetworkService::toggleReactionForResource(std::string const& simId, int likeType)
{
    ADD_FAILURE() << "Unexpected isolated network operation: toggleReactionForResource";
    return {};
}

bool NetworkService::uploadResource(
    std::string& resourceId,
    std::string const& resourceName,
    std::string const& description,
    IntVector2D const& worldSize,
    int numObjects,
    std::string const& data,
    std::string const& picture,
    NetworkResourceType resourceType,
    WorkspaceType workspaceType)
{
    ADD_FAILURE() << "Unexpected isolated network operation: uploadResource";
    return {};
}

bool NetworkService::replaceResource(
    std::string const& resourceId,
    IntVector2D const& worldSize,
    int numObjects,
    std::string const& data,
    std::string const& picture)
{
    ADD_FAILURE() << "Unexpected isolated network operation: replaceResource";
    return {};
}

bool NetworkService::downloadResource(std::string& mainData, std::string const& simId)
{
    ADD_FAILURE() << "Unexpected isolated network operation: downloadResource";
    return {};
}

void NetworkService::incDownloadCounter(std::string const& simId)
{
    ADD_FAILURE() << "Unexpected isolated network operation: incDownloadCounter";
}

bool NetworkService::editResource(std::string const& simId, std::string const& newName, std::string const& newDescription)
{
    ADD_FAILURE() << "Unexpected isolated network operation: editResource";
    return {};
}

bool NetworkService::moveResource(std::string const& simId, WorkspaceType targetWorkspace)
{
    ADD_FAILURE() << "Unexpected isolated network operation: moveResource";
    return {};
}

bool NetworkService::deleteResource(std::string const& simId)
{
    ADD_FAILURE() << "Unexpected isolated network operation: deleteResource";
    return {};
}

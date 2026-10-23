#pragma once

#include <Persister/Impl/PersisterFacadeImpl.h>
#include <Rendering/Interface/RenderingFacade.h>

namespace
{
    class AccountRenderStub : public _RenderingFacade
    {
    public:
        static void set(RenderingFacade const& value) { _instance = value; }
        void setWindowHints() override {}
        void setup(GLFWwindow* window) override {}
        void setupSimulationRendering(ImFont* labelFont) override {}
        void shutdown() override {}
        void newFrame() override {}
        void drawSimulation(RenderView const& view) override {}
        void clearScreen(FloatColorRGB const& color) override {}
        void render(ImDrawData* drawData) override {}
        PictureData renderSimulationPicture(RenderView const& view) override { return {}; }
        PictureData renderDrawList(ImDrawList* drawList, IntVector2D const& resolution, ImColor const& backgroundColor, int supersampling) override
        {
            return {};
        }
        TextureData loadTexture(std::filesystem::path const& filename) override { return {}; }
        TextureData loadTextureFromMemory(std::string const& encodedImage) override { return {}; }
        TextureData createTexture(uint8_t const* pixels, int width, int height, TextureFormat format, TextureFilter filter) override { return {}; }
        void deleteTexture(TextureData const& texture) override {}
        void deleteTexture(ImTextureID textureId) override {}
    };

    class AccountPersisterStub : public _PersisterFacade
    {
    public:
        GetNetworkResourcesResultData result;
        std::optional<PersisterRequestState> state = PersisterRequestState::InProgress;
        std::vector<PersisterErrorInfo> errors;
        int scheduled = 0;
        LoginRequestData loginRequest;
        LoginResultData loginResult;
        void setup() override {}
        void shutdown() override {}
        void restart() override {}
        bool isBusy() const override { return {}; }
        std::optional<PersisterRequestState> getRequestState(PersisterRequestId const& id) const override { return state; }
        PersisterRequestResult fetchPersisterRequestResult(PersisterRequestId const& id) override { return {}; }
        std::vector<PersisterErrorInfo> fetchAllErrorInfos(SenderId const& senderId) override { return std::exchange(errors, {}); }
        PersisterErrorInfo fetchError(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleSaveSimulation(SenderInfo const& senderInfo, SaveSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        SaveSimulationResultData fetchSaveSimulationData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleReadSimulation(SenderInfo const& senderInfo, ReadSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        ReadSimulationResultData fetchReadSimulationData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleLogin(SenderInfo const& senderInfo, LoginRequestData const& data) override
        {
            loginRequest = data;
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        LoginResultData fetchLoginData(PersisterRequestId const& id) override { return loginResult; }
        PersisterRequestId scheduleGetNetworkResources(SenderInfo const& senderInfo, GetNetworkResourcesRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetNetworkResourcesResultData fetchGetNetworkResourcesData(PersisterRequestId const& id) override { return result; }
        PersisterRequestId scheduleDownloadNetworkResource(SenderInfo const& senderInfo, DownloadNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        DownloadNetworkResourceResultData fetchDownloadNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleUploadNetworkResource(SenderInfo const& senderInfo, UploadNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        UploadNetworkResourceResultData fetchUploadNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleReplaceNetworkResource(SenderInfo const& senderInfo, ReplaceNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        ReplaceNetworkResourceResultData fetchReplaceNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleGetSimulationPictures(SenderInfo const& senderInfo, GetSimulationPicturesRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetSimulationPicturesResultData fetchGetSimulationPicturesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleGetUserNamesForReaction(SenderInfo const& senderInfo, GetUserNamesForReactionRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetUserNamesForReactionResultData fetchGetUserNamesForReactionData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleDeleteNetworkResource(SenderInfo const& senderInfo, DeleteNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        DeleteNetworkResourceResultData fetchDeleteNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleEditNetworkResource(SenderInfo const& senderInfo, EditNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        EditNetworkResourceResultData fetchEditNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleMoveNetworkResource(SenderInfo const& senderInfo, MoveNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        MoveNetworkResourceResultData fetchMoveNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleToggleReactionNetworkResource(SenderInfo const& senderInfo, ToggleReactionNetworkResourceRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        ToggleReactionNetworkResourceResultData fetchToggleReactionNetworkResourcesData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleGetPeakSimulation(SenderInfo const& senderInfo, GetPeakSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        GetPeakSimulationResultData fetchGetPeakSimulationData(PersisterRequestId const& id) override { return {}; }
        PersisterRequestId scheduleSaveDeserializedSimulation(SenderInfo const& senderInfo, SaveDeserializedSimulationRequestData const& data) override
        {
            return {"isolated-browser-" + std::to_string(++scheduled)};
        }
        SaveDeserializedSimulationResultData fetchSaveDeserializedSimulationData(PersisterRequestId const& id) override { return {}; }
    };

}

#include "CellFunctionChannels.h"

#include <ranges>

#include <Base/LocalizationService.h>

#include "GenomeEditorPresentation.h"

#include <Data/CellTypeConstants.h>

namespace
{
    // Cell properties determining which channels the cell functions of a cell access
    struct CellProperties
    {
        CellType cellType = CellType_Base;
        MuscleMode muscleMode = MuscleMode_AutoBending;
        MemoryMode memoryMode = MemoryMode_SignalDelay;
        uint16_t memoryChannelBitMask = Const::MemoryChannelBitMask_Max;
        CommunicatorMode communicatorMode = CommunicatorMode_Sender;
        bool constructorAvailable = false;
        bool constructorManuallyTriggered = false;
    };

    CellFunctionChannel readChannel(int channel, std::string const& label)
    {
        return {channel, label, ""};
    }

    CellFunctionChannel writeChannel(int channel, std::string const& label)
    {
        return {channel, "", label};
    }

    std::vector<CellFunctionChannel> getSensorChannels()
    {
        // The trigger channel also selects the relocation scan and is therefore read even with auto triggering
        return {
            {Channels::SensorFoundResult, tr("neural.channel.trigger"), tr("neural.channel.found")},
            writeChannel(Channels::SensorAngle, tr("neural.channel.angle")),
            writeChannel(Channels::SensorMass, tr("neural.channel.mass")),
            writeChannel(Channels::SensorDistance, tr("neural.channel.distance"))};
    }

    std::vector<CellFunctionChannel> getMuscleChannels(CellProperties const& properties)
    {
        std::vector<CellFunctionChannel> result{readChannel(Channels::CellTypeActivation, tr("neural.channel.activation"))};
        auto usesAngle = properties.muscleMode == MuscleMode_AutoBending || properties.muscleMode == MuscleMode_AngleBending
            || properties.muscleMode == MuscleMode_DirectMovement;
        if (usesAngle) {
            result.emplace_back(readChannel(Channels::MuscleAngle, tr("neural.channel.angle")));
        }
        return result;
    }

    std::vector<CellFunctionChannel> getMemoryChannels(CellProperties const& properties)
    {
        std::string writeLabel;
        auto readsAction = false;
        switch (properties.memoryMode) {
        case MemoryMode_SignalDelay:
            writeLabel = tr("neural.channel.delayed");
            break;
        case MemoryMode_SignalRecorder:
            writeLabel = tr("neural.channel.replayed");
            readsAction = true;
            break;
        case MemoryMode_SignalStorage:
            writeLabel = tr("neural.channel.restored");
            readsAction = true;
            break;
        case MemoryMode_SignalIntegrator:
            writeLabel = tr("neural.channel.smoothed");
            break;
        }

        std::vector<CellFunctionChannel> result;
        for (auto channel : std::views::iota(0, STANDARD_NEURONS_PER_CELL)) {
            auto isRead = readsAction && channel == Channels::MemoryReadWriteAction;
            auto isWritten = (properties.memoryChannelBitMask & (1 << channel)) != 0;
            if (isRead || isWritten) {
                result.emplace_back(channel, isRead ? tr("neural.channel.readwrite") : "", isWritten ? writeLabel : "");
            }
        }
        return result;
    }

    std::vector<CellFunctionChannel> getCommunicatorChannels(CellProperties const& properties)
    {
        if (properties.communicatorMode == CommunicatorMode_Sender) {
            return {
                readChannel(Channels::CellTypeActivation, tr("neural.channel.trigger")), readChannel(Channels::CommunicatorAngle, tr("neural.channel.angle"))};
        }

        // The angle is written by the sending cell into the signal of this cell
        return {writeChannel(Channels::CommunicatorAngle, tr("neural.channel.angle"))};
    }

    std::vector<CellFunctionChannel> getCellTypeChannels(CellProperties const& properties)
    {
        switch (properties.cellType) {
        case CellType_Depot:
            return {readChannel(Channels::CellTypeActivation, tr("neural.channel.transfer"))};
        case CellType_Sensor:
            return getSensorChannels();
        case CellType_Generator:
            return {writeChannel(Channels::GeneratorOutput, tr("neural.channel.value"))};
        case CellType_Attacker:
            return {
                readChannel(Channels::CellTypeActivation, tr("neural.channel.trigger")), writeChannel(Channels::AttackerSuccess, tr("neural.channel.success"))};
        case CellType_Injector:
            return {
                readChannel(Channels::CellTypeActivation, tr("neural.channel.trigger")), writeChannel(Channels::InjectorSuccess, tr("neural.channel.success"))};
        case CellType_Muscle:
            return getMuscleChannels(properties);
        case CellType_Reconnector:
            return {
                readChannel(Channels::CellTypeActivation, tr("neural.channel.trigger")),
                writeChannel(Channels::ReconnectorSuccess, tr("neural.channel.success"))};
        case CellType_Detonator:
            return {readChannel(Channels::CellTypeActivation, tr("neural.channel.trigger"))};
        case CellType_Memory:
            return getMemoryChannels(properties);
        case CellType_Communicator:
            return getCommunicatorChannels(properties);
        }
        return {};
    }

    // The names are drawn vertically next to the accessed channels and are therefore abbreviated to fit into a few rows
    std::string getModuleName(CellType cellType)
    {
        switch (cellType) {
        case CellType_Generator:
            return tr("neural.module.gen");
        case CellType_Attacker:
            return tr("neural.module.attack");
        case CellType_Injector:
            return tr("neural.module.inject");
        case CellType_Reconnector:
            return tr("neural.module.recon");
        case CellType_Detonator:
            return tr("neural.module.deton");
        case CellType_Communicator:
            return tr("neural.module.comm");
        }
        return GenomeEditorPresentation::cellTypes().at(cellType);
    }

    std::vector<CellFunctionModule> createModules(CellProperties const& properties)
    {
        std::vector<CellFunctionModule> result;

        auto cellTypeChannels = getCellTypeChannels(properties);
        if (!cellTypeChannels.empty()) {
            result.emplace_back(getModuleName(properties.cellType), cellTypeChannels);
        }

        // The constructor is a separate module because it is available independently of the cell type
        if (properties.constructorAvailable) {
            std::vector<CellFunctionChannel> constructorChannels;
            if (properties.constructorManuallyTriggered) {
                constructorChannels.emplace_back(readChannel(Channels::CellTypeActivation, tr("neural.channel.trigger")));
            }
            constructorChannels.emplace_back(writeChannel(Channels::ConstructorSuccess, tr("neural.channel.success")));
            result.emplace_back(tr("neural.module.constructor"), constructorChannels);
        }

        return result;
    }
}

std::vector<CellFunctionModule> CellFunctionChannels::getModules(NodeDesc const& node)
{
    CellProperties properties;
    properties.cellType = node.getCellType();
    if (auto const* muscle = std::get_if<MuscleGenomeDesc>(&node._cellType)) {
        properties.muscleMode = muscle->getMode();
    }
    if (auto const* memory = std::get_if<MemoryGenomeDesc>(&node._cellType)) {
        properties.memoryMode = memory->getMode();
        properties.memoryChannelBitMask = memory->_channelBitMask;
    }
    if (auto const* communicator = std::get_if<CommunicatorGenomeDesc>(&node._cellType)) {
        properties.communicatorMode = communicator->getMode();
    }
    properties.constructorAvailable = node._constructor.has_value();
    properties.constructorManuallyTriggered = properties.constructorAvailable && !node._constructor->_autoTriggerInterval.has_value();

    return createModules(properties);
}

std::vector<CellFunctionModule> CellFunctionChannels::getModules(CellDesc const& cell)
{
    CellProperties properties;
    properties.cellType = cell.getCellType();
    if (auto const* muscle = std::get_if<MuscleDesc>(&cell._cellType)) {
        properties.muscleMode = muscle->getMode();
    }
    if (auto const* memory = std::get_if<MemoryDesc>(&cell._cellType)) {
        properties.memoryMode = memory->getMode();
        properties.memoryChannelBitMask = memory->_channelBitMask;
    }
    if (auto const* communicator = std::get_if<CommunicatorDesc>(&cell._cellType)) {
        properties.communicatorMode = communicator->getMode();
    }
    properties.constructorAvailable = cell._constructor.has_value();
    properties.constructorManuallyTriggered = properties.constructorAvailable && !cell._constructor->_autoTriggerInterval.has_value();

    return createModules(properties);
}

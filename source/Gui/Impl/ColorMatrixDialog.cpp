#include "ColorMatrixDialog.h"

#include <algorithm>
#include <ranges>

#include <imgui.h>
#include <imgui_internal.h>
#include <Base/LocalizationService.h>

#include <Base/NumberGenerator.h>

#include "AlienGui.h"
#include "StyleService.h"

namespace
{
    auto const DialogSize = RealVector2D(560.0f, 480.0f);
    auto constexpr ButtonAreaHeight = 50.0f;
    auto constexpr LocalizedButtonAreaHeight = 110.0f;
    auto constexpr MaxCellSizeForSlider = 40.0f;
    auto constexpr MaxCellSizeForCheckbox = 28.0f;

    float buttonWidth(std::string const& text)
    {
        return ImGui::CalcTextSize(text.c_str()).x + 2 * ImGui::GetStyle().FramePadding.x;
    }

    template <typename T>
    void processToolButtons(bool, ColorMatrix<T>&, bool)
    {}

    void processToolButtons(bool disableDiagonal, ColorMatrix<bool>& matrix, bool localized)
    {
        auto forEachEditableEntry = [&](auto const& func) {
            for (int row = 0; row < MAX_COLORS; ++row) {
                for (int col = 0; col < MAX_COLORS; ++col) {
                    if (disableDiagonal && row == col) {
                        continue;
                    }
                    func(matrix[row][col]);
                }
            }
        };

        auto text = [&](std::string const& name, std::string const& key) { return localized ? tr("parameters.matrix." + key) : name; };
        if (localized) {
            ImGui::NewLine();
        } else {
            ImGui::SameLine();
        }
        auto toolButtonsWidth = buttonWidth(text("Clear", "clear")) + buttonWidth(text("Select all", "selectall")) + buttonWidth(text("Invert", "invert"))
            + buttonWidth(text("Randomize", "randomize")) + 4 * ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, ImGui::GetContentRegionAvail().x - toolButtonsWidth));

        if (AlienGui::DisplayButton("Clear", text("Clear", "clear"))) {
            forEachEditableEntry([](bool& entry) { entry = false; });
        }
        ImGui::SameLine();
        if (localized && ImGui::GetContentRegionAvail().x < buttonWidth(text("Select all", "selectall"))) {
            ImGui::NewLine();
        }
        if (AlienGui::DisplayButton("Select all", text("Select all", "selectall"))) {
            forEachEditableEntry([](bool& entry) { entry = true; });
        }
        ImGui::SameLine();
        if (localized && ImGui::GetContentRegionAvail().x < buttonWidth(text("Invert", "invert"))) {
            ImGui::NewLine();
        }
        if (AlienGui::DisplayButton("Invert", text("Invert", "invert"))) {
            forEachEditableEntry([](bool& entry) { entry = !entry; });
        }
        ImGui::SameLine();
        if (localized && ImGui::GetContentRegionAvail().x < buttonWidth(text("Randomize", "randomize"))) {
            ImGui::NewLine();
        }
        if (AlienGui::DisplayButton("Randomize", text("Randomize", "randomize"))) {
            forEachEditableEntry([](bool& entry) { entry = NumberGenerator::get().getRandomInt(2) == 0; });
        }
    }
}

template <typename T>
ColorMatrixDialog<T>::ColorMatrixDialog()
    : _modalWindow("", DialogSize)
{}

template <typename T>
void ColorMatrixDialog<T>::open(
    AlienGui::ExpandedColorMatrixParameters<T> const& parameters,
    T const (&value)[MAX_COLORS][MAX_COLORS],
    std::function<void(ColorMatrix<T> const&)> const& onAdoptCallback)
{
    _parameters = parameters;
    for (auto const& [matrixRow, valueRow] : std::views::zip(_matrix.values, value)) {
        std::ranges::copy(valueRow, std::begin(matrixRow));
    }
    _onAdoptCallback = onAdoptCallback;
    _modalWindow.setTitle(parameters._name);
    _modalWindow.open();
}

template <typename T>
void ColorMatrixDialog<T>::setPresentation(AlienGui::ExpandedColorMatrixParameters<T> const& parameters)
{
    if (_modalWindow.isOpen() && _parameters._name == parameters._name) {
        _parameters._label = parameters._label;
        _parameters._localizationKey = parameters._localizationKey;
        _parameters._rowLabel = parameters._rowLabel;
        _parameters._columnLabel = parameters._columnLabel;
    }
}

template <typename T>
void ColorMatrixDialog<T>::process()
{
    _modalWindow.process([this] { processContent(); });
}

template <typename T>
void ColorMatrixDialog<T>::processContent()
{
    auto localized = !_parameters._localizationKey.empty();
    if (localized) {
        // Paint the translated title without changing the legacy popup/window hash or child ID seed.
        auto window = ImGui::GetCurrentWindow();
        auto rect = window->TitleBarRect();
        auto padding = ImGui::GetStyle().FramePadding;
        rect.Min.x += padding.x;
        rect.Max.x -= scale(30.0f);
        ImGui::PushClipRect(rect.Min, rect.Max, false);
        window->DrawList->AddRectFilled(rect.Min, rect.Max, ImGui::GetColorU32(ImGuiCol_TitleBgActive));
        auto title = tr(_parameters._localizationKey + ".name");
        ImGui::RenderTextClipped({rect.Min.x, rect.Min.y + padding.y}, rect.Max, title.c_str(), nullptr, nullptr);
        ImGui::PopClipRect();
    }
    if (ImGui::BeginChild("##matrix", ImVec2(0, -scale(localized ? LocalizedButtonAreaHeight : ButtonAreaHeight)), false)) {
        auto maxCellSize = std::is_same_v<T, bool> ? MaxCellSizeForCheckbox : MaxCellSizeForSlider;
        AlienGui::ExpandedColorMatrix(
            AlienGui::ExpandedColorMatrixParameters<T>(_parameters).width(scaleInverse(ImGui::GetContentRegionAvail().x)).maxCellSize(maxCellSize),
            _matrix.values);
    }
    ImGui::EndChild();

    AlienGui::Separator();

    if (AlienGui::DisplayButton("Adopt", localized ? tr("parameters.matrix.adopt") : "Adopt")) {
        onAdopt();
        _modalWindow.close();
    }
    ImGui::SetItemDefaultFocus();

    ImGui::SameLine();
    if (AlienGui::DisplayButton("Cancel", localized ? tr("parameters.matrix.cancel") : "Cancel")) {
        _modalWindow.close();
    }

    processToolButtons(_parameters._disableDiagonal, _matrix, localized);
}

template <typename T>
void ColorMatrixDialog<T>::onAdopt()
{
    if (_onAdoptCallback) {
        _onAdoptCallback(_matrix);
    }
}

template class ColorMatrixDialog<bool>;
template class ColorMatrixDialog<int>;
template class ColorMatrixDialog<float>;

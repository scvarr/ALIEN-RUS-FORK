#include "GeneEditorWidget.h"

#include <imgui.h>

#include "AlienGui.h"
#include "GenomeEditorPresentation.h"
#include "GenomeTabEditData.h"
#include "GenomeTabLayoutData.h"
#include "StyleService.h"

namespace
{
    auto constexpr HeaderMinRightColumnWidth = 240.0f;  // Has to hold the longest label and the help marker
    auto constexpr HeaderMaxLeftColumnWidth = 200.0f;
    auto constexpr HeaderMinColumnWidth = 380.0f;
}


GeneEditorWidget _GeneEditorWidget::create(GenomeTabEditData const& editData, GenomeTabLayoutData const& layoutData)
{
    return GeneEditorWidget(new _GeneEditorWidget(editData, layoutData));
}

void _GeneEditorWidget::process()
{
    if (ImGui::BeginChild("GeneEditor", ImVec2(0, 0))) {
        if (_editData->selectedGeneIndex.has_value()) {
            ImGui::PushID(_editData->selectedGeneIndex.value());
            processHeaderData();
            ImGui::PopID();
        } else {
            processNoSelection();
        }
    }
    ImGui::EndChild();
}

_GeneEditorWidget::_GeneEditorWidget(GenomeTabEditData const& editData, GenomeTabLayoutData const& layoutData)
    : _editData(editData)
    , _layoutData(layoutData)
{}

void _GeneEditorWidget::processNoSelection()
{
    AlienGui::Group(AlienGui::GroupParameters().text(tr("genome.selection.empty")).highlighted(true));
    if (ImGui::BeginChild("overlay", ImVec2(0, 0), 0)) {
        auto startPos = ImGui::GetCursorScreenPos();
        auto size = ImGui::GetContentRegionAvail();
        AlienGui::DisabledField();
        auto text = tr("genome.selection.nogene");
        auto textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 textPos(startPos.x + size.x / 2 - textSize.x / 2, startPos.y + size.y / 2 - textSize.y / 2);
        ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
    }
    ImGui::EndChild();
}

void _GeneEditorWidget::processHeaderData()
{
    auto const& selectedGene = _editData->getSelectedGeneRef();
    auto title = GenomeEditorPresentation::format("genome.selection.gene", {{"index", std::to_string(_editData->selectedGeneIndex.value())}});
    if (!selectedGene._name.empty()) {
        title += " (" + selectedGene._name + ")";
    }
    AlienGui::Group(AlienGui::GroupParameters().text(title).highlighted(true));

    ImGui::SetNextWindowContentSize({ImGui::GetContentRegionAvail().x < scale(HeaderMinColumnWidth) ? scale(HeaderMinColumnWidth) : 0.0f, 0});
    if (ImGui::BeginChild("GeneHeader", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar)) {
        auto& gene = _editData->getSelectedGeneRef();

        _editData->updateGeometry();  // Do it every time in order to avoid check for changes

        AlienGui::DynamicTableLayout table(HeaderMinColumnWidth);
        if (table.begin()) {

            auto rightColumnWidth = std::max(HeaderMinRightColumnWidth, scaleInverse(ImGui::GetContentRegionAvail().x - scale(HeaderMaxLeftColumnWidth)));

            AlienGui::Group(AlienGui::GroupParameters().text(tr("genome.gene.base")));

            // Gene name
            AlienGui::InputText(
                AlienGui::InputTextParameters()
                    .name("Gene name")
                    .label(tr("genome.field.gene.name"))
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneName)),
                gene._name);

            // Shape
            if (AlienGui::Combo(
                    AlienGui::ComboParameters()
                        .name("Shape generator")
                        .label(tr("genome.field.gene.shape"))
                        .values(GenomeEditorPresentation::shapes())
                        .textWidth(rightColumnWidth)
                        .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneShape)),
                    gene._shape)) {
                {
                    ShapeGenerator shapeGenerator;
                    if (_editData->selectedGeneIndex.value() == 0) {
                        _editData->genome._frontAngle = shapeGenerator.getPreferredFrontAngle(gene._shape);
                    }
                }
            }

            // Connection distance
            AlienGui::InputFloat(
                AlienGui::InputFloatParameters()
                    .name("Connection distance")
                    .label(tr("genome.field.gene.distance"))
                    .format("%.2f")
                    .step(0.05f)
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneConnectionDistance)),
                gene._connectionDistance);

            // Stiffness
            AlienGui::InputFloat(
                AlienGui::InputFloatParameters()
                    .name("Stiffness")
                    .label(tr("genome.field.gene.stiffness"))
                    .format("%.2f")
                    .step(0.05f)
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneStiffness)),
                gene._stiffness);

            // Homogeneous cell type
            AlienGui::Checkbox(
                AlienGui::CheckboxParameters()
                    .name("Homogeneous cell type")
                    .label(tr("genome.field.gene.homogeneous"))
                    .textWidth(rightColumnWidth)
                    .tooltip(GenomeEditorPresentation::attributeDescription(EntityAttribute::GeneHomogeneousCellType)),
                &gene._homogeneousCellType);

            table.next();
            table.end();
        }
    }
    ImGui::EndChild();
}

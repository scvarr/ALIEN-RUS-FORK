"""Remove only L4-A3's additive shared presentation sites for older stage contracts.

Current behavior, IDs and fallback paths are tested by L4-A3 production ImGui tests.
"""
def legacy_node_widget_text(file, text):
    if file.endswith("AlienGui.h"):
        for struct in ("ComboColorParameters", "MultiCheckboxesParameters", "ColorCheckboxesParameters", "ButtonParameters"):
            text = text.replace(f'        MEMBER({struct}, std::optional<std::string>, label, std::nullopt);\n', '')
        text = text.replace('        MEMBER(ButtonParameters, std::optional<std::string>, buttonLabel, std::nullopt);\n', '')
        text = text.replace('        MEMBER(ComboParameters, std::vector<std::string>, displayValues, std::vector<std::string>());\n', '')
    if file.endswith("AlienGui.cpp"):
        text = text.replace('std::string text = parameters._label ? tr("common.infinity") : "Infinity";', 'std::string text = "Infinity";')
        for name in ("ComboColor", "MultiCheckboxes", "ColorCheckboxes", "Button"):
            import re
            match = re.search(r'bool AlienGui::' + name + r'\(' + name + r'Parameters[^\n]*\n\{.*?\n\}', text, re.S)
            if match:
                block = match.group().replace('.text(parameters._label.value_or(parameters._name))', '.text(parameters._name.c_str())')
                if name == 'ComboColor':
                    block = block.replace('.text(parameters._name.c_str())', '.text(parameters._name)')
                text = text[:match.start()] + block + text[match.end():]
        text = text.replace('auto result = parameters._buttonLabel ? DisplayButton(parameters._buttonText, *parameters._buttonLabel, {width, 0})\n                                          : ImGui::Button(parameters._buttonText.c_str(), {width, 0});', 'auto result = ImGui::Button(parameters._buttonText.c_str(), {width, 0});')
        text = text.replace('    if (parameters._displayValues.size() == parameters._values.size() && !parameters._displayValues.empty()) {\n        auto id = ImGui::GetCurrentWindow()->GetID(("##" + parameters._name).c_str());\n        auto preview = value >= 0 && value < parameters._displayValues.size() ? parameters._displayValues.at(value).c_str() : "";\n        if (ImGui::BeginCombo(("##" + parameters._name).c_str(), preview)) {\n            for (int index = 0; index < parameters._values.size(); ++index) {\n                ImGui::PushID(index);\n                auto selected = value == index;\n                auto label = parameters._displayValues.at(index) + "###" + parameters._values.at(index);\n                if (ImGui::Selectable(label.c_str(), selected)) {\n                    result |= value != index;\n                    value = index;\n                }\n                if (selected) {\n                    ImGui::SetItemDefaultFocus();\n                }\n                ImGui::PopID();\n            }\n            ImGui::EndCombo();\n        }\n        if (result) {\n            ImGui::MarkItemEdited(id);\n        }\n    } else {\n        result |= ImGui::Combo(("##" + parameters._name).c_str(), &value, vectorGetter, static_cast<void*>(&parameters._values), parameters._values.size());\n    }', '    result |= ImGui::Combo(("##" + parameters._name).c_str(), &value, vectorGetter, static_cast<void*>(&parameters._values), parameters._values.size());')
    return text

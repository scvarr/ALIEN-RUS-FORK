#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

// GUI-thread service. Translations are owning values: never retain a temporary's c_str().
class LocalizationService
{
public:
    LocalizationService();
    static LocalizationService& get();

    void initialize(std::filesystem::path const& directory);
    void load(std::filesystem::path const& directory);
    std::string getText(std::string const& key) const;
    std::string const& getLanguage() const;
    void requestLanguage(std::string const& language);
    void applyPendingLanguage();  // Call before ImGui::NewFrame, never during widget submission.
    std::vector<std::string> const& getDiagnostics() const;

private:
    using Catalog = std::map<std::string, std::string>;
    static Catalog parseCatalog(std::string const& data);
    void diagnose(std::string const& message) const;

    Catalog _english;
    Catalog _russian;
    std::string _language = "en";
    std::optional<std::string> _pendingLanguage;
    mutable std::vector<std::string> _diagnostics;
    mutable std::set<std::string> _unknownKeys;
};

inline std::string tr(std::string const& key)
{
    return LocalizationService::get().getText(key);
}

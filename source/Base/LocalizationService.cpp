#include "LocalizationService.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <regex>

#include <boost/json.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "DefaultLocale.h"
#include "GlobalSettings.h"
#include "LoggingService.h"

LocalizationService::LocalizationService()
    : _english(parseCatalog(Const::DEFAULT_LOCALE))
{}

LocalizationService& LocalizationService::get()
{
    static LocalizationService instance;
    return instance;
}

LocalizationService::Catalog LocalizationService::parseCatalog(std::string const& data)
{
    // Boost.JSON validates JSON types and strict UTF-8; property_tree retains duplicate members.
    auto value = boost::json::parse(data);
    if (!value.is_object() || value.as_object().empty()) {
        throw std::runtime_error("Expected a nonempty object of semantic keys and strings");
    }
    boost::property_tree::ptree members;
    std::istringstream stream(data);
    boost::property_tree::read_json(stream, members);
    if (members.size() != value.as_object().size()) {
        throw std::runtime_error("Duplicate catalog key");
    }
    static std::regex const keyPattern("[a-z][a-z0-9]*(\\.[a-z][a-z0-9]*)+");
    Catalog result;
    for (auto const& member : value.as_object()) {
        std::string key(member.key());
        if (!std::regex_match(key, keyPattern) || !member.value().is_string()) {
            throw std::runtime_error("Invalid key or non-string translation: " + key);
        }
        std::string text(member.value().as_string());
        if (text.empty() || text.find("##") != std::string::npos
            || std::ranges::any_of(text, [](unsigned char c) { return c < 32 && c != '\n' && c != '\t'; })) {
            throw std::runtime_error("Empty or unsafe translation: " + key);
        }
        result.emplace(key, text);
    }
    return result;
}

void LocalizationService::initialize(std::filesystem::path const& directory)
{
    load(directory);
    _language = GlobalSettings::get().getLanguage();
    _pendingLanguage.reset();
}

void LocalizationService::load(std::filesystem::path const& directory)
{
    _english = parseCatalog(Const::DEFAULT_LOCALE);
    _russian.clear();
    _diagnostics.clear();
    _unknownKeys.clear();
    for (auto const& language : {"en", "ru"}) {
        try {
            auto path = directory / (std::string(language) + ".json");
            std::ifstream stream(path, std::ios::binary);
            if (!stream) {
                throw std::runtime_error("Cannot read catalog");
            }
            auto catalog = parseCatalog(std::string(std::istreambuf_iterator<char>(stream), {}));
            if (std::string(language) == "en") {
                for (auto const& [key, text] : catalog) {
                    _english.insert_or_assign(key, text);
                }
            } else {
                for (auto const& [key, text] : catalog) {
                    if (_english.contains(key)) {
                        _russian.emplace(key, text);
                    } else {
                        diagnose("ru: Unknown catalog key: " + key);
                    }
                }
                for (auto const& [key, text] : _english) {
                    if (!_russian.contains(key)) {
                        diagnose("ru: Missing translation: " + key);
                    }
                }
            }
        } catch (std::exception const& exception) {
            diagnose(std::string(language) + ": " + exception.what());
        }
    }
}

std::string LocalizationService::getText(std::string const& key) const
{
    if (_language == "ru") {
        if (auto it = _russian.find(key); it != _russian.end()) {
            return it->second;
        }
    }
    if (auto it = _english.find(key); it != _english.end()) {
        return it->second;
    }
    if (_unknownKeys.insert(key).second) {
        diagnose("Unknown translation key: " + key);
    }
    return key;
}

std::string const& LocalizationService::getLanguage() const
{
    return _language;
}

void LocalizationService::requestLanguage(std::string const& language)
{
    _pendingLanguage = language == "ru" ? "ru" : "en";
}

void LocalizationService::applyPendingLanguage()
{
    if (_pendingLanguage) {
        _language = *_pendingLanguage;
        _pendingLanguage.reset();
        GlobalSettings::get().setLanguage(_language);
    }
}

std::vector<std::string> const& LocalizationService::getDiagnostics() const
{
    return _diagnostics;
}

void LocalizationService::diagnose(std::string const& message) const
{
    _diagnostics.emplace_back(message);
    log(Priority::Important, "Localization: " + message);
}

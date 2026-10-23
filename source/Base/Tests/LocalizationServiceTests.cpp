#include <gtest/gtest.h>

#include <chrono>
#include <fstream>
#include <regex>

#include <boost/json.hpp>

#include <Base/GlobalSettings.h>
#include <Base/LocalizationService.h>
#include "SettingsTestScope.h"

namespace
{
    std::filesystem::path const LOCALES = std::filesystem::path(ALIEN_SOURCE_DIR) / "resources/locales";

    std::string read(std::filesystem::path const& path)
    {
        std::ifstream stream(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(stream), {}};
    }
}

class LocalizationServiceTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        _originalLanguage = GlobalSettings::get().getValue("settings.language", std::string("en"));
        _directory =
            std::filesystem::temp_directory_path() / ("alien-localization-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directory(_directory);
    }

    void TearDown() override
    {
        GlobalSettings::get().setValue("settings.language", _originalLanguage);
        std::filesystem::remove_all(_directory);
    }

    void write(std::string const& language, std::string const& data)
    {
        std::ofstream stream(_directory / (language + ".json"), std::ios::binary);
        stream << data;
    }

    void select(std::string const& language)
    {
        _service.requestLanguage(language);
        _service.applyPendingLanguage();
    }

    LocalizationService _service;
    SettingsTestScope _settingsScope;
    std::filesystem::path _directory;
    std::string _originalLanguage;
};

TEST_F(LocalizationServiceTests, loadEnglishAndRussian)
{
    _service.load(LOCALES);
    EXPECT_TRUE(_service.getDiagnostics().empty());
    EXPECT_EQ("en", _service.getLanguage());
    EXPECT_EQ("Display settings", _service.getText("display.title"));
    select("ru");
    EXPECT_EQ("Настройки экрана", _service.getText("display.title"));
}

TEST_F(LocalizationServiceTests, missingCatalogsKeepEmbeddedEnglish)
{
    _service.load(_directory / "missing");
    EXPECT_EQ(2, _service.getDiagnostics().size());
    select("ru");
    EXPECT_EQ("Display settings", _service.getText("display.title"));
}

TEST_F(LocalizationServiceTests, malformedCatalogsAreAtomic)
{
    write("en", R"({"display.title":"Changed",)");
    write("ru", R"({"display.title":"Перевод","display.fps":17})");
    EXPECT_NO_THROW(_service.load(_directory));
    EXPECT_EQ(2, _service.getDiagnostics().size());
    select("ru");
    EXPECT_EQ("Display settings", _service.getText("display.title"));
}

TEST_F(LocalizationServiceTests, missingTranslationFallsBackPerKey)
{
    write("en", R"({"display.title":"English title"})");
    write("ru", R"({"display.fps":"Частота кадров"})");
    _service.load(_directory);
    select("ru");
    EXPECT_EQ("English title", _service.getText("display.title"));
    EXPECT_EQ("Частота кадров", _service.getText("display.fps"));
}

TEST_F(LocalizationServiceTests, unknownKeysAreDiagnosedOnce)
{
    _service.load(LOCALES);
    EXPECT_EQ("unknown.key", _service.getText("unknown.key"));
    EXPECT_EQ("unknown.key", _service.getText("unknown.key"));
    ASSERT_EQ(1, _service.getDiagnostics().size());
    write("en", read(LOCALES / "en.json"));
    write("ru", R"({"unknown.key":"Unknown"})");
    _service.load(_directory);
    select("ru");
    EXPECT_EQ("unknown.key", _service.getText("unknown.key"));
}

TEST_F(LocalizationServiceTests, extensionDoesNotRequireServiceChanges)
{
    write("en", R"({"future.caption":"Future"})");
    write("ru", R"({"future.caption":"Будущее"})");
    _service.load(_directory);
    select("ru");
    EXPECT_EQ("Будущее", _service.getText("future.caption"));
}

TEST_F(LocalizationServiceTests, rejectsDuplicateAndEscapedDuplicateKeys)
{
    for (auto const& data : {R"({"display.title":"A","display.title":"B"})", R"({"display.title":"A","display.\u0074itle":"B"})"}) {
        write("ru", data);
        _service.load(_directory);
        select("ru");
        EXPECT_EQ("Display settings", _service.getText("display.title"));
        EXPECT_NE(std::string::npos, _service.getDiagnostics().back().find("Duplicate"));
    }
}

TEST_F(LocalizationServiceTests, rejectsWrongStructureAndUnsafeText)
{
    for (auto const& data :
         {"[]",
          "{}",
          R"({"display.title":null})",
          R"({"display.title":{}})",
          R"({"display.title":true})",
          R"({"display.title":""})",
          R"({"Bad key":"Text"})",
          R"({"display.title":"Text###id"})",
          R"({"display.title":"\u0000"})"}) {
        SCOPED_TRACE(data);
        write("ru", data);
        _service.load(_directory);
        select("ru");
        EXPECT_EQ("Display settings", _service.getText("display.title"));
        EXPECT_EQ(2, _service.getDiagnostics().size());
    }
}

TEST_F(LocalizationServiceTests, rejectsInvalidUtf8AndUnpairedSurrogates)
{
    for (auto const& text :
         {std::string("\xc0\xaf"),
          std::string("\x80"),
          std::string("\xe2\x82"),
          std::string("\xed\xa0\x80"),
          std::string("\xf4\x90\x80\x80"),
          std::string("\\ud800")}) {
        write("ru", "{\"display.title\":\"" + text + "\"}");
        _service.load(_directory);
        select("ru");
        EXPECT_EQ("Display settings", _service.getText("display.title"));
        EXPECT_EQ(2, _service.getDiagnostics().size());
    }
}

TEST_F(LocalizationServiceTests, acceptsRequiredUnicodeAndEscapes)
{
    write("ru", R"({"display.title":"Ёё № — «Русский» \u0410"})");
    _service.load(_directory);
    select("ru");
    EXPECT_EQ("Ёё № — «Русский» А", _service.getText("display.title"));
}

TEST_F(LocalizationServiceTests, languageChangeIsDeferredAndStringsRemainOwned)
{
    _service.load(LOCALES);
    auto saved = _service.getText("display.title");
    _service.requestLanguage("ru");
    EXPECT_EQ("en", _service.getLanguage());
    _service.applyPendingLanguage();
    EXPECT_EQ("ru", _service.getLanguage());
    EXPECT_EQ("Display settings", saved);
    select("en");
    EXPECT_EQ(saved, _service.getText("display.title"));
}

TEST_F(LocalizationServiceTests, persistsAndRestoresLanguage)
{
    for (auto const& language : {"en", "ru"}) {
        select(language);
        LocalizationService restored;
        restored.initialize(LOCALES);
        EXPECT_EQ(language, restored.getLanguage());
        EXPECT_EQ(language, GlobalSettings::get().getValue("settings.language", std::string()));
    }
}

TEST_F(LocalizationServiceTests, invalidStoredOrRequestedLanguageResolvesToEnglish)
{
    GlobalSettings::get().setValue("settings.language", std::string("unsupported"));
    _service.initialize(LOCALES);
    EXPECT_EQ("en", _service.getLanguage());
    select("RU");
    EXPECT_EQ("en", _service.getLanguage());
    EXPECT_EQ("en", GlobalSettings::get().getLanguage());
}

TEST_F(LocalizationServiceTests, settingsRoundTripUtf8WithoutTruncation)
{
    auto& settings = GlobalSettings::get();
    std::string const text = "Ёё № — «Русский»";
    settings.setValue("tests.localization.utf8", text);
    EXPECT_EQ(text, settings.getValue("tests.localization.utf8", std::string()));
    settings.setValue("tests.localization.utf8", std::string());
    EXPECT_EQ("", settings.getValue("tests.localization.utf8", std::string("fallback")));
}

TEST_F(LocalizationServiceTests, shippedSliceIsCompleteAndValid)
{
    auto english = boost::json::parse(read(LOCALES / "en.json")).as_object();
    auto russian = boost::json::parse(read(LOCALES / "ru.json")).as_object();
    ASSERT_EQ(english.size(), russian.size());
    ASSERT_EQ(1044, english.size());
    _service.load(LOCALES);  // Also validates UTF-8, duplicates, key syntax and types in both shipped files.
    ASSERT_TRUE(_service.getDiagnostics().empty());
    for (auto const& member : english) {
        ASSERT_TRUE(russian.contains(member.key())) << member.key();
        EXPECT_FALSE(russian.at(member.key()).as_string().empty());
        if (!std::string(member.key()).starts_with("language.")) {
            EXPECT_NE(member.value(), russian.at(member.key())) << member.key();
        }
    }
    std::regex const reference(R"KEY(tr\("([a-z.]+)"\))KEY");
    for (auto const& file :
         {"DisplaySettingsDialog.cpp",
          "MainLoopController.cpp",
          "LocalizationGui.cpp",
          "GenericMessageDialog.cpp",
          "AlienGui.cpp",
          "NewSimulationDialog.cpp",
          "ExitDialog.cpp",
          "SavePictureDialog.cpp",
          "FileTransferController.cpp",
          "SimulationParametersMainWindow.cpp",
          "SimulationParametersGuiText.h",
          "SimulationParametersBaseWidget.cpp",
          "GenomeEditorWindow.cpp",
          "GenomeEditorWidget.cpp",
          "GeneEditorWidget.cpp",
          "GenomeTabWidget.cpp",
          "GenomeEditorPresentation.cpp",
          "NeuralNetEditorWidget.cpp",
          "NeuralNetEditorPresentation.h",
          "MutationRatesWidget.cpp",
          "MutationRatesDialog.cpp",
          "MutationRatesPresentation.h",
          "CellFunctionChannels.cpp",
          "CreaturePreviewWidget.cpp",
          "PreviewWidget.cpp",
          "PreviewSettingsDialog.cpp",
          "SignalsBufferDialog.cpp",
          "InspectionPresentation.h",
          "InspectionWindow.cpp",
          "InspectionController.cpp",
          "GenomeIssueDescription.cpp",
          "GeneGraphWidget.cpp",
          "PreviewDescView.cpp",
          "ChangeColorDialog.cpp",
          "UploadSimulationDialog.cpp"}) {
        auto source = read(std::filesystem::path(ALIEN_SOURCE_DIR) / "source/Gui/Impl" / file);
        for (std::sregex_iterator it(source.begin(), source.end(), reference), end; it != end; ++it) {
            EXPECT_TRUE(english.contains((*it)[1].str())) << file << ": " << (*it)[1].str();
        }
    }
}

TEST_F(LocalizationServiceTests, l2aEmbeddedEnglishSurvivesMissingCatalogs)
{
    auto english = boost::json::parse(read(LOCALES / "en.json")).as_object();
    _service.load(_directory / "missing");
    select("ru");
    for (auto const& member : english) {
        EXPECT_EQ(std::string(member.value().as_string()), _service.getText(std::string(member.key())));
    }
}

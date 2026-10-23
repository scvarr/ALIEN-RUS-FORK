"""Controls/statistics presentation contracts; runtime ImGui/ImPlot tests cover state."""
import json
from pathlib import Path
import re
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[4]
GUI = ROOT / 'source/Gui/Impl'
BASE = '33bc2be134e6d1f2d3f075a8c325ffbc915da4db'


def read(path):
    return (ROOT / path).read_text(encoding='utf8')


def original(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT).decode('utf8').replace('\r\n', '\n')


def function(text, name):
    start = re.search(r'^.*::' + re.escape(name) + r'\(', text, re.M).start()
    return text[start:start + re.search(r'^}\n', text[start:], re.M).end()]


class L5cContracts(unittest.TestCase):
    def test_catalog_preservation_complete_references_and_signatures(self):
        def unique(pairs):
            result = dict(pairs)
            self.assertEqual(len(pairs), len(result))
            return result
        catalogs = {lang: json.loads(read(f'resources/locales/{lang}.json'), object_pairs_hook=unique) for lang in ['en', 'ru']}
        self.assertEqual(catalogs['en'].keys(), catalogs['ru'].keys())
        consumers = read('source/Gui/Impl/ControlStatisticsPresentation.h') + read('source/Gui/Impl/EvolutionDashboardWindow.cpp')
        for lang, values in catalogs.items():
            old = json.loads(original(f'resources/locales/{lang}.json'))
            self.assertEqual(old, {k: values[k] for k in old})
            added = {key for key in values.keys() - old.keys() if not key.startswith(('localui.', 'browser.', 'account.'))}
            self.assertEqual(56, len(added))
            for key in added:
                self.assertTrue(key.startswith('simulationui.'))
                self.assertRegex(key, r'^[a-z][a-z0-9]*(\.[a-z][a-z0-9]*)+$')
                self.assertIn('"' + key + '"', consumers)
                self.assertNotIn('##', values[key])
                self.assertEqual(sorted(re.findall(r'\{\w+}', catalogs['en'][key])), sorted(re.findall(r'\{\w+}', catalogs['ru'][key])))

    def test_statistics_history_rates_and_numeric_formats_are_unchanged(self):
        path = 'source/Gui/Impl/EvolutionDashboardWindow.cpp'
        before, after = original(path), read(path)
        # All history/cache/selection derivation, aggregation and rate math remains byte-identical.
        start = 'void EvolutionDashboardWindow::initIntern()'
        end = 'void EvolutionDashboardWindow::processHeader()'
        self.assertEqual(before[before.index(start):before.index(end)], after[after.index(start):after.index(end)])
        for name in ['computePlotTimeRange', 'validateAndCorrect']:
            self.assertEqual(function(before, name), function(after, name))
        for pattern in [r'MetricDef const Metrics[^;]+;', r'auto constexpr RateAveragingTimesteps[^;]+;',
                        r'"%[^"]*"', r'ImPlot::SetNextAxesLimits\([^;]+;']:
            self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), pattern)
        for name in ['TemporalControlWindow', 'SpatialControlWindow']:
            path = f'source/Gui/Impl/{name}.cpp'
            self.assertEqual(re.findall(r'\.format\(([^)]+)\)', original(path)), re.findall(r'\.format\(([^)]+)\)', read(path)))

    def test_plot_series_cards_and_widget_scopes_keep_canonical_identity(self):
        for name in ['TemporalControlWindow', 'SpatialControlWindow', 'EvolutionDashboardWindow']:
            path = f'source/Gui/Impl/{name}.cpp'
            before, after = original(path), read(path)
            for pattern in [r'ImGui::PushID\(([^;]+)\);', r'ImGui::BeginChild\(\s*"([^"]+)"',
                            r'ImGui::BeginTable\("([^"]+)"', r'ImPlot::(?:BeginPlot|PlotLine|PlotShaded)\(\s*"([^"]+)"',
                            r'\.id\("([^"]+)"\)']:
                self.assertEqual(re.findall(pattern, before), re.findall(pattern, after), name + pattern)
        dashboard = read('source/Gui/Impl/EvolutionDashboardWindow.cpp')
        self.assertIn('ImGui::BeginChild(("##card" + label).c_str()', dashboard)
        self.assertIn('AlienGui::Text(text(label))', dashboard)
        self.assertIn('ImGui::PushTextWrapPos(0)', dashboard)
        self.assertIn('ImPlot::PlotText(text("No data").c_str()', dashboard)
        self.assertIn('.values(modeValues)', dashboard)
        self.assertIn('.displayValues(modeLabels)', dashboard)

    def test_location_widgets_names_and_session_lifetime_are_unchanged(self):
        for name in ['LocationController.cpp', 'LocationWindow.cpp', 'SimulationParametersBaseWidget.cpp',
                     'SimulationParametersLayerWidget.cpp', 'SimulationParametersSourceWidget.cpp']:
            path = f'source/Gui/Impl/{name}'
            self.assertEqual(original(path), read(path), path)

    def test_no_engine_data_csv_serialization_or_unrelated_stage_changes(self):
        allowed = {'resources/locales/en.json', 'resources/locales/ru.json', 'docs/localization-l5c-validation.md',
                   'source/Base/Tests/LocalizationServiceTests.cpp',
                   'source/Gui/Impl/ControlStatisticsPresentation.h', 'source/Gui/Impl/Tests/LocalizationGuiTests.cpp'}
        allowed.update(f'source/Gui/Impl/{component}.{ext}' for component in
                       ['TemporalControlWindow', 'SpatialControlWindow', 'EvolutionDashboardWindow'] for ext in ['cpp', 'h'])
        allowed.update(f'source/Gui/Impl/Tests/check_localization_{stage}.py' for stage in ['l4a1', 'l4a2', 'l4a3', 'l4a4', 'l4a5', 'l5a', 'l5b', 'l5c'])
        allowed.update(f'source/Gui/Impl/Tests/check_localization_{stage}.py' for stage in
                       ['l2a', 'l2b1', 'l2b2', 'l2b3a', 'l2b3b', 'l2b3c', 'l2b4a', 'l2b4b', 'l2b4c', 'l2b4d', 'l2b4e'])
        changed = subprocess.check_output(['git', 'diff', '--name-only', BASE, '4d85183ce261a8648baae90ac2aa6cd6f75843b3'], cwd=ROOT).decode().splitlines()
        self.assertLessEqual(set(changed), allowed)
        self.assertEqual(original('docs/localization-terminology.md'), read('docs/localization-terminology.md'))


if __name__ == '__main__':
    unittest.main()

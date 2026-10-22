"""Windows Ninja header-dependency regression test; run from an MSVC environment."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class WindowsNinjaDependencies(unittest.TestCase):
    def test_probe_preserves_raw_bytes_and_rejects_unverified_caches(self):
        module = (ROOT / 'cmake/MsvcShowIncludes.cmake').read_text()
        self.assertIn('ENCODING NONE', module)
        self.assertIn('set(CMAKE_CL_SHOWINCLUDES_PREFIX "${_includesPrefix}")', module)
        self.assertIn('NOT DEFINED ALIEN_MSVC_INCLUDE_PREFIX', module)
        self.assertIn('MSVC header dependency encoding changed', module)
        self.assertIn('include(cmake/MsvcShowIncludes.cmake)', (ROOT / 'CMakeLists.txt').read_text())

    @unittest.skipUnless(os.name == 'nt' and all(shutil.which(t) for t in ('cl', 'cmake', 'ninja')), 'Needs an MSVC developer environment')
    def test_header_edit_rebuilds_consumer_and_updates_program(self):
        environment = os.environ.copy()
        with tempfile.TemporaryDirectory(prefix='alien-ninja-deps-') as directory:
            project = Path(directory)
            build = project / 'build'
            module = (ROOT / 'cmake/MsvcShowIncludes.cmake').as_posix()
            (project / 'CMakeLists.txt').write_text(
                'cmake_minimum_required(VERSION 3.31)\n'
                'project(HeaderDependencyProbe LANGUAGES CXX)\n'
                f'include("{module}")\n'
                'add_executable(probe main.cpp)\n')
            (project / 'main.cpp').write_text('#include "value.h"\nint main() { return VALUE; }\n')
            header = project / 'value.h'
            header.write_text('#define VALUE 1\n')

            def run(*arguments):
                result = subprocess.run(arguments, env=environment, capture_output=True, text=True, errors='replace')
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                return result.stdout

            run('cmake', '-S', str(project), '-B', str(build), '-G', 'Ninja Multi-Config')
            rules = (build / 'CMakeFiles/rules.ninja').read_text()
            self.assertRegex(rules, r'msvc_deps_prefix = .+:')
            run('cmake', '--build', str(build), '--config', 'Release')
            executable = build / 'Release/probe.exe'
            self.assertEqual(1, subprocess.run([str(executable)]).returncode)
            dependencies = run('ninja', '-C', str(build), '-f', 'build-Release.ninja', '-t', 'deps')
            self.assertIn('value.h', dependencies)
            header.write_text('#define VALUE 2\n')
            run('cmake', '--build', str(build), '--config', 'Release')
            self.assertEqual(2, subprocess.run([str(executable)]).returncode)
            incompatible = subprocess.run(['cmake', '-S', str(project), '-B', str(build), '-DALIEN_MSVC_INCLUDE_PREFIX=broken'],
                                          env=environment, capture_output=True, text=True, errors='replace')
            self.assertNotEqual(0, incompatible.returncode)
            self.assertIn('MSVC header dependency encoding changed', incompatible.stdout + incompatible.stderr)


if __name__ == '__main__':
    unittest.main(verbosity=2)

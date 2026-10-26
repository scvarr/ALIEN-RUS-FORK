// English read_documentation responses at 8d20e14c593efbf73a8a80d021658d870f3343ac.
#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace DocumentationBaseline
{
    inline std::string const CONTENTS =
        R"baseline({"chapters":[{"chapter":"getting-started.md","title":"Getting started","part":"Start here"},{"chapter":"user-interface.md","title":"The user interface","part":"Start here"},{"chapter":"ai-agents.md","title":"AI agents","part":"Start here"},{"chapter":"sandbox.md","title":"Sandbox and editing","part":"Explore"},{"chapter":"how-life-works.md","title":"How life works","part":"Explore"},{"chapter":"first-creature.md","title":"Your first creature","part":"Explore"},{"chapter":"evolution.md","title":"Evolution experiments","part":"Explore"},{"chapter":"generative-art.md","title":"Generative art","part":"Explore"},{"chapter":"cell-types.md","title":"Cell types","part":"Reference"},{"chapter":"genomes.md","title":"Genomes and construction","part":"Reference"},{"chapter":"neural-networks.md","title":"Neural networks and signals","part":"Reference"},{"chapter":"energy.md","title":"Energy","part":"Reference"},{"chapter":"simulation-parameters.md","title":"Simulation parameters","part":"Reference"},{"chapter":"files.md","title":"Files and command line","part":"Reference"},{"chapter":"keyboard-and-mouse.md","title":"Keyboard and mouse","part":"Reference"},{"chapter":"troubleshooting.md","title":"Troubleshooting","part":"Reference"},{"chapter":"glossary.md","title":"Glossary","part":"Reference"}]})baseline";
    inline std::string const DESCRIPTION =
        R"baseline(Reads the documentation of ALIEN. It explains in detail how creatures live, the cell types, genomes, neural networks, energy and all simulation parameters. Without a chapter, the table of contents is returned. With a chapter, its text is returned in Markdown. A link to another chapter such as 'energy.md#the-external-energy-pool' refers to the chapter 'energy.md'.)baseline";
    inline std::map<std::string, uint64_t> const CHAPTER_HASHES = {
        {"getting-started.md", 18419976498006941345ull},
        {"user-interface.md", 16611550915381612591ull},
        {"ai-agents.md", 11949897441349235780ull},
        {"sandbox.md", 7469694447753530333ull},
        {"how-life-works.md", 13019815602489123620ull},
        {"first-creature.md", 5194218796345717322ull},
        {"evolution.md", 4427979830467269686ull},
        {"generative-art.md", 3065674632112147089ull},
        {"cell-types.md", 10151969220457909038ull},
        {"genomes.md", 6424990543013939872ull},
        {"neural-networks.md", 7305862209073701259ull},
        {"energy.md", 3656815697232271663ull},
        {"simulation-parameters.md", 15596951977634798708ull},
        {"files.md", 2151306831092277204ull},
        {"keyboard-and-mouse.md", 14308030707980630406ull},
        {"troubleshooting.md", 7032035892737291016ull},
        {"glossary.md", 17105696022726548194ull},
    };
}

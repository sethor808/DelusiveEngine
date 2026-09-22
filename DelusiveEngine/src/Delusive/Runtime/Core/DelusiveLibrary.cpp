#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>

namespace {
    void EraseFromSlice(std::vector<const DelusiveParser::DataBlock*>& slice,
        const DelusiveParser::DataBlock* block)
    {
        slice.erase(std::remove(slice.begin(), slice.end(), block), slice.end());
    }
}

#pragma region Load

void DelusiveLibrary::LoadAll() {
    //Animations still use their own line format and canvases still share one legacy file,
    //so neither is block-parseable yet.
    LoadDirectory(SCENE_PATH, SCENE_EXT);
    LoadDirectory(AGENT_PATH, AGENT_EXT);
}

void DelusiveLibrary::LoadDirectory(const std::string& path, const std::string& extension) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        std::cerr << "[Library] Missing directory: " << path << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != extension) continue;

        LoadFile(entry.path().string());
    }
}

void DelusiveLibrary::LoadFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "[Library] Cannot open " << path << std::endl;
        return;
    }

    for (auto& block : DelusiveParser::ParseFile(in)) {
        //Closing tags from pre-flat files parse as their own block
        if (!block.category.empty() && block.category.front() == '/') continue;

        if (!block.id.IsValid()) {
            std::cerr << "[Library] Block without UUID in " << path
                << " (" << block.category << " " << block.type << ")" << std::endl;
            continue;
        }

        Add(std::move(block), path);
    }
}

void DelusiveLibrary::Clear() {
    entries.clear();
    byCategory.clear();
    byFile.clear();
}

#pragma endregion

#pragma region Lookup

bool DelusiveLibrary::Has(const UUID& id) const {
    return entries.contains(id);
}

const DelusiveParser::DataBlock* DelusiveLibrary::Find(const UUID& id) const {
    auto entry = entries.find(id);

    if (entry == entries.end()) return nullptr;

    return &entry->second.block;
}

const std::vector<const DelusiveParser::DataBlock*>& DelusiveLibrary::List(const std::string& category) const {
    static const std::vector<const DelusiveParser::DataBlock*> empty;

    auto categoryList = byCategory.find(category);

    if (categoryList == byCategory.end()) return empty;

    return categoryList->second;
}

std::vector<UUID> DelusiveLibrary::FindReferences(const UUID& id) const {
    std::vector<UUID> references;
    if (!id.IsValid()) return references;

    const std::string target = id.ToString();

    for (const auto& [entryID, entry] : entries) {
        if (entryID == id) continue;

        for (const auto& [key, value] : entry.block.properties) {
            //Substring so UUIDs inside space separated ownership lists still match
            if (value.find(target) != std::string::npos) {
                references.push_back(entryID);
                break;
            }
        }
    }

    return references;
}

std::string DelusiveLibrary::GetSourceFile(const UUID& id) const {
    auto entry = entries.find(id);

    if (entry == entries.end()) return "";

    return entry->second.sourceFile;
}

#pragma endregion

#pragma region Editing

void DelusiveLibrary::Add(DelusiveParser::DataBlock block, const std::string& sourceFile) {
    if (!block.id.IsValid()) return;

    UUID id = block.id;
    auto [entry, inserted] = entries.try_emplace(id, Entry{ std::move(block), sourceFile });

    if (!inserted) {
        std::cerr << "[Library] Duplicate UUID " << id.ToString()
            << " in " << sourceFile
            << " and " << entry->second.sourceFile << std::endl;
        return;
    }

    const DelusiveParser::DataBlock* stored = &entry->second.block;
    byCategory[stored->category].push_back(stored);
    byFile[sourceFile].push_back(stored);
}

void DelusiveLibrary::Update(const UUID& id, DelusiveParser::DataBlock block) {
    auto entry = entries.find(id);
    if (entry == entries.end()) return;

    const std::string previousCategory = entry->second.block.category;

    //Assigning in place keeps the slice pointers valid
    entry->second.block = std::move(block);
    entry->second.block.id = id;

    const DelusiveParser::DataBlock* stored = &entry->second.block;

    if (stored->category != previousCategory) {
        EraseFromSlice(byCategory[previousCategory], stored);
        byCategory[stored->category].push_back(stored);
    }
}

void DelusiveLibrary::Remove(const UUID& id) {
    auto entry = entries.find(id);
    if (entry == entries.end()) return;

    const DelusiveParser::DataBlock* stored = &entry->second.block;

    EraseFromSlice(byCategory[stored->category], stored);
    EraseFromSlice(byFile[entry->second.sourceFile], stored);

    entries.erase(entry);
}

#pragma endregion

#pragma region Save

bool DelusiveLibrary::SaveFile(const std::string& path) const {
    auto fileList = byFile.find(path);
    if (fileList == byFile.end()) return false;

    std::filesystem::path parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
    }

    //Write to a temp file first so a crash mid write cannot corrupt the original
    const std::string tempPath = path + ".tmp";
    {
        std::ofstream out(tempPath);
        if (!out) {
            std::cerr << "[Library] Cannot write " << tempPath << std::endl;
            return false;
        }

        out << std::setprecision(std::numeric_limits<float>::max_digits10);

        for (const DelusiveParser::DataBlock* block : fileList->second) {
            DelusiveParser::WriteBlock(out, *block);
            out << "\n";
        }

        if (!out) return false;
    }

    std::error_code ec;
    std::filesystem::rename(tempPath, path, ec);

    if (ec) {
        std::cerr << "[Library] Cannot replace " << path << ": " << ec.message() << std::endl;
        return false;
    }

    return true;
}

bool DelusiveLibrary::SaveAll() const {
    bool success = true;

    for (const auto& [path, blocks] : byFile) {
        if (!SaveFile(path)) success = false;
    }

    return success;
}

#pragma endregion

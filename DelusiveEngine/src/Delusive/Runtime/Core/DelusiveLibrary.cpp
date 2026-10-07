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

    //One spelling per file so "../assets/scenes/a.scene" and "../assets//scenes/a.scene"
    //cannot be indexed as two different files
    std::string FileKey(const std::string& path) {
        return std::filesystem::path(path).lexically_normal().generic_string();
    }

    void ReplaceAll(std::string& text, const std::string& from, const std::string& to) {
        for (size_t at = text.find(from); at != std::string::npos; at = text.find(from, at + to.size())) {
            text.replace(at, from.size(), to);
        }
    }

    bool WriteBlocks(const std::string& path, const std::vector<const DelusiveParser::DataBlock*>& blocks) {
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

            for (const DelusiveParser::DataBlock* block : blocks) {
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
}

#pragma region Load

void DelusiveLibrary::LoadAll() {
    LoadDirectory(SCENE_PATH, SCENE_EXT);
    LoadDirectory(AGENT_PATH, AGENT_EXT);
    LoadDirectory(ANIM_PATH, ANIM_EXT);
    LoadDirectory(CANVAS_PATH, CANVAS_EXT);
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

bool DelusiveLibrary::LoadFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "[Library] Cannot open " << path << std::endl;
        return false;
    }

    std::vector<DelusiveParser::DataBlock> blocks;

    for (auto& block : DelusiveParser::ParseFile(in)) {
        //Closing tags from pre-flat files parse as their own block
        if (!block.category.empty() && block.category.front() == '/') continue;

        if (!block.id.IsValid()) {
            std::cerr << "[Library] Block without UUID in " << path
                << " (" << block.category << " " << block.type << ")" << std::endl;
            continue;
        }

        blocks.push_back(std::move(block));
    }

    ReplaceFile(path, std::move(blocks));
    return true;
}

void DelusiveLibrary::ReplaceFile(const std::string& path, std::vector<DelusiveParser::DataBlock> blocks) {
    RemoveFile(path);

    for (auto& block : blocks) {
        Add(std::move(block), path);
    }
}

void DelusiveLibrary::RemoveFile(const std::string& path) {
    auto fileList = byFile.find(FileKey(path));
    if (fileList == byFile.end()) return;

    for (const DelusiveParser::DataBlock* stored : fileList->second) {
        UUID id = stored->id;
        EraseFromSlice(byCategory[stored->category], stored);
        entries.erase(id);
    }

    byFile.erase(fileList);
}

void DelusiveLibrary::Clear() {
    entries.clear();
    byCategory.clear();
    byFile.clear();
}

#pragma endregion

#pragma region Lookup

bool DelusiveLibrary::Has(const UUID& id) const {
    return Find(id) != nullptr;
}

const DelusiveParser::DataBlock* DelusiveLibrary::Find(const UUID& id) const {
    for (auto overlay = overlays.rbegin(); overlay != overlays.rend(); ++overlay) {
        auto block = (*overlay)->find(id);
        if (block != (*overlay)->end()) return block->second;
    }

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

const std::vector<const DelusiveParser::DataBlock*>& DelusiveLibrary::ListFile(const std::string& path) const {
    static const std::vector<const DelusiveParser::DataBlock*> empty;

    auto fileList = byFile.find(FileKey(path));

    if (fileList == byFile.end()) return empty;

    return fileList->second;
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

DelusiveLibrary::Overlay::Overlay(DelusiveLibrary& library, const std::vector<DelusiveParser::DataBlock>& source)
    : library(library)
{
    for (const auto& block : source) {
        if (block.id.IsValid()) blocks[block.id] = &block;
    }
    library.overlays.push_back(&blocks);
}

DelusiveLibrary::Overlay::~Overlay() {
    //Overlays are scoped, so this one is always the innermost
    library.overlays.pop_back();
}

#pragma region Editing

void DelusiveLibrary::Add(DelusiveParser::DataBlock block, const std::string& sourceFile) {
    if (!block.id.IsValid()) return;

    UUID id = block.id;
    const std::string file = FileKey(sourceFile);
    auto [entry, inserted] = entries.try_emplace(id, Entry{ std::move(block), file });

    if (!inserted) {
        //Usually a file copied outside the editor - the copy keeps its source's ids
        std::cerr << "[Library] Duplicate UUID " << id.ToString()
            << " in " << file
            << " (already indexed from " << entry->second.sourceFile << ")" << std::endl;
        return;
    }

    const DelusiveParser::DataBlock* stored = &entry->second.block;
    byCategory[stored->category].push_back(stored);
    byFile[file].push_back(stored);
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

void DelusiveLibrary::ApplyRemap(std::vector<DelusiveParser::DataBlock>& blocks, const IDRemap& remap) {
    if (remap.empty()) return;

    for (auto& block : blocks) {
        auto fresh = remap.find(block.id);
        if (fresh != remap.end()) block.id = fresh->second;

        for (auto& [key, value] : block.properties) {
            for (const auto& [from, to] : remap) {
                ReplaceAll(value, from.ToString(), to.ToString());
            }
        }
    }
}

bool DelusiveLibrary::WriteFile(const std::string& path, std::vector<DelusiveParser::DataBlock> blocks,
    IDRemap* remapped)
{
    const std::string file = FileKey(path);
    IDRemap remap;

    for (const auto& block : blocks) {
        auto entry = entries.find(block.id);
        if (entry != entries.end() && entry->second.sourceFile != file) {
            remap[block.id] = UUID::GenerateRandom();
        }
    }

    ApplyRemap(blocks, remap);
    if (remapped) *remapped = remap;

    std::vector<const DelusiveParser::DataBlock*> view;
    view.reserve(blocks.size());
    for (const auto& block : blocks) view.push_back(&block);

    if (!WriteBlocks(path, view)) return false;

    ReplaceFile(path, std::move(blocks));
    return true;
}

bool DelusiveLibrary::SaveFile(const std::string& path) const {
    auto fileList = byFile.find(FileKey(path));
    if (fileList == byFile.end()) return false;

    return WriteBlocks(path, fileList->second);
}

bool DelusiveLibrary::SaveAll() const {
    bool success = true;

    for (const auto& [path, blocks] : byFile) {
        if (!SaveFile(path)) success = false;
    }

    return success;
}

#pragma endregion

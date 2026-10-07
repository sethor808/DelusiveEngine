#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <Delusive/Runtime/Core/UUID.h>
#include <string>
#include <vector>
#include <unordered_map>

//Resident store of every parsed DataBlock in the game, searchable by UUID, category and name.
//Holds recipes only - live instances belong to UUIDManager.
class DelusiveLibrary {
public:
    //Old id -> fresh id, for blocks that were saved as copies
    using IDRemap = std::unordered_map<UUID, UUID, UUID::Hash>;
    //Gives remapped blocks their fresh ids and rewrites every reference to them -
    //ownership lists and links store ids as text
    static void ApplyRemap(std::vector<DelusiveParser::DataBlock>& blocks, const IDRemap& remap);

    //In-memory blocks that Find checks before the indexed files while this is alive.
    //Clone uses it so owned objects resolve from the copy's current state instead of
    //the saved versions. Nothing is read from or written to disk.
    class Overlay {
    public:
        Overlay(DelusiveLibrary&, const std::vector<DelusiveParser::DataBlock>& blocks);
        ~Overlay();
        Overlay(const Overlay&) = delete;
        Overlay& operator=(const Overlay&) = delete;
    private:
        DelusiveLibrary& library;
        std::unordered_map<UUID, const DelusiveParser::DataBlock*, UUID::Hash> blocks;
    };

    DelusiveLibrary() = default;
    DelusiveLibrary(const DelusiveLibrary&) = delete;
    DelusiveLibrary& operator=(const DelusiveLibrary&) = delete;
    DelusiveLibrary(DelusiveLibrary&&) noexcept = default;
    DelusiveLibrary& operator=(DelusiveLibrary&&) noexcept = default;

    //Load - a file is the unit of truth: (re)loading one replaces everything
    //previously indexed from it, so reloads never serve stale or deleted blocks
    void LoadAll();
    void LoadDirectory(const std::string& path, const std::string& extension);
    bool LoadFile(const std::string& path);
    void ReplaceFile(const std::string& path, std::vector<DelusiveParser::DataBlock> blocks);
    void RemoveFile(const std::string& path);
    void Clear();

    //Lookup
    bool Has(const UUID&) const;
    const DelusiveParser::DataBlock* Find(const UUID&) const;
    const std::vector<const DelusiveParser::DataBlock*>& List(const std::string&) const;
    //Blocks indexed from one file, in file order
    const std::vector<const DelusiveParser::DataBlock*>& ListFile(const std::string& path) const;

    //Returns every block holding a property whose value is this UUID
    std::vector<UUID> FindReferences(const UUID&) const;

    //Editing
    void Add(DelusiveParser::DataBlock, const std::string& sourceFile);
    void Update(const UUID&, DelusiveParser::DataBlock);
    void Remove(const UUID&);

    //Save
    //Writes blocks to disk, then makes them the library's copy of that file.
    //A block belongs to one file: any whose id is owned by another file is a copy
    //(save as, copied file, pasted content) and gets a fresh id, with references to
    //it inside these blocks rewritten. Pass remapped to learn which ids changed.
    bool WriteFile(const std::string& path, std::vector<DelusiveParser::DataBlock> blocks,
        IDRemap* remapped = nullptr);
    bool SaveFile(const std::string& path) const;
    bool SaveAll() const;
    std::string GetSourceFile(const UUID&) const;

private:
    struct Entry {
        DelusiveParser::DataBlock block;
        std::string sourceFile;
    };

    std::unordered_map<UUID, Entry, UUID::Hash> entries;
    //Innermost overlay last
    std::vector<const std::unordered_map<UUID, const DelusiveParser::DataBlock*, UUID::Hash>*> overlays;
    std::unordered_map<std::string, std::vector<const DelusiveParser::DataBlock*>> byCategory;
    //Preserves per-file ordering so saving does not reshuffle the file
    std::unordered_map<std::string, std::vector<const DelusiveParser::DataBlock*>> byFile;
};

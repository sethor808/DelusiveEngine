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
    DelusiveLibrary() = default;
    DelusiveLibrary(const DelusiveLibrary&) = delete;
    DelusiveLibrary& operator=(const DelusiveLibrary&) = delete;
    DelusiveLibrary(DelusiveLibrary&&) noexcept = default;
    DelusiveLibrary& operator=(DelusiveLibrary&&) noexcept = default;

    //Load
    void LoadAll();
    void LoadDirectory(const std::string& path, const std::string& extension);
    void LoadFile(const std::string& path);
    void Clear();

    //Lookup
    bool Has(const UUID&) const;
    const DelusiveParser::DataBlock* Find(const UUID&) const;
    const std::vector<const DelusiveParser::DataBlock*>& List(const std::string&) const;

    //Returns every block holding a property whose value is this UUID
    std::vector<UUID> FindReferences(const UUID&) const;

    //Editing
    void Add(DelusiveParser::DataBlock, const std::string& sourceFile);
    void Update(const UUID&, DelusiveParser::DataBlock);
    void Remove(const UUID&);

    //Save
    bool SaveFile(const std::string& path) const;
    bool SaveAll() const;
    std::string GetSourceFile(const UUID&) const;

private:
    struct Entry {
        DelusiveParser::DataBlock block;
        std::string sourceFile;
    };

    std::unordered_map<UUID, Entry, UUID::Hash> entries;
    std::unordered_map<std::string, std::vector<const DelusiveParser::DataBlock*>> byCategory;
    //Preserves per-file ordering so saving does not reshuffle the file
    std::unordered_map<std::string, std::vector<const DelusiveParser::DataBlock*>> byFile;
};

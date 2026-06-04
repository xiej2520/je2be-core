#pragma once

#include <minecraft-file.hpp>
#include <je2be/pos2.hpp>
#include "_volume.hpp"
#include "je2be/nbt.hpp"

#include "_structure-piece.hpp"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace je2be::lce {
  
struct StructurePiece;

enum class StructureType {
  BuriedTreasure,
  DesertPyramid,
  EndCity,
  Fortress,
  Igloo,
  JungleTemple,
  Mineshaft,
  OceanMonument,
  Stronghold,
  SwampHut,
  Village,
  WoodlandMansion,
};

std::u8string_view FeatureName(StructureType type);
std::u8string NamespaceFeatureName(StructureType type);

struct Structure {
  Structure(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::vector<std::unique_ptr<StructurePiece>> pieces = {});
  Structure(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::unique_ptr<StructurePiece> piece);
  static std::optional<Structure> Parse(std::span<const u8> bytes, StructureType type);

  Structure(const Structure&) = delete;
  Structure& operator=(const Structure&) = delete;

  Structure(Structure&&) noexcept;
  Structure& operator=(Structure&&) noexcept;

  ~Structure();

  StructureType fType;
  i32 fChunkX;
  i32 fChunkZ;
  Volume fBoundingBox;
  std::vector<std::unique_ptr<StructurePiece>> fPieces;
  std::vector<Pos2i> fProcessed; // monuments only

  CompoundTagPtr Convert() const;
};

} // namespace je2be::lce

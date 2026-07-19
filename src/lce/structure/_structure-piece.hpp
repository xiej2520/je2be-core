#pragma once

#include <minecraft-file.hpp>
#include <je2be/pos2.hpp>
#include <string>
#include <vector>
#include "_volume.hpp"
#include "je2be/nbt.hpp"

namespace je2be::lce {

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

constexpr std::u8string_view FeatureName(StructureType type) {
  // 1.13-1.17: feature name ids are stored in chunks, without "minecraft:" namespace prefix
  //   in Level.Structure.Starts.{FeatureName} and Level.Structure.References.{FeatureName}.
  //   Structure start "id" field does use "minecraft:" prefix.
  // 1.18+: chunk structure data uses identifier with "minecraft:" namespace prefix
  switch (type) {
    case StructureType::BuriedTreasure: return u8"buried_treasure";
    case StructureType::DesertPyramid: return u8"desert_pyramid";
    case StructureType::EndCity: return u8"endcity";
    case StructureType::Fortress: return u8"fortress";
    case StructureType::Igloo: return u8"igloo";
    case StructureType::JungleTemple: return u8"jungle_pyramid";
    case StructureType::Mineshaft: return u8"mineshaft";
    case StructureType::OceanMonument: return u8"monument";
    case StructureType::Stronghold: return u8"stronghold";
    case StructureType::SwampHut: return u8"swamp_hut";
    case StructureType::Village: return u8"village";
    case StructureType::WoodlandMansion: return u8"mansion";
    default: return u8"INVALID";
  }
}

// used in structure start "id" field
constexpr std::u8string NamespaceFeatureName(StructureType type) {
  return u8"minecraft:" + std::u8string(FeatureName(type));
}

enum class StructurePieceType {
  OMB,
  TeSH,
};

constexpr std::u8string_view PieceId(StructurePieceType piece) {
  switch (piece) {
  case StructurePieceType::OMB: return u8"minecraft:omb"; // ocean monument building
  case StructurePieceType::TeSH: return u8"minecraft:tesh"; // swamp hut
  default: return u8"INVALID";
  }
}

struct StructurePiece {
  Volume fBB;
  i32 fOrientation;
  i32 fGenerationDepth;
  StructurePieceType fId;
  StructurePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id) :
    fBB(bb), fOrientation{orientation}, fGenerationDepth{generationDepth}, fId(id) {}

  class Impl;

  virtual ~StructurePiece() = default;
  virtual CompoundTagPtr Convert() const;
};

struct TemplePiece : StructurePiece {
  i32 fWidth;
  i32 fHeight;
  i32 fDepth;
  i32 fHPos;
  TemplePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, i32 width, i32 height, i32 depth, i32 hPos)
  : StructurePiece(bb, orientation, generationDepth, id),
    fWidth{width}, fHeight{height}, fDepth{depth}, fHPos{hPos} {}

  class Impl;

  CompoundTagPtr Convert() const override;
};

struct StructureFeature {
  i32 fChunkX;
  i32 fChunkZ;
  Volume fBoundingBox;
  StructureType fType;
  std::vector<std::unique_ptr<StructurePiece>> fPieces;
  std::vector<Pos2i> fProcessed; // monuments only

  StructureFeature(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::vector<std::unique_ptr<StructurePiece>> pieces = {})
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fBoundingBox{std::move(bb)}, fPieces(std::move(pieces)) {}

  StructureFeature(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::unique_ptr<StructurePiece> piece)
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fBoundingBox{std::move(bb)}, fPieces() {
    fPieces.emplace_back(std::move(piece));
  }
  
  class Impl;
  
  CompoundTagPtr Convert() const;
};

#include <iostream>

inline std::ostream& operator<<(std::ostream& os, const je2be::Volume& v) {
    os << "Volume("
       << "start=(" << v.fStart.fX << ", " << v.fStart.fY << ", " << v.fStart.fZ << "), "
       << "end=(" << v.fEnd.fX << ", " << v.fEnd.fY << ", " << v.fEnd.fZ << "))";
    return os;
}
inline std::ostream& operator<<(std::ostream& os, const je2be::lce::StructurePiece& p) {
    os << "StructurePiece("
       << "volume=" << p.fBB
       << ", orientation=" << p.fOrientation
       << ", generationDepth=" << p.fGenerationDepth
       << ")";
    return os;
}
inline std::ostream& operator<<(std::ostream& os, const je2be::lce::TemplePiece& p) {
    os << "TemplePiece("
       << "volume=" << p.fBB
       << ", orientation=" << p.fOrientation
       << ", generationDepth=" << p.fGenerationDepth
       << ", width=" << p.fWidth
       << ", height=" << p.fHeight
       << ", depth=" << p.fDepth
       << ", hPos=" << p.fHPos
       << ")";
    return os;
}
inline std::ostream& operator<<(std::ostream& os, je2be::lce::StructureType type) {
    using je2be::lce::StructureType;

    switch (type) {
    case StructureType::Fortress:;
    case StructureType::OceanMonument:;
    case StructureType::SwampHut:
    case StructureType::BuriedTreasure:
    case StructureType::DesertPyramid:
    case StructureType::EndCity:
    case StructureType::Igloo:
    case StructureType::JungleTemple:
    case StructureType::Mineshaft:
    case StructureType::Stronghold:
    case StructureType::Village:
    case StructureType::WoodlandMansion:
      return os << std::string(FeatureName(type).begin(), FeatureName(type).end());
    default: return os << "Unknown(" << static_cast<int>(type) << ")";
    }
}
inline std::ostream& operator<<(std::ostream& os, const je2be::lce::StructureFeature& s) {
    os << "StructureStart {\n";
    os << "  type: " << s.fType << "\n";
    os << "  chunk: (" << s.fChunkX << ", " << s.fChunkZ << ")\n";
    os << "  volume: " << s.fBoundingBox << "\n";
    os << "  pieces:\n";

    for (auto const& piece : s.fPieces) {
        os << "    " << *piece << '\n';
    }
    os << "  processed: ";
    for (auto const& proc : s.fProcessed) {
        os << "(" << proc.fX << ", " << proc.fZ << "), ";
    }

    os << "\n}";
    return os;
}


} // namespace je2be::lce


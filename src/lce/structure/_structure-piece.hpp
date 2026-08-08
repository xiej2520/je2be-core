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
  //   v2970 StructuresBecomeConfiguredFix converts structures to use prefix
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

// namespaced feature name is used in structure start "id" field
constexpr std::u8string NamespaceFeatureName(StructureType type) {
  return u8"minecraft:" + std::u8string(FeatureName(type));
}

enum class StructurePieceType {
  // ocean monument building seems to be the only ocean monument piece saved in LCE and Java 1.12.2
  OMB,

  TeJP, // jungle pyramid
  Iglu, // igloo
  TeSH, // swamp hut
  TeDP, // desert pyramid
  
  SHCC,    // StrongholdChestCorridor
  SHPR,    // StrongholdPortalRoom
  SH5C,    // StrongholdFiveCrossing
  SHLi,    // StrongholdLibrary
  SHFC,    // StrongholdFillerCorridor
  SHRC,    // StrongholdRoomCrossing
  SHS,     // StrongholdStraight
  SHStart, // StrongholdStartPiece
  SHSD,    // StrongholdStairsDown
  SHLT,    // StrongholdLeftTurn
  SHPH,    // StrongholdPrisonHall
  SHRT,    // StrongholdRightTurn
  SHSSD,   // StrongholdStraightStairsDown

  NeBCr,   // BridgeCrossing
  NeBEF,   // BridgeEndFiller
  NeBS,    // BridgeStraight
  NeCCS,   // CastleCorridorStairsPiece
  NeCTB,   // CastleCorridorTBalconyPiece
  NeCE,    // CastleEntrance
  NeSCSC,  // CastleSmallCorridorCrossingPiece
  NeSCLT,  // CastleSmallCorridorLeftTurnPiece
  NeSC,    // CastleSmallCorridorPiece
  NeSCRT,  // CastleSmallCorridorRightTurnPiece
  NeCSR,   // CastleStalkRoom (netherwart farm)
  NeMT,    // MonsterThrone (blaze spawner)
  NeRC,    // RoomCrossing
  NeSR,    // StairsRoom
  NeStart, // StartPiece
  
  ECP,     // EndCityPiece
};

extern const std::unordered_map<std::u8string, StructurePieceType> sPieceType;

// namespaced structure pieces are registered in lowercase in 1.14+
constexpr std::u8string_view PieceId(StructurePieceType piece) {
  switch (piece) {
  case StructurePieceType::OMB: return u8"minecraft:omb";

  case StructurePieceType::TeJP: return u8"minecraft:tejp";
  case StructurePieceType::Iglu: return u8"minecraft:iglu";
  case StructurePieceType::TeSH: return u8"minecraft:tesh";
  case StructurePieceType::TeDP: return u8"minecraft:tedp";

  case StructurePieceType::SHCC: return u8"minecraft:shcc";
  case StructurePieceType::SHPR: return u8"minecraft:shpr";
  case StructurePieceType::SH5C: return u8"minecraft:sh5c";
  case StructurePieceType::SHLi: return u8"minecraft:shli";
  case StructurePieceType::SHFC: return u8"minecraft:shfc";
  case StructurePieceType::SHRC: return u8"minecraft:shrc";
  case StructurePieceType::SHS: return u8"minecraft:shs";
  case StructurePieceType::SHStart: return u8"minecraft:shstart";
  case StructurePieceType::SHSD: return u8"minecraft:shsd";
  case StructurePieceType::SHLT: return u8"minecraft:shlt";
  case StructurePieceType::SHPH: return u8"minecraft:shph";
  case StructurePieceType::SHRT: return u8"minecraft:shrt";
  case StructurePieceType::SHSSD: return u8"minecraft:shssd";

  case StructurePieceType::NeBCr: return u8"minecraft:nebcr";
  case StructurePieceType::NeBEF: return u8"minecraft:nebef";
  case StructurePieceType::NeBS: return u8"minecraft:nebs";
  case StructurePieceType::NeCCS: return u8"minecraft:neccs";
  case StructurePieceType::NeCTB: return u8"minecraft:nectb";
  case StructurePieceType::NeCE: return u8"minecraft:nece";
  case StructurePieceType::NeSCSC: return u8"minecraft:nescsc";
  case StructurePieceType::NeSCLT: return u8"minecraft:nesclt";
  case StructurePieceType::NeSC: return u8"minecraft:nesc";
  case StructurePieceType::NeSCRT: return u8"minecraft:nescrt";
  case StructurePieceType::NeCSR: return u8"minecraft:necsr";
  case StructurePieceType::NeMT: return u8"minecraft:nemt";
  case StructurePieceType::NeRC: return u8"minecraft:nerc";
  case StructurePieceType::NeSR: return u8"minecraft:nesr";
  case StructurePieceType::NeStart: return u8"minecraft:nestart";

  case StructurePieceType::ECP: return u8"ecp";
  default: return u8"INVALID";
  }
}

struct StructurePiece {
  Volume fBB;
  i32 fOrientation;
  i32 fGenerationDepth;
  StructurePieceType fId;
  StructurePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id);

  class Impl;

  virtual ~StructurePiece() = default;
  virtual CompoundTagPtr Convert() const;

  static std::unique_ptr<StructurePiece> ExtractPiece(mcfile::stream::InputStreamReader &reader);
};

struct TemplePiece : StructurePiece {
  i32 fWidth;
  i32 fHeight;
  i32 fDepth;
  i32 fHPos;  // y level of surface the structure was moved to, or -1 if not moved
  TemplePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, i32 width, i32 height, i32 depth, i32 hPos);

  class Impl;

  CompoundTagPtr Convert() const override;
};

struct FortressPiece : StructurePiece {
  std::optional<bool> fMob;
  std::optional<i32> fSeed;
  std::optional<bool> fChest;

  FortressPiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id,
    std::optional<bool> mob, std::optional<i32> seed, std::optional<bool> chest
  );

  CompoundTagPtr Convert() const override;
};

struct StrongholdPiece : StructurePiece {
  i32 fEntryDoor;
  CompoundTagPtr fData;

  StrongholdPiece(
    Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id,
    i32 entryDoor,
    CompoundTagPtr data
  );

  CompoundTagPtr Convert() const override;
};

// EndCity structures store 0-length string for id instead of "EndCity",
// and GD is a random Int
struct EndCityPiece : StructurePiece {
  i32 fTPX;
  i32 fTPY;
  i32 fTPZ;
  // LCE doesn't store "Rot", "OW", or "Template" fields for end cities
  EndCityPiece(
    Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id,
    i32 tpx, i32 tpy, i32 tpz
  );

  CompoundTagPtr Convert() const override;
};

struct StructureFeature {
  StructureType fType;
  i32 fChunkX;
  i32 fChunkZ;
  Volume fBoundingBox;
  std::vector<std::unique_ptr<StructurePiece>> fPieces;
  std::vector<Pos2i> fProcessed; // monuments only

  StructureFeature(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::vector<std::unique_ptr<StructurePiece>> pieces = {});

  StructureFeature(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::unique_ptr<StructurePiece> piece);
  
  class Impl;
  
  CompoundTagPtr Convert() const;

  static std::optional<StructureFeature> Extract(std::span<const u8> bytes, StructureType type);
};

bool readBB(mcfile::stream::InputStreamReader& reader, Volume& out);

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


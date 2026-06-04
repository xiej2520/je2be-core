#pragma once

#include <minecraft-file.hpp>
#include <je2be/pos2.hpp>

#include "_volume.hpp"
#include "je2be/nbt.hpp"

#include <memory>

namespace je2be::lce {
  
enum class StructureType;

enum class StructurePieceType {
  OMB, // OceanMonumentBuilding is the only piece saved in Java 1.12.2 and LCE
  BTP, // BuriedTreasurePiece

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

std::u8string_view PieceId(StructurePieceType piece);

struct StructurePiece {
  Volume fBB;
  i32 fOrientation;
  i32 fGenerationDepth;
  StructurePieceType fId;
  // piece-specific
  CompoundTagPtr fData;
  StructurePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, CompoundTagPtr data);

  virtual ~StructurePiece() = default;
  virtual CompoundTagPtr Convert() const;

  static std::unique_ptr<StructurePiece> Parse(mcfile::stream::InputStreamReader &reader, StructureType type);
};

struct TemplePiece : StructurePiece {
  i32 fWidth;
  i32 fHeight;
  i32 fDepth;
  i32 fHPos;  // y level of surface the structure was moved to, or -1 if not moved
  TemplePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, CompoundTagPtr data,
    i32 width, i32 height, i32 depth, i32 hPos
  );

  CompoundTagPtr Convert() const override;
};

struct StrongholdPiece : StructurePiece {
  i32 fEntryDoor;

  StrongholdPiece(
    Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, CompoundTagPtr data,
    i32 entryDoor
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
    CompoundTagPtr data, i32 tpx, i32 tpy, i32 tpz
  );

  CompoundTagPtr Convert() const override;
};

bool readBB(mcfile::stream::InputStreamReader& reader, Volume& out);

} // namespace je2be::lce

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

bool readBB(mcfile::stream::InputStreamReader& reader, Volume& out);

} // namespace je2be::lce

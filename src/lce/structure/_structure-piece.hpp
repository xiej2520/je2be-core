#pragma once

#include <minecraft-file.hpp>
#include <je2be/pos2.hpp>

#include "_volume.hpp"
#include "je2be/nbt.hpp"

#include <memory>

namespace je2be::lce {
  
enum class StructureType;

enum class StructurePieceType {
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

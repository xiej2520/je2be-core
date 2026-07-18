#pragma once

#include <minecraft-file.hpp>
#include <je2be/pos2.hpp>
#include <vector>
#include "_volume.hpp"
#include "je2be/nbt.hpp"

namespace je2be::lce {

enum class StructureType : u8 {
  Fortress,
  Monument,
  Outpost,
  SwampHut,
};

struct StructurePiece {
  Volume fBB;
  i32 fOrientation;
  i32 fGenerationDepth;
  i32 fWidth;
  i32 fHeight;
  i32 fDepth;
  i32 fHPos;
  StructurePiece(Volume bb, i32 orientation, i32 generationDepth, i32 width, i32 height, i32 depth, i32 hPos) :
    fBB(bb), fOrientation{orientation}, fGenerationDepth{generationDepth}, fWidth{width}, fHeight{height}, fDepth{depth}, fHPos{hPos} {}

  class Impl;
  
  CompoundTagPtr Convert(StructureType type) const;
};

struct StructureFeature {
  i32 fChunkX;
  i32 fChunkZ;
  Volume fBoundingBox;
  StructureType fType;
  std::vector<StructurePiece> fPieces;

  StructureFeature(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::vector<StructurePiece> pieces = {})
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fBoundingBox{std::move(bb)}, fPieces{std::move(pieces)} {}
  
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
    case StructureType::Fortress: return os << "Fortress";
    case StructureType::Monument: return os << "Monument";
    case StructureType::Outpost:  return os << "Outpost";
    case StructureType::SwampHut: return os << "SwampHut";
    default: return os << "Unknown(" << static_cast<int>(type) << ")";
    }
}
inline std::ostream& operator<<(std::ostream& os, const je2be::lce::StructureFeature& s) {
    os << "StructureStart {\n";
    os << "  type: " << s.fType << "\n";
    os << "  chunk: (" << s.fChunkX << ", " << s.fChunkZ << ")\n";
    os << "  volume: " << s.fBoundingBox << "\n";
    os << "  pieces:\n";

    for (const auto& piece : s.fPieces) {
        os << "    " << piece << '\n';
    }

    os << "}";
    return os;
}


} // namespace je2be::lce


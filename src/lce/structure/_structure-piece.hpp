#pragma once

#include <minecraft-file.hpp>
#include <je2be/pos2.hpp>
#include <vector>
#include "_volume.hpp"

namespace je2be::lce {

enum class StructureType : u8 {
  Fortress = 1,
  Monument = 3,
  Outpost = 5,
  SwampHut,
};

struct StructurePiece {
  i32 fOrientation;
  i32 fGenerationDepth;
  i32 fWidth;
  i32 fHeight;
  i32 fDepth;
  i32 fHPos;
  StructurePiece(i32 orientation, i32 generationDepth, i32 width, i32 height, i32 depth, i32 hPos) :
    fOrientation{orientation}, fGenerationDepth{generationDepth}, fWidth{width}, fHeight{height}, fDepth{depth}, fHPos{hPos} {}
};

struct StructureStart {
  i32 fChunkX;
  i32 fChunkZ;
  Volume fVolume;
  StructureType fType;
  std::vector<StructurePiece> fPieces;

  StructureStart(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::vector<StructurePiece> pieces = {})
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fVolume{std::move(bb)}, fPieces{std::move(pieces)} {}
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
       << "orientation=" << p.fOrientation
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
inline std::ostream& operator<<(std::ostream& os, const je2be::lce::StructureStart& s) {
    os << "StructureStart {\n";
    os << "  type: " << s.fType << "\n";
    os << "  chunk: (" << s.fChunkX << ", " << s.fChunkZ << ")\n";
    os << "  volume: " << s.fVolume << "\n";
    os << "  pieces:\n";

    for (const auto& piece : s.fPieces) {
        os << "    " << piece << '\n';
    }

    os << "}";
    return os;
}


} // namespace je2be::lce


#pragma once

#include <minecraft-file.hpp>
#include <je2be/pos2.hpp>
#include "_volume.hpp"

namespace je2be::lce {

enum class StructureType : u8 {
  Fortress = 1,
  Monument = 3,
  Outpost = 5,
};

struct StructurePiece {
  i32 fChunkX;
  i32 fChunkZ;
  Volume fVolume;
  i32 fOrientation;
  i32 fGenerationDepth;
  i32 fWidth;
  i32 fHeight;
  i32 fDepth;
  i32 fHPos;

  StructurePiece(i32 chunkX, i32 chunkZ, Volume bb, i32 O, i32 GD, i32 width, i32 height, i32 depth, i32 hPos)
  : fChunkX(chunkX), fChunkZ(chunkZ), fVolume(bb), fOrientation(O), fGenerationDepth(GD), fWidth(width), fHeight(height), fDepth(depth),
    fHPos(hPos) {}
};

} // namespace je2be::java


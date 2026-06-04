#pragma once

#include "structure/_structure-piece.hpp"

namespace je2be::lce {

class Structures {
public:
  struct Structure {
    StructureType fType;
    Volume fBounds;
    Pos2i fStartChunk;

    Structure(StructureType type, Volume bounds, Pos2i startChunk) : fType(type), fBounds(bounds), fStartChunk(startChunk) {}
  };

  void add(StructurePiece p, mcfile::Dimension dim) {
    switch (dim) {
    case mcfile::Dimension::Overworld:
      fOverworld.push_back(p);
      break;
    case mcfile::Dimension::Nether:
      fNether.push_back(p);
      break;
    case mcfile::Dimension::End:
      fEnd.push_back(p);
      break;
    }
  }

private:
  std::vector<StructurePiece> fOverworld;
  std::vector<StructurePiece> fNether;
  std::vector<StructurePiece> fEnd;
  //StructurePieceCollection fOverworld;
  //StructurePieceCollection fNether;
  //StructurePieceCollection fEnd;
};

} // namespace je2be::java

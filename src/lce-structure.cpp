#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>

#include "mcfile/encoding.hpp"
#include "_volume.hpp"
#include <je2be/nbt.hpp>

#include "lce/structure/_structure.hpp"
#include "lce/structure/_structure-piece.hpp"

#include "mcfile/stream/input-stream-reader.hpp"
#include "mcfile/stream/byte-stream.hpp"

namespace je2be::lce {

std::u8string_view FeatureName(StructureType type) {
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
std::u8string NamespaceFeatureName(StructureType type) {
  return u8"minecraft:" + std::u8string(FeatureName(type));
}

const std::unordered_map<std::u8string, StructurePieceType> sPieceType {
  {u8"OMB", StructurePieceType::OMB},

  {u8"NeBCr", StructurePieceType::NeBCr},
  {u8"NeBEF", StructurePieceType::NeBEF},
  {u8"NeBS", StructurePieceType::NeBS},
  {u8"NeCCS", StructurePieceType::NeCCS},
  {u8"NeCTB", StructurePieceType::NeCTB},
  {u8"NeCE", StructurePieceType::NeCE},
  {u8"NeSCSC", StructurePieceType::NeSCSC},
  {u8"NeSCLT", StructurePieceType::NeSCLT},
  {u8"NeSC", StructurePieceType::NeSC},
  {u8"NeSCRT", StructurePieceType::NeSCRT},
  {u8"NeCSR", StructurePieceType::NeCSR},
  {u8"NeMT", StructurePieceType::NeMT},
  {u8"NeRC", StructurePieceType::NeRC},
  {u8"NeSR", StructurePieceType::NeSR},
  {u8"NeStart", StructurePieceType::NeStart},
};

// namespaced structure pieces are registered in lowercase in 1.14+
std::u8string_view PieceId(StructurePieceType piece) {
  switch (piece) {
  case StructurePieceType::OMB: return u8"minecraft:omb";

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
  default: return u8"INVALID";
  }
}


std::shared_ptr<IntArrayTag> toBoundingBox(const Volume &volume) {
  auto const &start = volume.fStart;
  auto const &end = volume.fEnd;

  std::vector<i32> values {
    start.fX, start.fY, start.fZ,
    end.fX, end.fY, end.fZ,
  };

  return std::make_shared<IntArrayTag>(values);
}

Structure::Structure(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::vector<std::unique_ptr<StructurePiece>> pieces)
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fBoundingBox{std::move(bb)}, fPieces(std::move(pieces)) {}

Structure::Structure(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::unique_ptr<StructurePiece> piece)
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fBoundingBox{std::move(bb)}, fPieces() {
    fPieces.emplace_back(std::move(piece));
}

Structure::Structure(Structure&&) noexcept = default;
Structure& Structure::operator=(Structure&&) noexcept = default;
Structure::~Structure() = default;

// structure:
//   id
//   ChunkX
//   ChunkZ
//   Children[]
//   Processed (Monument only)
std::optional<Structure> Structure::Parse(std::span<const u8> bytes, StructureType type) {
  std::vector<u8> buffer(bytes.begin(), bytes.end());
  auto stream = std::make_shared<mcfile::stream::ByteStream>(buffer);
  
  mcfile::stream::InputStreamReader reader{stream, mcfile::Encoding::Java};

  // 4 byte version (?) header, 4 for 1.83, 3 for 1.69, 1.45, 1 for 1.21, 1.31, 0 for 1.15
  if (!reader.seek(4)) {
    return std::nullopt;
  }

  std::u8string id;
  if (!reader.read(id)) {
    return std::nullopt;
  }
 
  i32 chunkX;
  i32 chunkZ;
  if (!reader.read(&chunkX) || !reader.read(&chunkZ)) {
    return std::nullopt;
  }

  Volume bb{{0, 0, 0}, {0, 0, 0}};
  if (!readBB(reader, bb)) {
    return std::nullopt;
  }
  
  i32 childrenLen;
  if (!reader.read(&childrenLen) || childrenLen < 0 || childrenLen > 4096) { // arbitrary 4096 limit
    return std::nullopt;
  }
  
  std::vector<std::unique_ptr<StructurePiece>> pieces;
  
  for (size_t i = 0; i < childrenLen; i++) {
    auto piece = StructurePiece::Parse(reader, type);
    if (!piece) {
      // invalid piece, stop
      break;
    }
    pieces.push_back(std::move(piece));
  }


  Structure start{ type, chunkX, chunkZ, bb, std::move(pieces) };

  if (type == StructureType::OceanMonument) {
    // Ocean Monument has extra `Processed` bytes, if we can't read it then just skip them, there
    // should only be 1 OMB piece in the structure.
    i32 processedLen;
    if (reader.read(&processedLen)) {
      for (size_t i = 0; i < processedLen; i++) {
        i32 x;
        i32 z;
        if (!reader.read(&x) || !reader.read(&z)) {
          break;
        }
        start.fProcessed.emplace_back(x, z);
      }
    }
  }
  
  return start;
}

CompoundTagPtr Structure::Convert() const {
  auto out = Compound();
  out->set(u8"ChunkX", Int(fChunkX));
  out->set(u8"ChunkZ", Int(fChunkZ));
  out->set(u8"id", String(NamespaceFeatureName(fType)));
  //out->set(u8"references", Int(0)); // not sure what this is used for
  out->set(u8"BB", toBoundingBox(fBoundingBox));

  auto children = List<Tag::Type::Compound>();
  for (auto const &piece : fPieces) {
    children->push_back(piece->Convert());
  }
  out->set(u8"Children", children);

  switch (fType) {
  case StructureType::OceanMonument: {
      // "Processed" doesn't appear to be used in game code since 1.14
      //auto processedTag = List<Tag::Type::Compound>();
      //for (auto const &chunkCoord : fProcessed) {
      //  auto chunkCoordTag = Compound();
      //  chunkCoordTag->set(u8"X", Int(chunkCoord.fX));
      //  chunkCoordTag->set(u8"Z", Int(chunkCoord.fZ));
      //  processedTag->push_back(chunkCoordTag);
      //}
      //out->set(u8"Processed", processedTag);
      break;
    }
    default:
      break;
    }

  return out;
}

StructurePiece::StructurePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, CompoundTagPtr data) :
  fBB(bb), fOrientation{orientation}, fGenerationDepth{generationDepth}, fId(id), fData(data) {}

std::unique_ptr<StructurePiece> StructurePiece::Parse(mcfile::stream::InputStreamReader &reader, StructureType type) {
  std::u8string pieceId;
  if (!reader.read(pieceId)) {
    return nullptr;
  }

  // common StructurePiece fields
  Volume pieceBB{{0, 0, 0}, {0, 0, 0}};
  if (!readBB(reader, pieceBB)) {
    return nullptr;
  }
  i32 O;  // orientation
  i32 GD; // generation depth
  if (!reader.read(&O) || !reader.read(&GD)) {
    return nullptr;
  }

  auto data = Compound();

  StructurePieceType id;

  auto it = sPieceType.find(pieceId);
  if (it == sPieceType.end()) {
    return nullptr;
  }
  id = it->second;
  switch (id) {
  case StructurePieceType::OMB: break;

  case StructurePieceType::NeMT: { // blaze spawner
    u8 b;
    if (!reader.read(&b)) {
      return nullptr;
    }
    data->set(u8"Mob", Bool(static_cast<bool>(b)));
    break;
  }
  case StructurePieceType::NeBEF: {
    i32 seed;
    if (!reader.read(&seed)) {
      return nullptr;
    }
    data->set(u8"Seed", Int(seed));
    break;
  }
  case StructurePieceType::NeSCLT: // fallthrough
  case StructurePieceType::NeSCRT: {
    u8 b;
    if (!reader.read(&b)) {
      return nullptr;
    }
    data->set(u8"Chest", Bool(static_cast<bool>(b)));
    break;
  }
  case StructurePieceType::NeBCr:
  case StructurePieceType::NeBS:
  case StructurePieceType::NeCCS:
  case StructurePieceType::NeCTB:
  case StructurePieceType::NeCE:
  case StructurePieceType::NeSCSC:
  case StructurePieceType::NeSC:
  case StructurePieceType::NeCSR:
  case StructurePieceType::NeRC:
  case StructurePieceType::NeSR:
  case StructurePieceType::NeStart:
    break;
  }

  return std::make_unique<StructurePiece>(pieceBB, O, GD, id, data);
}

CompoundTagPtr StructurePiece::Convert() const {
  auto out = Compound();
  // sometimes child bounding box can be outside structure bounding box (e.g. witch huts),
  // fixed at some point between Java 1.13 and 1.15
  out->set(u8"BB", toBoundingBox(fBB));
  out->set(u8"GD", Int(fGenerationDepth));
  out->set(u8"O", Int(fOrientation));
  out->set(u8"id", String(PieceId(fId)));

  if (fData) {
    for (auto const &t : *fData) {
      out->set(t.first, t.second);
    }
  }

  return out;
}

bool readBB(mcfile::stream::InputStreamReader& reader, Volume& out) {
  i32 x1, y1, z1, x2, y2, z2;

  if (!reader.read(&x1) || !reader.read(&y1) || !reader.read(&z1) ||
      !reader.read(&x2) || !reader.read(&y2) || !reader.read(&z2)) {
    return false;
  }

  out = Volume{
    Pos3i{x1, y1, z1},
    Pos3i{x2, y2, z2},
  };

  return true;
}

} // namespace je2be::lce

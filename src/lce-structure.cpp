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

  {u8"TeJP", StructurePieceType::TeJP},
  {u8"Iglu", StructurePieceType::Iglu},
  {u8"TeSH", StructurePieceType::TeSH},
  {u8"TeDP", StructurePieceType::TeDP},

  {u8"SHCC",     StructurePieceType::SHCC},
  {u8"SHPR",     StructurePieceType::SHPR},
  {u8"SH5C",     StructurePieceType::SH5C},
  {u8"SHLi",     StructurePieceType::SHLi},
  {u8"SHFC",     StructurePieceType::SHFC},
  {u8"SHRC",    StructurePieceType::SHRC},
  {u8"SHS",     StructurePieceType::SHS},
  {u8"SHStart", StructurePieceType::SHStart},
  {u8"SHSD",    StructurePieceType::SHSD},
  {u8"SHLT",    StructurePieceType::SHLT},
  {u8"SHPH",    StructurePieceType::SHPH},
  {u8"SHRT",    StructurePieceType::SHRT},
  {u8"SHSSD",   StructurePieceType::SHSSD},

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

  // pieces should always be > 0, but allow it for now in case we can't parse all children
  if (pieces.size() != 0) {
    switch (pieces[0]->fId) {
    case StructurePieceType::TeJP: type = StructureType::JungleTemple; break;
    case StructurePieceType::Iglu: type = StructureType::Igloo; break;
    case StructurePieceType::TeSH: type = StructureType::SwampHut; break;
    case StructurePieceType::TeDP: type = StructureType::DesertPyramid; break;
    default:
      break;
    }
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
    case StructureType::SwampHut: {
      // note: witch hut bounding box is 7x7x9 in 1.8.1+
      // LCE usually has correct size, sometimes broken 8x7x10 boxes idk?
      // In Java 1.12- and LCE, witch hut child bounding boxes are sometimes not aligned with the
      // full structure bounding box, which causes spawning to fail. Fix by setting BB to child box.
      if (!fPieces.empty()) {
        out->set(u8"BB", toBoundingBox(fPieces[0]->fBB));
      }
      break;
    }
    case StructureType::Igloo:
      // Igloo bounding box is commonly misaligned in Java 1.12/LCE
      //
      // Mojang just deletes igloo bounding boxes in v1488 datafixer `IglooMetadataRemovalFix`: 
      // if all pieces are `Iglu` then delete `Children` and set `id: Igloo`
      // Upon load the structure data will be deleted and be replaced with `igloo: { id: "INVALID" }`
      // in vanilla 1.16.5 :shrug:
      out->erase(u8"Children");
      break;
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
  if (id == StructurePieceType::TeJP || id == StructurePieceType::Iglu
    || id == StructurePieceType::TeSH || id == StructurePieceType::TeDP) {
    // common ScatteredFeaturePiece fields
    i32 Width, Height, Depth;
    i32 HPos;
    if (!reader.read(&Width) || !reader.read(&Height) || !reader.read(&Depth) || !reader.read(&HPos)) {
      return nullptr;
    }

    // TODO: the Temple pieces have some bytes after HPos before their specific fields,
    // that I don't know how to parse. Figure out what rest of bytes mean.
    // Temples should only have 1 child, so we can skip parsing the rest and just set these bools
    // to true in the converter.
    switch (id) {
    case StructurePieceType::TeJP:
      // placedMainChest, placedHiddenChest, placedTrap1, placedTrap2 bools after unknown bytes
    case StructurePieceType::Iglu:
      // doesn't have any other fields in Java 1.12, but 1.13 has "Template" and "Rot" fields
      break;
    case StructurePieceType::TeSH:
      // bool Witch; // assume Witch has been spawned, don't know which byte this is
      // ignore rest of bytes, unknown & not needed in Java
      break;
    case StructurePieceType::TeDP:
      // hasPlacedChest0, hasPlacedChest1, hasPlacedChest2, hasPlacedChest3 bools after unknown bytes
      break;
    default: return nullptr;
    }
    return std::make_unique<TemplePiece>(pieceBB, O, GD, id, data, Width, Height, Depth, HPos);
  }

  switch (id) {
  case StructurePieceType::OMB: break;
  case StructurePieceType::TeJP: break;
  case StructurePieceType::Iglu: break;
  case StructurePieceType::TeSH: break;
  case StructurePieceType::TeDP: break;

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

  case StructurePieceType::SHCC:
  case StructurePieceType::SHPR:
  case StructurePieceType::SH5C:
  case StructurePieceType::SHLi:
  case StructurePieceType::SHFC:
  case StructurePieceType::SHRC:
  case StructurePieceType::SHS:
  case StructurePieceType::SHStart:
  case StructurePieceType::SHSD:
  case StructurePieceType::SHLT:
  case StructurePieceType::SHPH:
  case StructurePieceType::SHRT:
  case StructurePieceType::SHSSD: {
    i32 entryDoor;
    if (!reader.read(&entryDoor)) {
      return nullptr;
    }

    switch (id) {
    case StructurePieceType::SHCC: { // StrongholdChestCorridor
      u8 b;
      if (!reader.read(&b)) {
        return nullptr;
      }
      bool chest = static_cast<bool>(b);
      data->set(u8"Chest", Bool(chest));
      break;
    }
    case StructurePieceType::SHPR: { // StrongholdPortalRoom
      u8 b;
      if (!reader.read(&b)) {
        return nullptr;
      }
      bool mob = static_cast<bool>(b);
      data->set(u8"Mob", Bool(mob));
      break;
    }
    case StructurePieceType::SH5C: {
      // StrongholdFiveCrossing
      u8 b[4];
      for (int i = 0; i < 4; i++) {
        if (!reader.read(&b[i])) {
          return nullptr;
        }
      }
      data->set(u8"leftLow", Bool(b[0]));
      data->set(u8"leftHigh", Bool(b[1]));
      data->set(u8"rightLow", Bool(b[2]));
      data->set(u8"rightHigh", Bool(b[3]));
      break;
    }
    case StructurePieceType::SHLi: { // StrongholdLibrary
      u8 b;
      if (!reader.read(&b)) {
        return nullptr;
      }
      bool tall = static_cast<bool>(b);
      data->set(u8"Tall", Bool(tall));
      break;
    }
    case StructurePieceType::SHFC: { // StrongholdFillerCorridor
      i32 steps;
      if (!reader.read(&steps)) {
        return nullptr;
      }
      data->set(u8"Steps", Int(steps));
      break;
    }
    case StructurePieceType::SHRC: { // StrongholdRoomCrossing
      i32 type;
      if (!reader.read(&type)) {
        return nullptr;
      }
      data->set(u8"Type", Int(type));
      break;
    }
    case StructurePieceType::SHStart: // StrongholdStartPiece
      // fallthrough
    case StructurePieceType::SHSD: { // StrongholdStairsDown
      u8 b;
      if (!reader.read(&b)) {
        return nullptr;
      }
      bool source = static_cast<bool>(b);
      data->set(u8"Source", Bool(source));
      break;
    }
    case StructurePieceType::SHS: { // StrongholdStraight
      u8 b;
      bool left, right;
      if (!reader.read(&b)) {
        return nullptr;
      }
      data->set(u8"Left", Bool(static_cast<bool>(b)));
      if (!reader.read(&b)) {
        return nullptr;
      }
      data->set(u8"Right", Bool(static_cast<bool>(b)));
      break;
    }
    default: break;
    }
    return std::make_unique<StrongholdPiece>(pieceBB, O, GD, id, data, entryDoor);
  }
  }

  return std::make_unique<StructurePiece>(pieceBB, O, GD, id, data);
}

TemplePiece::TemplePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, CompoundTagPtr data, i32 width, i32 height, i32 depth, i32 hPos)
  : StructurePiece(bb, orientation, generationDepth, id, data), fWidth{width}, fHeight{height}, fDepth{depth}, fHPos{hPos} {}

StrongholdPiece::StrongholdPiece(
  Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id,
  CompoundTagPtr data, i32 entryDoor
) : StructurePiece(bb, orientation, generationDepth, id, data),
  fEntryDoor(entryDoor) {}

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

CompoundTagPtr TemplePiece::Convert() const {
  auto out = StructurePiece::Convert();

  out->set(u8"Width", Int(fWidth));
  out->set(u8"Height", Int(fHeight));
  out->set(u8"Depth", Int(fDepth));
  out->set(u8"HPos", Int(fHPos));

  switch (fId) {
  case StructurePieceType::TeJP: {
    // TODO: decode bool from save data
    out->set(u8"placedMainChest", Bool(true));
    out->set(u8"placedHiddenChest", Bool(true));
    out->set(u8"placedTrap1", Bool(true));
    out->set(u8"placedTrap2", Bool(true));
    break;
  }
  case StructurePieceType::Iglu:
    break;
  case StructurePieceType::TeSH:
    // TODO: decode bool from save data
    out->set(u8"Witch", Bool(true));
    // no cats in LCE
    out->set(u8"Cat", Bool(false));
    break;
  case StructurePieceType::TeDP:
    // TODO: decode bool from save data
    out->set(u8"hasPlacedChest0", Bool(true));
    out->set(u8"hasPlacedChest1", Bool(true));
    out->set(u8"hasPlacedChest2", Bool(true));
    out->set(u8"hasPlacedChest3", Bool(true));
    break;
  default:
    break;
  }
  return out;
}

CompoundTagPtr StrongholdPiece::Convert() const {
  auto out = StructurePiece::Convert();

  constexpr std::array<std::u8string_view, 4> entryDoors = {
    u8"OPENING",
    u8"WOOD_DOOR",
    u8"GRATES",
    u8"IRON_DOOR",
  };

  out->set(u8"EntryDoor", String(entryDoors[std::clamp(fEntryDoor, 0, 3)]));

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

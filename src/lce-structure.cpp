#include "je2be/nbt.hpp"
#include "lce/structure/_structure-piece.hpp"
#include "mcfile/encoding.hpp"
#include "mcfile/stream/byte-stream.hpp"
#include "mcfile/stream/input-stream-reader.hpp"
#include <memory>
#include <optional>

namespace je2be::lce {

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

StructureFeature::StructureFeature(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::vector<std::unique_ptr<StructurePiece>> pieces)
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fBoundingBox{std::move(bb)}, fPieces(std::move(pieces)) {}

StructureFeature::StructureFeature(StructureType type, i32 chunkX, i32 chunkZ, Volume bb, std::unique_ptr<StructurePiece> piece)
  : fType{type}, fChunkX{chunkX}, fChunkZ{chunkZ}, fBoundingBox{std::move(bb)}, fPieces() {
    fPieces.emplace_back(std::move(piece));
}

class StructureFeature::Impl {
  Impl() = delete;

public:

static std::optional<StructureFeature> Extract(std::span<const u8> bytes) {
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

  Volume featureBB{{0, 0, 0}, {0, 0, 0}};
  if (!readBB(reader, featureBB)) {
    return std::nullopt;
  }
  
  i32 childrenLen;
  if (!reader.read(&childrenLen) || childrenLen < 0 || childrenLen > 4096) { // arbitrary 4096 limit
    return std::nullopt;
  }
  
  std::vector<std::unique_ptr<StructurePiece>> pieces;
  
  for (size_t i = 0; i < childrenLen; i++) {
    auto piece = StructurePiece::ExtractPiece(reader);
    if (!piece) {
      std::cout << "error extracting structure piece in " << std::string(id.begin(), id.end()) << std::endl;
      // invalid piece, stop
      break;
    }
    pieces.push_back(std::move(piece));
  }
  if (pieces.size() == 0) {
    return std::nullopt;
  }
  
  if (pieces[0]->fId == StructurePieceType::OMB) {
    StructureFeature start{
      StructureType::OceanMonument, chunkX, chunkZ,
      featureBB, std::move(pieces),
    };
    // Ocean Monument has extra `Processed` bytes, if we can't read it then just skip them
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
    return start;
  }
  
  StructureType type;
  switch (pieces[0]->fId) {
  case StructurePieceType::TeJP: type = StructureType::JungleTemple; break;
  case StructurePieceType::Iglu: type = StructureType::Igloo; break;
  case StructurePieceType::TeSH: type = StructureType::SwampHut; break;
  case StructurePieceType::TeDP: type = StructureType::DesertPyramid; break;

  case StructurePieceType::NeBCr:
  case StructurePieceType::NeBEF:
  case StructurePieceType::NeBS:
  case StructurePieceType::NeCCS:
  case StructurePieceType::NeCTB:
  case StructurePieceType::NeCE:
  case StructurePieceType::NeSCSC:
  case StructurePieceType::NeSCLT:
  case StructurePieceType::NeSC:
  case StructurePieceType::NeSCRT:
  case StructurePieceType::NeCSR:
  case StructurePieceType::NeMT:
  case StructurePieceType::NeRC:
  case StructurePieceType::NeSR:
  case StructurePieceType::NeStart:
    type = StructureType::Fortress;
    break;
  case StructurePieceType::OMB:
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
  case StructurePieceType::SHSSD:
    type = StructureType::Stronghold;
    break;
  }

  StructureFeature start{type, chunkX, chunkZ, featureBB, std::move(pieces) };
  
  return start;
}

};

std::optional<StructureFeature> StructureFeature::Extract(std::span<const u8> bytes) {
  return Impl::Extract(bytes);
}

CompoundTagPtr StructureFeature::Convert() const {
  auto out = Compound();
  out->set(u8"ChunkX", Int(fChunkX));
  out->set(u8"ChunkZ", Int(fChunkZ));
  out->set(u8"id", String(NamespaceFeatureName(fType)));
  //out->set(u8"references", Int(0)); // not sure what this is used for
  auto start = fBoundingBox.fStart;
  auto end = fBoundingBox.fEnd;
  std::vector<i32> boundingBox{start.fX, start.fY, start.fZ, end.fX, end.fY, end.fZ};
  out->set(u8"BB", make_shared<IntArrayTag>(boundingBox));

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
      break;
    }
    case StructureType::DesertPyramid: break;
    case StructureType::Igloo:
      // Igloo bounding box is commonly misaligned in Java 1.12/LCE
      //
      // Mojang just deletes igloo bounding boxes in v1488 datafixer `IglooMetadataRemovalFix`: 
      // if all pieces are `Iglu` then delete `Children` and set `id: Igloo`
      // Upon load the structure data will be deleted and be replaced with `igloo: { id: "INVALID" }`
      // in vanilla 1.16.5 :shrug:
      out->erase(u8"Children");
      break;
    case StructureType::JungleTemple: break;
    case StructureType::Fortress: break;

    case StructureType::BuriedTreasure:
    case StructureType::EndCity:
    case StructureType::Mineshaft:
    case StructureType::Stronghold:
    case StructureType::Village:
    case StructureType::WoodlandMansion:
      // TODO
      break;
    }

  return out;
}

StructurePiece::StructurePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id) :
  fBB(bb), fOrientation{orientation}, fGenerationDepth{generationDepth}, fId(id) {}

class StructurePiece::Impl {
  Impl() = delete;

public:
  static std::unique_ptr<StructurePiece> ExtractPiece(mcfile::stream::InputStreamReader &reader) {
    std::u8string pieceId;
    if (!reader.read(pieceId)) {
      return nullptr;
    }

    // common StructurePiece fields
    Volume pieceBB{{0, 0, 0}, {0, 0, 0}};
    if (!readBB(reader, pieceBB)) {
      return nullptr;
    }
    i32 O; // orientation
    i32 GD; // generation depth
    if (!reader.read(&O) || !reader.read(&GD)) {
      return nullptr;
    }

    auto it = sPieceType.find(pieceId);
    if (it == sPieceType.end()) {
      return nullptr;
    }
    auto id = it->second;
    
    if (id == StructurePieceType::TeJP || id == StructurePieceType::Iglu
      || id == StructurePieceType::TeSH || id == StructurePieceType::TeDP) {
      // common ScatteredFeaturePiece fields
      i32 Width, Height, Depth;
      i32 HPos;
      if (!reader.read(&Width) || !reader.read(&Height) || !reader.read(&Depth) || !reader.read(&HPos)) {
        return nullptr;
      }

      switch (id) {
      case StructurePieceType::TeJP:
        // placedMainChest, placedHiddenChest, placedTrap1, placedTrap2 bools after unknown bytes
      case StructurePieceType::Iglu:
        // doesn't have any other fields in Java 1.12, but 1.13 has "Template" and "Rot" fields
        // TODO: figure out what rest of bytes mean
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
      return std::make_unique<TemplePiece>(pieceBB, O, GD, id, Width, Height, Depth, HPos);
    }

    switch (id) {
    case StructurePieceType::OMB: break;
    case StructurePieceType::TeJP: break;
    case StructurePieceType::Iglu: break;
    case StructurePieceType::TeSH: break;
    case StructurePieceType::TeDP: break;

    case StructurePieceType::NeMT: { // blaze spawner
      if (u8 b; reader.read(&b)) {
        bool mob = static_cast<bool>(b);
        return std::make_unique<FortressPiece>(pieceBB, O, GD, id, mob, std::nullopt, std::nullopt);
      }
      return nullptr;
    }
    case StructurePieceType::NeBEF: {
      if (i32 seed; reader.read(&seed)) {
        return std::make_unique<FortressPiece>(pieceBB, O, GD, id, std::nullopt, seed, std::nullopt);
      }
      return nullptr;
    }
    case StructurePieceType::NeSCLT: // fallthrough
    case StructurePieceType::NeSCRT: {
      if (u8 b; reader.read(&b)) {
        bool chest = static_cast<bool>(b);
        return std::make_unique<FortressPiece>(pieceBB, O, GD, id, std::nullopt, std::nullopt, chest);
      }
      return nullptr;
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
      return std::make_unique<FortressPiece>(pieceBB, O, GD, id, std::nullopt, std::nullopt, std::nullopt);

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
      auto data = Compound();

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
      return std::make_unique<StrongholdPiece>(pieceBB, O, GD, id, entryDoor, data);
    }
    }

    return std::make_unique<StructurePiece>(pieceBB, O, GD, id);

    return nullptr;
  }
};

std::unique_ptr<StructurePiece> StructurePiece::ExtractPiece(mcfile::stream::InputStreamReader &reader) {
  return Impl::ExtractPiece(reader);
}

TemplePiece::TemplePiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, i32 width, i32 height, i32 depth, i32 hPos)
  : StructurePiece(bb, orientation, generationDepth, id), fWidth{width}, fHeight{height}, fDepth{depth}, fHPos{hPos} {}

FortressPiece::FortressPiece(Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id, std::optional<bool> mob, std::optional<i32> seed, std::optional<bool> chest)
  : StructurePiece(bb, orientation, generationDepth, id), fMob{mob}, fSeed{seed}, fChest{chest} {}

StrongholdPiece::StrongholdPiece(
  Volume bb, i32 orientation, i32 generationDepth, StructurePieceType id,
  i32 entryDoor,
  CompoundTagPtr data
) : StructurePiece(bb, orientation, generationDepth, id),
  fEntryDoor(entryDoor),
  fData(data) {}

CompoundTagPtr StructurePiece::Convert() const {
  auto out = Compound();
  auto start = fBB.fStart;
  auto end = fBB.fEnd;
  // sometimes child bounding box can be outside structure bounding box (e.g. witch huts),
  // fixed at some point between Java 1.13 and 1.15
  std::vector<i32> boundingBox{start.fX, start.fY, start.fZ, end.fX, end.fY, end.fZ};
  out->set(u8"BB", make_shared<IntArrayTag>(boundingBox));
  out->set(u8"GD", Int(fGenerationDepth));
  out->set(u8"O", Int(fOrientation));
  out->set(u8"id", String(PieceId(fId)));
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

CompoundTagPtr FortressPiece::Convert() const {
  auto out = StructurePiece::Convert();

  switch (fId) {
  case StructurePieceType::NeMT: { // blaze spawner
    if (fMob.has_value()) {
      out->set(u8"Mob", Bool(fMob.value()));
    }
    break;
  }
  case StructurePieceType::NeBEF: { // bridge end filler
    if (fSeed.has_value()) {
      out->set(u8"Seed", Int(fSeed.value()));
    }
    break;
  }
  case StructurePieceType::NeSCLT:
    // fallthrough
  case StructurePieceType::NeSCRT: {
    if (fChest.has_value()) {
      out->set(u8"Chest", Bool(fChest.value()));
    }
    break;
  }
  default: break;
  }
  return out;
}

CompoundTagPtr StrongholdPiece::Convert() const {
  auto out = StructurePiece::Convert();

  constexpr std::u8string_view entryDoors[] = {
    u8"OPENING",
    u8"WOOD_DOOR",
    u8"GRATES",
    u8"IRON_DOOR",
  };
  out->set(u8"EntryDoor", String(entryDoors[fEntryDoor >= 0 && fEntryDoor <= 3 ? fEntryDoor : 0]));
  for (auto &t : *fData) {
    out->set(t.first, t.second);
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

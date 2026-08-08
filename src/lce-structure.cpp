#include "je2be/nbt.hpp"
#include "lce/structure/_structure-piece.hpp"
#include <optional>

namespace je2be::lce {

static i32 readI32BE(std::span<const unsigned char> bytes, size_t *off) {
  assert(*off + 4 <= bytes.size());
  i32 value = static_cast<i32>(
      static_cast<u32>(bytes[*off]) << 24 |
      static_cast<u32>(bytes[*off + 1]) << 16 |
      static_cast<u32>(bytes[*off + 2]) << 8 |
      static_cast<u32>(bytes[*off + 3])
  );
  *off += 4;
  return value;
}

static Volume readBB(std::span<const unsigned char> bytes, size_t *off) {
  std::array<i32, 6> result{};
  for (i32 &i : result) {
    i = readI32BE(bytes, off);
  }
  return Volume{
    Pos3i{result[0], result[1], result[2]},
    Pos3i{result[3], result[4], result[5]}
  };
}

class StructureFeature::Impl {
  Impl() = delete;

public:

static std::optional<StructureFeature> Extract(std::span<const unsigned char> bytes) {
  size_t off = 0;
  // 4 byte unknown header, 4 for 1.83, 3 for 1.69, 1.45, 1 for 1.21, 1.31, 0 for 1.15
  off += 4;

  if (off + 2 > bytes.size()) {
    return std::nullopt;
  }
  u16 idLen = (static_cast<u16>(bytes[off]) << 8) | static_cast<u16>(bytes[off + 1]);
  off += 2;
  
  if (off + idLen > bytes.size()) {
    return std::nullopt;
  }
  std::u8string id{bytes.begin() + off, bytes.begin() + off + idLen};
  off += idLen;
  
  i32 chunkX = readI32BE(bytes, &off);
  i32 chunkZ = readI32BE(bytes, &off);

  Volume featureBB = readBB(bytes, &off);
  
  i32 childrenLen = readI32BE(bytes, &off);
  if (childrenLen < 0) {
    return std::nullopt;
  }
  
  std::vector<std::unique_ptr<StructurePiece>> pieces;
  
  for (size_t i = 0; i < childrenLen; i++) {
    auto piece = StructurePiece::ExtractPiece(bytes, off);
    if (!piece) {
      // invalid piece, stop
      break;
    }
    pieces.push_back(std::move(piece));
  }
  
  if (pieces[0]->fId == StructurePieceType::OMB) {
    StructureFeature start{
      StructureType::OceanMonument, chunkX, chunkZ,
      featureBB, std::move(pieces),
    };
    // Ocean Monument has extra `Processed` bytes
    i32 processedLen = readI32BE(bytes, &off);
    for (size_t i = 0; i < processedLen; i++) {
      i32 x = readI32BE(bytes, &off);
      i32 z = readI32BE(bytes, &off);
      start.fProcessed.emplace_back(x, z);
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
  default:
    return std::nullopt;
  }

  StructureFeature start{type, chunkX, chunkZ, featureBB, std::move(pieces) };
  
  return start;
}

};

std::optional<StructureFeature> StructureFeature::Extract(std::span<const unsigned char> bytes) {
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
    case StructureType::Fortress: {
      break;
    }
    case StructureType::SwampHut: {
      // note: witch hut bounding box is 7x7x9 in 1.8.1+
      // LCE usually has correct size, sometimes broken 8x7x10 boxes idk?
      break;
    }
    case StructureType::BuriedTreasure:
    case StructureType::DesertPyramid:
    case StructureType::EndCity:
    case StructureType::Igloo:
      // Igloo bounding box is commonly misaligned in Java 1.12/LCE
      //
      // Mojang just deletes igloo bounding boxes in v1488 datafixer `IglooMetadataRemovalFix`: 
      // if all pieces are `Iglu` then delete `Children` and set `id: Igloo`
      // Upon load the structure data will be deleted and be replaced with `igloo: { id: "INVALID" }`
      // in vanilla 1.16.5 :shrug:
      out->erase(u8"Children");
      break;
    case StructureType::JungleTemple:
    case StructureType::Mineshaft:
    case StructureType::Stronghold:
    case StructureType::Village:
    case StructureType::WoodlandMansion:
      // TODO
      break;
    }

  return out;
}

class StructurePiece::Impl {
  Impl() = delete;

public:
  static std::unique_ptr<StructurePiece> ExtractPiece(std::span<const unsigned char> bytes, size_t &off) {
    if (off + 2 > bytes.size()) {
      return nullptr;
    }
    u16 pieceIdLen = (static_cast<u16>(bytes[off]) << 8) | static_cast<u16>(bytes[off + 1]);
    off += 2;

    if (off + pieceIdLen > bytes.size()) {
      return nullptr;
    }
    std::u8string pieceId(bytes.begin() + off, bytes.begin() + off + pieceIdLen);
    off += pieceIdLen;

    // common StructurePiece fields
    Volume pieceBB = readBB(bytes, &off);
    i32 O = readI32BE(bytes, &off); // orientation
    i32 GD = readI32BE(bytes, &off); // generation depth

    auto it = sPieceType.find(pieceId);
    if (it == sPieceType.end()) {
      return nullptr;
    }
    auto id = it->second;
    
    if (id == StructurePieceType::TeJP || id == StructurePieceType::Iglu
      || id == StructurePieceType::TeSH || id == StructurePieceType::TeDP) {
      // common ScatteredFeaturePiece fields
      i32 Width = readI32BE(bytes, &off);
      i32 Height = readI32BE(bytes, &off);
      i32 Depth = readI32BE(bytes, &off);
      i32 HPos = readI32BE(bytes, &off); // y level of surface the structure was moved to, or -1 if not moved
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
      if (off >= bytes.size()) {
        return nullptr;
      }
      bool mob = static_cast<bool>(bytes[off]);
      off += 1;
      return std::make_unique<FortressPiece>(pieceBB, O, GD, id, mob, std::nullopt, std::nullopt);
    }
    case StructurePieceType::NeBEF: {
      i32 seed = readI32BE(bytes, &off);
      return std::make_unique<FortressPiece>(pieceBB, O, GD, id, std::nullopt, seed, std::nullopt);
    }
    case StructurePieceType::NeSCLT: // fallthrough
    case StructurePieceType::NeSCRT: {
      bool chest = static_cast<bool>(bytes[off]);
      off += 1;
      return std::make_unique<FortressPiece>(pieceBB, O, GD, id, std::nullopt, std::nullopt, chest);
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
    }

    return std::make_unique<StructurePiece>(pieceBB, O, GD, id);

    return nullptr;
  }
};

std::unique_ptr<StructurePiece> StructurePiece::ExtractPiece(std::span<const unsigned char> bytes, size_t &off) {
  return Impl::ExtractPiece(bytes, off);
}

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
  //// IdentifyFacingOfOceanMonument https://github.com/kbinani/je2be/commit/6ca28383bc557bcf60b8203e655fdbb7a87d39d7
  //// O=0: north
  //// O=1: east
  //// O=3: west
  out->set(u8"id", String(PieceId(fId)));
  return out;
}

CompoundTagPtr TemplePiece::Convert() const {
  auto out = StructurePiece::Convert();

  out->set(u8"Width", Int(fWidth));
  out->set(u8"Height", Int(fHeight));
  out->set(u8"Depth", Int(fDepth));
  out->set(u8"HPos", Int(fHPos));
  out->set(u8"id", String(PieceId(fId)));

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

  out->set(u8"id", String(PieceId(fId)));

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

/*
public interface StructurePieceType {
	StructurePieceType MINE_SHAFT_CORRIDOR = setPieceId(MineShaftPieces.MineShaftCorridor::new, "MSCorridor");
	StructurePieceType MINE_SHAFT_CROSSING = setPieceId(MineShaftPieces.MineShaftCrossing::new, "MSCrossing");
	StructurePieceType MINE_SHAFT_ROOM = setPieceId(MineShaftPieces.MineShaftRoom::new, "MSRoom");
	StructurePieceType MINE_SHAFT_STAIRS = setPieceId(MineShaftPieces.MineShaftStairs::new, "MSStairs");
	StructurePieceType NETHER_FORTRESS_BRIDGE_CROSSING = setPieceId(NetherBridgePieces.BridgeCrossing::new, "NeBCr");
	StructurePieceType NETHER_FORTRESS_BRIDGE_END_FILLER = setPieceId(NetherBridgePieces.BridgeEndFiller::new, "NeBEF");
	StructurePieceType NETHER_FORTRESS_BRIDGE_STRAIGHT = setPieceId(NetherBridgePieces.BridgeStraight::new, "NeBS");
	StructurePieceType NETHER_FORTRESS_CASTLE_CORRIDOR_STAIRS = setPieceId(NetherBridgePieces.CastleCorridorStairsPiece::new, "NeCCS");
	StructurePieceType NETHER_FORTRESS_CASTLE_CORRIDOR_T_BALCONY = setPieceId(NetherBridgePieces.CastleCorridorTBalconyPiece::new, "NeCTB");
	StructurePieceType NETHER_FORTRESS_CASTLE_ENTRANCE = setPieceId(NetherBridgePieces.CastleEntrance::new, "NeCE");
	StructurePieceType NETHER_FORTRESS_CASTLE_SMALL_CORRIDOR_CROSSING = setPieceId(NetherBridgePieces.CastleSmallCorridorCrossingPiece::new, "NeSCSC");
	StructurePieceType NETHER_FORTRESS_CASTLE_SMALL_CORRIDOR_LEFT_TURN = setPieceId(NetherBridgePieces.CastleSmallCorridorLeftTurnPiece::new, "NeSCLT");
	StructurePieceType NETHER_FORTRESS_CASTLE_SMALL_CORRIDOR = setPieceId(NetherBridgePieces.CastleSmallCorridorPiece::new, "NeSC");
	StructurePieceType NETHER_FORTRESS_CASTLE_SMALL_CORRIDOR_RIGHT_TURN = setPieceId(NetherBridgePieces.CastleSmallCorridorRightTurnPiece::new, "NeSCRT");
	StructurePieceType NETHER_FORTRESS_CASTLE_STALK_ROOM = setPieceId(NetherBridgePieces.CastleStalkRoom::new, "NeCSR");
	StructurePieceType NETHER_FORTRESS_MONSTER_THRONE = setPieceId(NetherBridgePieces.MonsterThrone::new, "NeMT");
	StructurePieceType NETHER_FORTRESS_ROOM_CROSSING = setPieceId(NetherBridgePieces.RoomCrossing::new, "NeRC");
	StructurePieceType NETHER_FORTRESS_STAIRS_ROOM = setPieceId(NetherBridgePieces.StairsRoom::new, "NeSR");
	StructurePieceType NETHER_FORTRESS_START = setPieceId(NetherBridgePieces.StartPiece::new, "NeStart");
	StructurePieceType STRONGHOLD_CHEST_CORRIDOR = setPieceId(StrongholdPieces.ChestCorridor::new, "SHCC");
	StructurePieceType STRONGHOLD_FILLER_CORRIDOR = setPieceId(StrongholdPieces.FillerCorridor::new, "SHFC");
	StructurePieceType STRONGHOLD_FIVE_CROSSING = setPieceId(StrongholdPieces.FiveCrossing::new, "SH5C");
	StructurePieceType STRONGHOLD_LEFT_TURN = setPieceId(StrongholdPieces.LeftTurn::new, "SHLT");
	StructurePieceType STRONGHOLD_LIBRARY = setPieceId(StrongholdPieces.Library::new, "SHLi");
	StructurePieceType STRONGHOLD_PORTAL_ROOM = setPieceId(StrongholdPieces.PortalRoom::new, "SHPR");
	StructurePieceType STRONGHOLD_PRISON_HALL = setPieceId(StrongholdPieces.PrisonHall::new, "SHPH");
	StructurePieceType STRONGHOLD_RIGHT_TURN = setPieceId(StrongholdPieces.RightTurn::new, "SHRT");
	StructurePieceType STRONGHOLD_ROOM_CROSSING = setPieceId(StrongholdPieces.RoomCrossing::new, "SHRC");
	StructurePieceType STRONGHOLD_STAIRS_DOWN = setPieceId(StrongholdPieces.StairsDown::new, "SHSD");
	StructurePieceType STRONGHOLD_START = setPieceId(StrongholdPieces.StartPiece::new, "SHStart");
	StructurePieceType STRONGHOLD_STRAIGHT = setPieceId(StrongholdPieces.Straight::new, "SHS");
	StructurePieceType STRONGHOLD_STRAIGHT_STAIRS_DOWN = setPieceId(StrongholdPieces.StraightStairsDown::new, "SHSSD");
	StructurePieceType JUNGLE_PYRAMID_PIECE = setPieceId(JunglePyramidPiece::new, "TeJP");
	StructurePieceType OCEAN_RUIN = setPieceId(OceanRuinPieces.OceanRuinPiece::new, "ORP");
	StructurePieceType IGLOO = setPieceId(IglooPieces.IglooPiece::new, "Iglu");
	StructurePieceType RUINED_PORTAL = setPieceId(RuinedPortalPiece::new, "RUPO");
	StructurePieceType SWAMPLAND_HUT = setPieceId(SwamplandHutPiece::new, "TeSH");
	StructurePieceType DESERT_PYRAMID_PIECE = setPieceId(DesertPyramidPiece::new, "TeDP");
	StructurePieceType OCEAN_MONUMENT_BUILDING = setPieceId(OceanMonumentPieces.MonumentBuilding::new, "OMB");
	StructurePieceType OCEAN_MONUMENT_CORE_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentCoreRoom::new, "OMCR");
	StructurePieceType OCEAN_MONUMENT_DOUBLE_X_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentDoubleXRoom::new, "OMDXR");
	StructurePieceType OCEAN_MONUMENT_DOUBLE_XY_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentDoubleXYRoom::new, "OMDXYR");
	StructurePieceType OCEAN_MONUMENT_DOUBLE_Y_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentDoubleYRoom::new, "OMDYR");
	StructurePieceType OCEAN_MONUMENT_DOUBLE_YZ_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentDoubleYZRoom::new, "OMDYZR");
	StructurePieceType OCEAN_MONUMENT_DOUBLE_Z_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentDoubleZRoom::new, "OMDZR");
	StructurePieceType OCEAN_MONUMENT_ENTRY_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentEntryRoom::new, "OMEntry");
	StructurePieceType OCEAN_MONUMENT_PENTHOUSE = setPieceId(OceanMonumentPieces.OceanMonumentPenthouse::new, "OMPenthouse");
	StructurePieceType OCEAN_MONUMENT_SIMPLE_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentSimpleRoom::new, "OMSimple");
	StructurePieceType OCEAN_MONUMENT_SIMPLE_TOP_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentSimpleTopRoom::new, "OMSimpleT");
	StructurePieceType OCEAN_MONUMENT_WING_ROOM = setPieceId(OceanMonumentPieces.OceanMonumentWingRoom::new, "OMWR");
	StructurePieceType END_CITY_PIECE = setPieceId(EndCityPieces.EndCityPiece::new, "ECP");
	StructurePieceType WOODLAND_MANSION_PIECE = setPieceId(WoodlandMansionPieces.WoodlandMansionPiece::new, "WMP");
	StructurePieceType BURIED_TREASURE_PIECE = setPieceId(BuriedTreasurePieces.BuriedTreasurePiece::new, "BTP");
	StructurePieceType SHIPWRECK_PIECE = setPieceId(ShipwreckPieces.ShipwreckPiece::new, "Shipwreck");
	StructurePieceType NETHER_FOSSIL = setPieceId(NetherFossilPieces.NetherFossilPiece::new, "NeFos");
	StructurePieceType JIGSAW = setPieceId(PoolElementStructurePiece::new, "jigsaw");

	StructurePiece load(StructureManager structureManager, CompoundTag compoundTag);

	static StructurePieceType setPieceId(StructurePieceType structurePieceType, String string) {
		return Registry.register(Registry.STRUCTURE_PIECE, string.toLowerCase(Locale.ROOT), structurePieceType);
	}
  */


} // namespace je2be::lce

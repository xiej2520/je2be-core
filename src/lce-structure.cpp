#include "lce/structure/_structure-piece.hpp"

namespace je2be::lce {

class StructureFeature::Impl {
  Impl() = delete;

public:
};

CompoundTagPtr StructureFeature::Convert() const {
  auto out = Compound();
  switch (fType) {
  case StructureType::OceanMonument: {
      out->set(u8"ChunkX", Int(fChunkX));
      out->set(u8"ChunkZ", Int(fChunkZ));
      out->set(u8"id", String(NamespaceFeatureName(StructureType::OceanMonument)));
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
    case StructureType::Fortress:
      break;
    case StructureType::SwampHut: {
      out->set(u8"ChunkX", Int(fChunkX));
      out->set(u8"ChunkZ", Int(fChunkZ));
      out->set(u8"id", String(NamespaceFeatureName(StructureType::SwampHut)));
      //out->set(u8"references", Int(0)); // not sure what this is used for

      auto start = fBoundingBox.fStart;
      auto end = fBoundingBox.fEnd;
      std::vector<i32> boundingBox{start.fX, start.fY, start.fZ, end.fX, end.fY, end.fZ};
      out->set(u8"BB", make_shared<IntArrayTag>(boundingBox));
      // note: witch hut bounding box is 7x7x9 in 1.8.1+
      // LCE usually has correct size, sometimes broken 8x7x10 boxes idk?
      
      auto children = List<Tag::Type::Compound>();
      for (auto const &piece : fPieces) {
        children->push_back(piece->Convert());
      }
      out->set(u8"Children", children);

      break;
    }
    case StructureType::BuriedTreasure:
    case StructureType::DesertPyramid:
    case StructureType::EndCity:
    case StructureType::Igloo:
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
};

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
  auto out = Compound();
  auto start = fBB.fStart;
  auto end = fBB.fEnd;
  // sometimes child bounding box can be outside structure bounding box (e.g. witch huts),
  // fixed at some point between Java 1.13 and 1.15
  std::vector<i32> boundingBox{start.fX, start.fY, start.fZ, end.fX, end.fY, end.fZ};
  out->set(u8"BB", make_shared<IntArrayTag>(boundingBox));
  out->set(u8"GD", Int(fGenerationDepth));
  out->set(u8"O", Int(fOrientation));
  out->set(u8"Width", Int(fWidth));
  out->set(u8"Height", Int(fHeight));
  out->set(u8"Depth", Int(fDepth));
  out->set(u8"HPos", Int(fHPos));
  out->set(u8"id", String(PieceId(fId)));

  if (fId == StructurePieceType::TeSH) {
    // TODO: decode bool from save data
    out->set(u8"Witch", Bool(true));
    // no cats in LCE
    out->set(u8"Cat", Bool(false));
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

#pragma once

#include <je2be/pos2.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include "_pos3.hpp"
#include "je2be/strings.hpp"
#include "_structure-piece.hpp"
#include "_volume.hpp"

namespace je2be::lce {

// Reference 1.16.5 LegacyStructureDataHandler, 26.1+ LegacyStructureFileFix
class LegacyStructures {
public:
  void add(StructureFeature p, mcfile::Dimension dim) {
    switch (dim) {
    case mcfile::Dimension::Overworld:
      fOverworld.push_back(std::move(p));
      break;
    case mcfile::Dimension::Nether:
      fNether.push_back(std::move(p));
      break;
    case mcfile::Dimension::End:
      fEnd.push_back(std::move(p));
      break;
    }
  }

  std::vector<const StructureFeature *> nearbyStarts(mcfile::Dimension d, Pos2i chunk) const {
    const std::vector<StructureFeature> *structures = nullptr;
    switch (d) {
    case mcfile::Dimension::Overworld:
      structures = &fOverworld;
      break;
    case mcfile::Dimension::Nether:
      structures = &fNether;
      break;
    case mcfile::Dimension::End:
      structures = &fEnd;
      break;
    default:
      return {};
    }

    std::vector<const StructureFeature *> out;
    for (auto const &s : *structures) {
      // Vanilla LegacyStructureDataHandler: if chunk is within 8 of structure start (inclusive), add reference
      if (std::abs(chunk.fX - s.fChunkX) <= 8 && std::abs(chunk.fZ - s.fChunkZ) <= 8) {
        out.push_back(&s);
      }
    }
    return out;
  }
  
  void decodeFortress(CompoundTag const &in) {
    auto data = in.compoundTag(u8"data");
    //std::cout << "fortress data" << data->toSnbt({}) << std::endl;
    if (!data) {
      return;
    }
    auto features = data->compoundTag(u8"Features");
    //std::cout << "FORTRESS FEATURES " << features->toSnbt({}) << std::endl;
  }

  void decodeMonument(CompoundTag const &in) {
    auto data = in.compoundTag(u8"data");
    if (!data) {
      return;
    }
    auto features = data->compoundTag(u8"Features");
    if (!features) {
      return;
    }

    for (auto const &[key, value] : *features) {
      auto ba = value->asByteArray();
      if (!ba) {
        continue;
      }
      auto const &bytes = ba->value();
      auto coords = ParseChunkCoords(key);
      if (!coords) {
        continue;
      }
      auto const [_x, _z] = *coords;
      // ignore chunk coords key, they are unused in Java as well. Use ChunkX, ChunkZ in tag.
      
      // byte array containing "Monument" feature
      size_t off = 0;
      // 4 byte unknown header, 4 for TU1.83, 3 for TU1.69
      off += 4;

      if (off + 2 > bytes.size()) {
        continue;
      }
      u16 idLen = (static_cast<u16>(bytes[off]) << 8) | static_cast<u16>(bytes[off + 1]);
      off += 2;
      
      if (off + idLen > bytes.size()) {
        continue;
      }
      std::u8string id{bytes.begin() + off, bytes.begin() + off + idLen};
      off += idLen;
      if (id != u8"Monument") {
        continue;
      }
      
      i32 chunkX = readI32BE(bytes, &off);
      i32 chunkZ = readI32BE(bytes, &off);

      Volume featureBB = readBB(bytes, &off);
      
      i32 childrenLen = readI32BE(bytes, &off);
      // expect one OMB piece for ocean monument
      if (childrenLen != 1) {
        continue;
      }
      
      if (off + 2 > bytes.size()) {
        break;
      }
      u16 childIdLen = (static_cast<u16>(bytes[off]) << 8) | static_cast<u16>(bytes[off + 1]);
      off += 2;
      if (off + childIdLen > bytes.size()) {
        break;
      }
      std::u8string childId(bytes.begin() + off, bytes.begin() + off + childIdLen);
      off += childIdLen;
      if (childId != u8"OMB") {
        // unknown piece type, do not try to read more
        continue;
      }
      Volume pieceBB = readBB(bytes, &off);
      i32 O = readI32BE(bytes, &off); // orientation
      i32 GD = readI32BE(bytes, &off); // generation depth
      // OMB child end
      StructureFeature start{
        StructureType::OceanMonument, chunkX, chunkZ,
        featureBB,
        std::make_unique<StructurePiece>(pieceBB, O, GD, StructurePieceType::OMB),
      };

      i32 processedLen = readI32BE(bytes, &off);
      for (size_t i = 0; i < processedLen; i++) {
        i32 x = readI32BE(bytes, &off);
        i32 z = readI32BE(bytes, &off);
        start.fProcessed.emplace_back(x, z);
      }

      std::cout << start << std::endl;
      // ignore rest of bytes, unknown & not needed in Java
      this->add(std::move(start), mcfile::Dimension::Overworld);
      
      for (int i = 0; i < bytes.size(); i++) {
        char c = bytes[i];
        if (c != 0) {
          //std::cout << std::hex << "0x" << i << ": " << c << " " << (int) c << std::dec << std::endl;
        }
      }
      for (auto const &b : bytes) {
        std::cout << (int) b << " ";
      }
      for (auto const &b : bytes) {
        std::cout << b << " ";
      }
      std::cout << std::endl;
    }
  }

  void decodeTemple(CompoundTag const &in) {
    auto data = in.compoundTag(u8"data");
    if (!data) {
      return;
    }
    auto features = data->compoundTag(u8"Features");
    if (!features) {
      return;
    }
    for (auto const &[key, value] : *features) {
      auto ba = value->asByteArray();
      if (!ba) {
        continue;
      }
      auto const &bytes = ba->value();
      auto coords = ParseChunkCoords(key);
      if (!coords) {
        continue;
      }
      auto const [_x, _z] = *coords;
      // ignore chunk coords key, they are unused in Java as well. Use ChunkX, ChunkZ in tag.
      
      // byte array containing "Temple" feature
      size_t off = 0;
      // 4 byte unknown header, 4 for 1.83, 3 for 1.69, 1.45, 1 for 1.21, 1.31, 0 for 1.15
      off += 4;

      if (off + 2 > bytes.size()) {
        continue;
      }
      u16 idLen = (static_cast<u16>(bytes[off]) << 8) | static_cast<u16>(bytes[off + 1]);
      off += 2;
      
      if (off + idLen > bytes.size()) {
        continue;
      }
      std::u8string id{bytes.begin() + off, bytes.begin() + off + idLen};
      off += idLen;
      if (id != u8"Temple") {
        continue;
      }
      
      i32 chunkX = readI32BE(bytes, &off);
      i32 chunkZ = readI32BE(bytes, &off);

      Volume featureBB = readBB(bytes, &off);
      // TODO: check "Patch 1.25 Fix for MCCE #1756 - Witch Hut Bounding Box Too Small."
      
      i32 childrenLen = readI32BE(bytes, &off);
      if (childrenLen < 0) {
        continue;
      }
      
      for (size_t i = 0; i < childrenLen; i++) {
        if (off + 2 > bytes.size()) {
          break;
        }
        u16 childIdLen = (static_cast<u16>(bytes[off]) << 8) | static_cast<u16>(bytes[off + 1]);
        off += 2;
        
        if (off + childIdLen > bytes.size()) {
          break;
        }
        std::u8string childId(bytes.begin() + off, bytes.begin() + off + childIdLen);
        off += childIdLen;

        if (childId == u8"TeSH") { // swamp hut (witch hut)
          Volume pieceBB = readBB(bytes, &off);
          i32 O = readI32BE(bytes, &off); // orientation
          i32 GD = readI32BE(bytes, &off); // generation depth
          i32 Width = readI32BE(bytes, &off);
          i32 Height = readI32BE(bytes, &off);
          i32 Depth = readI32BE(bytes, &off);
          i32 HPos = readI32BE(bytes, &off); // y level of surface the structure was moved to, or -1 if not moved
          // bool Witch; // assume Witch has been spawned, don't know which byte this is

          StructureFeature start{
            StructureType::SwampHut, chunkX, chunkZ,
            featureBB,
            std::move(std::make_unique<TemplePiece>(pieceBB, O, GD, StructurePieceType::TeSH, Width, Height, Depth, HPos)),
          };
          std::cout << start << std::endl;

          // ignore rest of bytes, unknown & not needed in Java
          
          this->add(std::move(start), mcfile::Dimension::Overworld);

        } else if (childId == u8"Iglu") { // igloo

        } else if (childId == u8"TeDP") { // desert pyramid (temple)

        } else if (childId == u8"TeJP") { // jungle pyramid (temple)

        } else {
          // unknown piece type, do not try to read more
          break;
        }
        
      }
      
      
      std::cout << "idLen " << idLen << " id: " << std::string{id.begin(), id.end()} << std::endl;
      
      for (int i = 0; i < bytes.size(); i++) {
        char c = bytes[i];
        if (c != 0) {
          //std::cout << std::hex << "0x" << i << ": " << c << " " << (int) c << std::dec << std::endl;
        }
      }
      for (auto const &b : bytes) {
        std::cout << (int) b << " ";
      }
      for (auto const &b : bytes) {
        std::cout << b << " ";
      }
      std::cout << std::endl;
    }
    //std::cout << "TEMPLE FEATURES " << features->toSnbt({}) << std::endl;
  }

  // ChunkPos.asLong(cx, cz), avoid strict aliasing UB
  static i64 PackStructureStartsReference(i32 cx, i32 cz) {
    return static_cast<i64>((static_cast<u64>(static_cast<u32>(cz)) << 32) | static_cast<u32>(cx));
  }

private:
  std::vector<StructureFeature> fOverworld;
  std::vector<StructureFeature> fNether;
  std::vector<StructureFeature> fEnd;

  static std::optional<std::pair<i32, i32>> ParseChunkCoords(std::u8string_view key) {
    if (!key.starts_with(u8"[") || !key.ends_with(u8"]")) {
      return std::nullopt;
    }

    auto split = mcfile::String::Split(strings::RemovePrefixAndSuffix(u8"[", key, u8"]"), u8',');
    if (split.size() != 2) {
      return std::nullopt;
    }

    auto x = strings::ToI32(split[0]);
    auto z = strings::ToI32(split[1]);
    if (!x || !z) {
      return std::nullopt;
    }
    
    return std::make_pair(*x, *z);
  }
  
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
};

} // namespace je2be::lce

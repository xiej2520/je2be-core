#pragma once

#include <je2be/pos2.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include "_pos3.hpp"
#include "je2be/strings.hpp"
#include "_mem.hpp"
#include "mcfile/endianness.hpp"
#include "_structure-piece.hpp"
#include "_volume.hpp"

namespace je2be::lce {

// Reference 1.16.5 LegacyStructureDataHandler, 26.1+ LegacyStructureFileFix
class LegacyStructures {
public:
  void add(StructureFeature p, Pos2i startChunk, mcfile::Dimension dim) {
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

  std::vector<StructureFeature> nearbyStarts(mcfile::Dimension d, Pos2i chunk) const {
    const std::vector<StructureFeature> *structures;
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
    }

    std::vector<StructureFeature> out;
    for (auto s : *structures) {
      // Vanilla LegacyStructureDataHandler: if chunk is within 8 of structure start (inclusive), add reference
      if (std::abs(chunk.fX - s.fChunkX) <= 8 && std::abs(chunk.fZ - s.fChunkZ) <= 8) {
        out.push_back(s);
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
    //std::cout << "monument data" << data->toSnbt({}) << std::endl;
    if (!data) {
      return;
    }
    auto features = data->compoundTag(u8"Features");
    //std::cout << "MONUMENT FEATURES " << features << std::endl;
  }

  void decodeTemple(CompoundTag const &in) {
    auto data = in.compoundTag(u8"data");
    //std::cout << "temple data" << data->toSnbt({}) << std::endl;
    if (!data) {
      return;
    }
    auto features = data->compoundTag(u8"Features");
    for (auto const &[key, value] : *features) {
      auto bytes = value->asByteArray()->value();
      auto coords = ParseChunkCoords(key);
      if (!coords) {
        continue;
      }
      auto const [x, z] = *coords;
      std::cout << "x: " << x << " z: " << z << std::endl;
      
      // byte array containing "Temple" feature
      size_t off = 0;
      // 4 byte unknown header
      off += 4;

      u16 idLen = (bytes.at(off) << 8) | bytes.at(off + 1);
      off += 2;
      std::u8string id(&bytes.at(off), &bytes.at(off + idLen));
      off += idLen;
      if (id != u8"Temple") {
        continue;
      }
      
      i32 chunkX = readI32BE(bytes, off);
      off += 4;
      i32 chunkZ = readI32BE(bytes, off);
      off += 4;

      std::array<i32, 6> featureBB = readBB(bytes, off);
      off += sizeof(i32) * 6;
      
      i32 childrenLen = readI32BE(bytes, off);
      off += 4;
      
      for (int i = 0; i < childrenLen; i++) {
        u16 idLen = (bytes.at(off) << 8) | bytes.at(off + 1);
        off += 2;
        std::u8string id(&bytes.at(off), &bytes.at(off + idLen));
        off += idLen;

        if (id == u8"TeSH") { // swamp hut (witch hut)
          std::array<i32, 6> pieceBB = readBB(bytes, off);
          off += sizeof(i32) * 6;
          
          int O = readI32BE(bytes, off); // orientation
          O = mcfile::I32FromBE(Mem::Read<i32>(bytes, off));
          off += 4;
          int GD = readI32BE(bytes, off); // generation depth
          off += 4;
          int Width = readI32BE(bytes, off);
          Width = mcfile::I32FromBE(Mem::Read<i32>(bytes, off));
          off += 4;
          int Height = readI32BE(bytes, off);
          Height = mcfile::I32FromBE(Mem::Read<i32>(bytes, off));
          off += 4;
          int Depth = readI32BE(bytes, off);
          off += 4;
          int HPos = readI32BE(bytes, off); // y level of surface the structure was moved to, or -1 if not moved
          off += 4;

          // bool Witch; // assume Witch has been spawned, don't know which byte this is
          StructureFeature start{
            StructureType::SwampHut, chunkX, chunkZ,
            Volume{Pos3i{featureBB[0], featureBB[1], featureBB[2]}, Pos3i{featureBB[3], featureBB[4], featureBB[5]}},
            {StructurePiece{Volume{Pos3i{pieceBB[0], pieceBB[1], pieceBB[2]}, Pos3i{pieceBB[3], pieceBB[4], pieceBB[5]}}, O, GD, Width, Height, Depth, HPos}}
          };
          std::cout << start << std::endl;

          // ignore rest of array, unknown & not needed in Java
          
          this->add(start, Pos2i{x, z}, mcfile::Dimension::Overworld);

        } else if (id == u8"Iglu") { // igloo

        } else if (id == u8"TeDP") { // desert pyramid (temple)

        } else if (id == u8"TeJP") { // jungle pyramid (temple)

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
  
  static i32 readI32BE(std::span<unsigned char> bytes, size_t off) {
    return (bytes[off] << 24) | (bytes[off + 1] << 16) | (bytes[off + 2] << 8) | bytes[off + 3];
  }

  static std::array<i32, 6> readBB(std::span<unsigned char> bytes, size_t off) {
    std::array<i32, 6> result{};
    for (int i = 0; i < 6; i++) {
      result[i] = readI32BE(bytes, off + i * 4);
    }
    return result;
  }
};

} // namespace je2be::java

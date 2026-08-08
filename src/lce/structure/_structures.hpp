#pragma once

#include <je2be/pos2.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include "_pos3.hpp"
#include "je2be/strings.hpp"
#include "_structure-piece.hpp"
#include "_volume.hpp"
#include "mcfile/dimension.hpp"

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

  void ExtractStructures(CompoundTag const &in, mcfile::Dimension dim) {
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
      
      auto start = StructureFeature::Extract(bytes);
      if (start.has_value()) {
        std::cout << start.value() << std::endl;

        this->add(std::move(start).value(), dim);
      }
    }
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
};

} // namespace je2be::lce

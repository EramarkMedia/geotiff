#ifndef TILE_GENERATOR_TYPES_H
#define TILE_GENERATOR_TYPES_H

namespace TileGeneratorInternal {
	struct key_t {
		int x;
		int z;
		friend constexpr bool operator ==(const key_t& a, const key_t& b) {
			return a.x == b.x && a.z == b.z;
		};
		friend constexpr bool operator !=(const key_t& a, const key_t& b) {
			return !(a == b);
		};
	};
	
	struct cache_entry_t {
		key_t tile_key;
		Ref<Image> image;
	};
}

#endif // TILE_GENERATOR_TYPES_H

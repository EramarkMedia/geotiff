#ifndef TILE_GENERATOR_FUNCTIONS_H
#define TILE_GENERATOR_FUNCTIONS_H

#include "tile_generator_types.h"

namespace TileGeneratorInternal {
	constexpr inline int floor_div(const int n, const int d){
		if (n >= 0) {
			return (n - (n % d)) / d;
		}
		else {
			return (n - (n % d)) / d - 1;
		}
	}
	
	constexpr inline key_t get_tile_key_from_position(const int x_position, const int z_position) {
		const int tile_size = 3000;
		const int tile_x_index = floor_div(x_position, tile_size);
		const int tile_z_index = floor_div(z_position, tile_size);
		return key_t{.x = tile_x_index, .z = tile_z_index};
	}
}

#endif // TILE_GENERATOR_FUNCTIONS_H

#ifndef TILE_GENERATOR_H
#define TILE_GENERATOR_H

#include "modules/voxel/generators/voxel_generator.h"
#include "modules/voxel/streams/voxel_block_request.h"
#include "core/ustring.h"
#include <unordered_map>
#include <functional>

class TileGenerator : public  VoxelGenerator {
	GDCLASS(TileGenerator, VoxelGenerator)

private:
	int _floor_elevation;
	int _ceiling_elevation;
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
	std::unordered_map<key_t,String,std::function<size_t(key_t)>> _tile_map;
	constexpr int _floor_div(const int n, const int d){
		if (n >= 0) {
			return (n - (n % d)) / d;
		}
		else {
			return (n - (n % d)) / d - 1;
		}
	}
	constexpr key_t _get_tile_key_from_position(const int x_position, const int z_position) {
		const int tile_size = 3000;
		const int tile_x_index = _floor_div(x_position, tile_size);
		const int tile_z_index = _floor_div(z_position, tile_size);
		return key_t{.x = tile_x_index, .z = tile_z_index};
	}

protected:
	static void _bind_methods();

public:
	TileGenerator();
	virtual void generate_block(VoxelBlockRequest &input);
	void set_floor_elevation(int floor_elevation);
	int get_floor_elevation();
	void set_ceiling_elevation(int ceiling_elevation);
	void set_cell(int x, int z, String resource);
	int get_ceiling_elevation();
};

#endif // TILE_GENERATOR_H

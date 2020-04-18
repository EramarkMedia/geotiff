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
	};
	std::unordered_map<key_t,String,std::function<size_t(key_t)>> _tile_map;

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

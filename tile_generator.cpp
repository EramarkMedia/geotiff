#include "tile_generator.h"
#include "modules/voxel/voxel_buffer.h"
#include "modules/voxel/math/vector3i.h"

TileGenerator::TileGenerator() : _floor_elevation(0), _ceiling_elevation(256) {
	std::function<size_t(key_t)> hash_f = [](key_t k) {
		return k.x ^ (k.z << 3);
	};
	_tile_map = std::unordered_map<key_t,String,std::function<size_t(key_t)>>(8, hash_f);
}

void TileGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_floor_elevation", "floor_elevation"), &TileGenerator::set_floor_elevation);
	ClassDB::bind_method(D_METHOD("get_floor_elevation"), &TileGenerator::get_floor_elevation);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "floor_elevation"), "set_floor_elevation", "get_floor_elevation");
	ClassDB::bind_method(D_METHOD("set_ceiling_elevation", "ceiling_elevation"), &TileGenerator::set_ceiling_elevation);
	ClassDB::bind_method(D_METHOD("get_ceiling_elevation"), &TileGenerator::get_ceiling_elevation);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ceiling_elevation"), "set_ceiling_elevation", "get_ceiling_elevation");
	ClassDB::bind_method(D_METHOD("set_cell", "x", "z", "resource"), &TileGenerator::set_cell);
}

void TileGenerator::set_floor_elevation(int floor_elevation) {
	_floor_elevation = floor_elevation;
}

int TileGenerator::get_floor_elevation() {
	return _floor_elevation;
}

void TileGenerator::set_ceiling_elevation(int ceiling_elevation) {
	_ceiling_elevation = ceiling_elevation;
}

int TileGenerator::get_ceiling_elevation() {
	return _ceiling_elevation;
}

void TileGenerator::set_cell(int x, int z, String resource) {
	key_t k = key_t{.x = x, .z = z};
	_tile_map.insert_or_assign(k, resource);
}

void TileGenerator::generate_block(VoxelBlockRequest &input) {
	ERR_FAIL_COND(input.voxel_buffer.is_null());
	//auto it = _tile_map.find(key_t{.x=1,.z=1});
	//if (it == _tile_map.end()) {
		//print_line("generate_block: end");
	//}
	//else {
		//print_line("generate_block: _tile_map[1,1] = " + it->second);
	//}
	if (input.origin_in_voxels.y < 0) {
		//print_line("Request: lod=" + String::num_int64(input.lod) + ", origin_in_voxels.y=" + String::num_int64(input.origin_in_voxels.y));
		VoxelBuffer &out_buffer = **input.voxel_buffer;
		out_buffer.fill(1, 1);
	}
}

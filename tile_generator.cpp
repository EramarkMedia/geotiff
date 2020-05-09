#include "tile_generator.h"
#include "modules/voxel/voxel_buffer.h"
#include "modules/voxel/math/vector3i.h"
#include <cmath>

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

constexpr int absmod(const int n, const int d) {
	if (n < 0) {
		return (n % d) + d;
	}
	else {
		return n % d;
	}
};

void TileGenerator::generate_block(VoxelBlockRequest &input) {
	ERR_FAIL_COND(input.voxel_buffer.is_null());
	ERR_FAIL_COND(input.lod < 0);
	VoxelBuffer &out_buffer = **input.voxel_buffer;
	const Vector3i &buffer_size = out_buffer.get_size();
	const int stride = 1 << input.lod;
	const int y_low = input.origin_in_voxels.y;
	const int y_high = y_low + (buffer_size.y - 1) * stride;
	
	if (y_high <= _floor_elevation) {
		out_buffer.fill(1, 1);
		return;
	}
	
	if (y_low >= _ceiling_elevation) return;
	
	const int x_low = input.origin_in_voxels.x;
	const int x_high = x_low + (buffer_size.x - 1) * stride;
	const int z_low = input.origin_in_voxels.z;
	const int z_high = z_low + (buffer_size.z - 1) * stride;
	
	const int tile_size = 3000;
	key_t tile_key = _get_tile_key_from_position(x_low, z_low);
	auto it = _tile_map.find(tile_key);
	RES tile_res;
	if (it != _tile_map.end()) {
		tile_res = ResourceLoader::load(it->second);
	}
	Ref<Image> tile_img = Ref<Image>(Object::cast_to<Image>(*tile_res));
	
	for (int xi = 0; xi < buffer_size.x; xi++) {
		const int x_position = x_low + xi * stride;
		for (int zi = 0; zi < buffer_size.z; zi++) {
			const int z_position = z_low + zi * stride;
			const key_t tile_key_for_position = _get_tile_key_from_position(x_position, z_position);
			if (tile_key != tile_key_for_position) {
				tile_key = tile_key_for_position;
				it = _tile_map.find(tile_key);
				if (it != _tile_map.end()) {
					tile_res = ResourceLoader::load(it->second);
					tile_img = Ref<Image>(Object::cast_to<Image>(*tile_res));
				}
				else {
					tile_res = RES();
					tile_img = Ref<Image>();
				}
			}
			
			if (tile_img.is_valid()) {
				const int tile_reference_position_x = absmod(x_position, tile_size);
				const int tile_reference_position_z = absmod(z_position, tile_size);
				//print_line("ref: " + String::num_int64(tile_reference_position_x) + ", " + String::num_int64(tile_reference_position_z));
				tile_img->lock();
				const float elevation_at_position = tile_img->get_pixel(tile_reference_position_x, tile_reference_position_z).r;
				tile_img->unlock();
				if (elevation_at_position >= y_low) {
					if (elevation_at_position >= y_high) {
						out_buffer.fill_area(1,
							Vector3i(xi, 0, zi),
							Vector3i(xi+1, buffer_size.y, zi+1),
							1);
					}
					else {
						float integral_part = 0;
						float fractional_part = std::modf(elevation_at_position, &integral_part);
						const int top_cell = (int)((integral_part - (float)y_low) / (float)stride);
						const float top_cell_value = fractional_part / (float)stride;
						out_buffer.fill_area(0,
							Vector3i(xi, 0, zi),
							Vector3i(xi+1, top_cell, zi+1),
							1);
						out_buffer.set_voxel_f(0.5-top_cell_value, xi, top_cell, zi, 1);
					}
				}
			}
		}
	}
}

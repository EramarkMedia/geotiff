#include "tile_generator.h"
#include "tile_generator_functions.h"
#include "modules/voxel/voxel_buffer.h"
#include "modules/voxel/math/vector3i.h"
#include "core/io/resource_loader.h"
#include "core/image.h"
#include <cmath>

namespace TGI = TileGeneratorInternal;

TileGenerator::TileGenerator() : _floor_elevation(0), _ceiling_elevation(256) {
	std::function<size_t(TGI::key_t)> hash_f = [](TGI::key_t k) {
		return k.x ^ (k.z << 3);
	};
	_tile_map = std::unordered_map<TGI::key_t,String,std::function<size_t(TGI::key_t)>>(8, hash_f);
}

void TileGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_floor_elevation", "floor_elevation"), &TileGenerator::set_floor_elevation);
	ClassDB::bind_method(D_METHOD("get_floor_elevation"), &TileGenerator::get_floor_elevation);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "floor_elevation"), "set_floor_elevation", "get_floor_elevation");
	ClassDB::bind_method(D_METHOD("set_ceiling_elevation", "ceiling_elevation"), &TileGenerator::set_ceiling_elevation);
	ClassDB::bind_method(D_METHOD("get_ceiling_elevation"), &TileGenerator::get_ceiling_elevation);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ceiling_elevation"), "set_ceiling_elevation", "get_ceiling_elevation");
	ClassDB::bind_method(D_METHOD("set_cell", "x", "z", "resource"), &TileGenerator::set_cell);
	ClassDB::bind_method(D_METHOD("get_elevation", "x", "z"), &TileGenerator::get_elevation);
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
	TGI::key_t k = TGI::key_t{.x = x, .z = z};
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

const inline std::optional<Ref<Image>> TileGenerator::_get_tile_from_tile_key(const TGI::key_t tile_key) {
	// examine cache
	for (int i = 0; i < 4; i++) {
		if (_image_cache[i] && _image_cache[i]->tile_key == tile_key) {
			return _image_cache[i]->image;
		}
	}
	
	// load if not in cache
	auto it = _tile_map.find(tile_key);
	if (it == _tile_map.end()) {
		return std::optional<Ref<Image>>();
	}
	RES tile_res = ResourceLoader::load(it->second);
	Ref<Image> tile_img = Ref<Image>(Object::cast_to<Image>(*tile_res));
	if (!tile_img.is_valid()) {
		return std::optional<Ref<Image>>();
	}
	
	// save to cache
	_image_cache[_next_cache_entry] = TGI::cache_entry_t{.tile_key = tile_key, .image = tile_img};
	_next_cache_entry = (_next_cache_entry + 1) % 4;
	
	return tile_img;
};

struct TileGenerator::HeightMapAccess {
	const int tile_size = 3000;
	TGI::key_t tile_key;
	std::optional<Ref<Image>> tile_img;
	TileGenerator& tile_generator;
	
	HeightMapAccess(TileGenerator* tile_generator) : tile_generator(*tile_generator) {};
	
	std::optional<float> get_elevation_internal(const int x, const int z) {
		TGI::key_t request_tile_key = TGI::get_tile_key_from_position(x, z);
		if (!tile_img || !tile_img->is_valid() || request_tile_key != tile_key) {
			tile_img = tile_generator._get_tile_from_tile_key(request_tile_key);
			tile_key = request_tile_key;
		}
		
		if (!tile_img || !tile_img->is_valid() || request_tile_key != tile_key)
			return false;
		
		const int tile_reference_position_x = absmod(x, tile_size);
		const int tile_reference_position_z = absmod(z, tile_size);
		(*tile_img)->lock();
		const float elevation_at_position = (*tile_img)->get_pixel(tile_reference_position_x, tile_reference_position_z).r;
		(*tile_img)->unlock();
		
		return elevation_at_position;
	}
	
	std::optional<float> get_elevation(const real_t x, const real_t z) {
		const int x_floor = floor(x);
		const int z_floor = floor(z);
		const std::optional<float> north_west_elevation = get_elevation_internal(x_floor, z_floor);
		const std::optional<float> north_east_elevation = get_elevation_internal(x_floor + 1, z_floor);
		const std::optional<float> south_west_elevation = get_elevation_internal(x_floor, z_floor + 1);
		const std::optional<float> south_east_elevation = get_elevation_internal(x_floor + 1, z_floor + 1);
		
		if (north_west_elevation && north_east_elevation && south_west_elevation && south_east_elevation) {
			const float west_east_part = x - x_floor;
			const float north_south_part = z - z_floor;
			const float north_mix = *north_east_elevation * west_east_part + *north_west_elevation * (1 - west_east_part);
			const float south_mix = *south_east_elevation * west_east_part + *south_west_elevation * (1 - west_east_part);
			const float full_mix = south_mix * north_south_part + north_mix * (1 - north_south_part);
			return full_mix;
		}
		
		return false;
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
	//const int x_high = x_low + (buffer_size.x - 1) * stride;
	const int z_low = input.origin_in_voxels.z;
	//const int z_high = z_low + (buffer_size.z - 1) * stride;
	
	HeightMapAccess height_map(this);
	
	for (int xi = 0; xi < buffer_size.x; xi++) {
		const int x_position = x_low + xi * stride;
		for (int zi = 0; zi < buffer_size.z; zi++) {
			const int z_position = z_low + zi * stride;
			
			const std::optional<float> elevation_at_position = height_map.get_elevation(x_position, z_position);
			if (elevation_at_position) {
				if (elevation_at_position >= y_low) {
					if (elevation_at_position >= y_high) {
						out_buffer.fill_area(1,
							Vector3i(xi, 0, zi),
							Vector3i(xi+1, buffer_size.y, zi+1),
							1);
					}
					else {
						float integral_part = 0;
						float fractional_part = std::modf(*elevation_at_position, &integral_part);
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

real_t TileGenerator::get_elevation(const real_t x, const real_t z) {
	HeightMapAccess height_map(this);
	const std::optional<float> elevation = height_map.get_elevation(x, z);
	if (elevation) {
		return *elevation;
	}
	
	return -1.0;
}

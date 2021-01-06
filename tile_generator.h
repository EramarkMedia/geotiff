#ifndef TILE_GENERATOR_H
#define TILE_GENERATOR_H

#include "modules/voxel/generators/voxel_generator.h"
#include "modules/voxel/streams/voxel_block_request.h"
#include "core/ustring.h"
#include <unordered_map>
#include <functional>
#include <array>
#include <optional>

#include "tile_generator_types.h"

namespace TGI = TileGeneratorInternal;

class TileGenerator : public  VoxelGenerator {
	GDCLASS(TileGenerator, VoxelGenerator)

private:
	int _floor_elevation;
	int _ceiling_elevation;
	std::array<std::optional<TGI::cache_entry_t>,4> _image_cache;
	int _next_cache_entry = 0;
	std::unordered_map<TGI::key_t,String,std::function<size_t(TGI::key_t)>> _tile_map;
	const inline std::optional<Ref<Image>> _get_tile_from_tile_key(const TGI::key_t tile_key);

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

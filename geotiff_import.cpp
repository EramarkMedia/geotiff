#include "core/print_string.h"
#include "core/class_db.h"
#include "core/color.h"
#include "core/image.h"
#include "core/os/os.h"
#include "core/io/resource_saver.h"
#include "geotiff_import.h"
#include <geotiff/xtiffio.h>
#include <vector>
#include <algorithm>

void GeotiffImportPlugin::_bind_methods() {
}

String GeotiffImportPlugin::get_importer_name() const {
	return "geotiff.import.plugin";
}

String GeotiffImportPlugin::get_visible_name() const {
	return "GeoTIFF Importer";
}

void GeotiffImportPlugin::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("tif");
	p_extensions->push_back("tiff");
}

String GeotiffImportPlugin::get_preset_name(int p_idx) const {
	return "Default";
}

int GeotiffImportPlugin::get_preset_count() const {
	return 1;
}

String GeotiffImportPlugin::get_save_extension() const {
	return "res";
}

String GeotiffImportPlugin::get_resource_type() const {
	return "Image";
}

void GeotiffImportPlugin::get_import_options(List<ImportOption> *r_options, int p_preset) const { }

bool GeotiffImportPlugin::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return false;
}

class TiffFile {
	TIFF* tif;
public:
	TiffFile(String source_file) : tif(XTIFFOpen(source_file.utf8(), "r")) { }
	~TiffFile() {
		XTIFFClose(tif);
	}
	operator bool() { return tif; }
	const unsigned int GetHeight() {
		unsigned int height = 0;
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		return height;
	}
	const unsigned int GetWidth() {
		unsigned int height = 0;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &height);
		return height;
	}
	const unsigned int GetTileHeight() {
		unsigned int height = 0;
		TIFFGetField(tif, TIFFTAG_TILELENGTH, &height);
		return height;
	}
	const unsigned int GetTileWidth() {
		unsigned int height = 0;
		TIFFGetField(tif, TIFFTAG_TILEWIDTH, &height);
		return height;
	}
	const unsigned int GetTileSize() {
		return TIFFTileSize(tif);
	}
	inline void ReadTile(std::vector<float> &buf, unsigned int x, unsigned int y) {
		ERR_FAIL_COND_MSG((buf.size()*sizeof(float)) < GetTileSize(), "buffer too small to store tile");
		TIFFReadTile(tif, buf.data(), x, y, 0, 0);
	}
	inline void ReadScanline(std::vector<float> &buf, unsigned int row) {
		TIFFReadScanline(tif, buf.data(), row);
	}
};

constexpr unsigned int minuint(const unsigned int a, const unsigned int b) {
	if (a < b) return a;
	else return b;
}

Error GeotiffImportPlugin::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	String source_path = OS::get_singleton()->get_resource_dir() + p_source_file.substr(5);
	//String save_path = OS::get_singleton()->get_resource_dir() + p_save_path.substr(5) + ".exr";
	
	unsigned int width, height;
	PoolVector<uint8_t> *image_data = memnew(PoolVector<uint8_t>);
	
	{
		TiffFile tiff_file = TiffFile(source_path);
		if (!tiff_file) { return ERR_FILE_CANT_READ; }
		height = tiff_file.GetHeight();
		width = tiff_file.GetWidth();
		image_data->resize(width * height * sizeof(float));
		float* p_image_data = (float*)image_data->write().ptr();
		
		const unsigned int tile_height = tiff_file.GetTileHeight();
		print_line("tile_height = " + String::num_uint64(tile_height));
		if (tile_height == 0) {
			std::vector<float> buf = std::vector<float>(width);
			for (unsigned int row = 0; row < height; row++) {
				tiff_file.ReadScanline(buf, row);
				std::copy_n(&buf[0], width, &p_image_data[row * width]);
			}
		}
		else {
			const unsigned int tile_width = tiff_file.GetTileWidth();
			std::vector<float> buf = std::vector<float>(tile_width * tile_height);
			for (unsigned int y = 0; y < height; y += tile_height) {
				for (unsigned int x = 0; x < width; x += tile_width) {
					tiff_file.ReadTile(buf, x, y);
					unsigned int row_width = minuint(tile_width, width - x);
					unsigned int row_count = minuint(tile_height, height - y);
					for (unsigned int row = 0; row < row_count; row++) {
						std::copy_n(&buf[row * tile_width], row_width, &p_image_data[(y + row) * width + x]);
					}
				}
			}
		}
	}
	
	Ref<Image> image_import;
	image_import.instance();
	image_import->create(width, height, false, Image::Format::FORMAT_RF, *image_data);
	memdelete(image_data);
	
	return ResourceSaver::save(p_save_path + ".res", image_import);
}

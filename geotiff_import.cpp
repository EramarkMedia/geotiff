#include "core/print_string.h"
#include "core/class_db.h"
#include "core/image.h"
#include "core/os/os.h"
#include "core/io/resource_saver.h"
#include "geotiff_import.h"
#include <geotiff/xtiffio.h>
#include <vector>

void GeotiffImportPlugin::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("do_test"), &TestRef::do_test);
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
};

Error GeotiffImportPlugin::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	print_line("(" + p_source_file + "," + p_save_path + ")");
	//print_line("**" + OS::get_singleton()->get_resource_dir());
	//print_line("*-" + OS::get_singleton()->get_resource_dir() + p_save_path.substr(5));

	String source_path = OS::get_singleton()->get_resource_dir() + p_source_file.substr(5);
	String save_path = OS::get_singleton()->get_resource_dir() + p_save_path.substr(5) + ".exr";
	
	Ref<Image> image_import;
	
	{
		TiffFile tiff_file = TiffFile(source_path);
		if (!tiff_file) { return ERR_FILE_CANT_READ; }
		const unsigned int height = tiff_file.GetHeight();
		const unsigned int width = tiff_file.GetWidth();
		const unsigned int tile_height = tiff_file.GetTileHeight();
		const unsigned int tile_width = tiff_file.GetTileWidth();
		const unsigned int tile_size = tiff_file.GetTileSize();
		print_line("tile_size = " + String::num_uint64(tile_size));
		if (tile_size != tile_height * tile_width * sizeof(float)) { return ERR_FILE_UNRECOGNIZED; }
		image_import.instance();
		image_import->create(width, height, false, Image::Format::FORMAT_RF);
		std::vector<float> buf = std::vector<float>(tile_width * tile_height);
		for (unsigned int y = 0; y < height; y += tile_height) {
			for (unsigned int x = 0; x < width; x += tile_width) {
				if (x == 0 && y == 0) { print_line(String::num_real(buf[0])); }
				tiff_file.ReadTile(buf, x, y);
				if (x == 0 && y == 0) { print_line(String::num_real(buf[0])); }
			}
		}
	}
	
	return FAILED;
	
	List<String> *p_extensions = memnew(List<String>);
	ResourceSaver::get_recognized_extensions(image_import, p_extensions);
	print_line("Size " + String::num_uint64(p_extensions->size()));
	for (int i = 0; i < p_extensions->size(); i++) {
		print_line((*p_extensions)[i]);
	}
	//Error ret = image_import->save_exr(save_path, true);
	Error ret = ResourceSaver::save(p_save_path + ".res", image_import);
	print_line("ding" + String::num_uint64((uint64_t)ret));
	return ret;
}

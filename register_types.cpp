#include "register_types.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "core/io/resource_importer.h"
#include "editor/editor_file_system.h"

#include "etrs89_projection.h"
#include "geotiff_import.h"
#include "tile_generator.h"

static GeotiffImportPlugin* geotiff_import_plugin_instance = nullptr;

void register_geotiff_types() {
	ClassDB::register_class<TileGenerator>();
	ClassDB::register_class<ETRS89Projection>();

#ifdef TOOLS_ENABLED
	ClassDB::register_class<GeotiffImportPlugin>();

	if (Engine::get_singleton()->is_editor_hint()) {
		geotiff_import_plugin_instance = memnew(GeotiffImportPlugin);
		ResourceFormatImporter::get_singleton()->add_importer(geotiff_import_plugin_instance);
		//EditorFileSystem::get_singleton()->call_deferred("scan");
	}
#endif // TOOLS_ENABLED
}

void unregister_geotiff_types() {
#ifdef TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint()) {
		ResourceFormatImporter::get_singleton()->remove_importer(geotiff_import_plugin_instance);
	}
#endif // TOOLS_ENABLED
}

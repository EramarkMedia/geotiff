#ifndef ETRS89_PROJECTION_H
#define ETRS89_PROJECTION_H

#include "core/reference.h"
#include "core/math/vector2.h"

class ETRS89Projection : public Reference {
	GDCLASS(ETRS89Projection, Reference);
	
	Vector2 _origin = Vector2(326000.0, 7050000.0);
	Vector2 _scale = Vector2(0.5,0.5);
protected:
	static void _bind_methods();
public:
	Vector2 get_origin() const;
	void set_origin(const Vector2& coords);
	Vector2 get_scale() const;
	void set_scale(const Vector2& scale);
	Vector2 local_to_global(const Vector2& coords) const;
	Vector2 global_to_local(const Vector2& coords) const;
};

#endif // ETRS89_PROJECTION_H

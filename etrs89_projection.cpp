// The implementation herein is made in reference to the implementation
// found at http://www.loukko.net/koord_proj/

#include "etrs89_projection.h"

Vector2 ETRS89Projection::get_origin() const {
	return _origin;
}

void ETRS89Projection::set_origin(const Vector2& origin) {
	_origin = origin;
}

Vector2 ETRS89Projection::get_scale() const {
	return _scale;
}

void ETRS89Projection::set_scale(const Vector2& scale) {
	_scale = scale;
}

const real_t f = 1.0/298.257222101;
const real_t a = 6378137.0;
const real_t lambda_nolla = 0.471238898;
const real_t k_nolla = 0.9996;
const real_t E_nolla = 500000.0;

Vector2 ETRS89Projection::local_to_global(const Vector2& coords) const {
	const real_t N = _origin.y - (coords.y / _scale.y);
	const real_t E = _origin.x + (coords.x / _scale.x);
	
	const real_t n = f/(2.0-f);
	const real_t A1 = (a/(1.0+n)) * (1.0+(pow(n,2.0)/4.0) + (pow(n,4.0)/64.0));
	const real_t e_toiseen = (2.0 * f) - pow(f, 2.0);
	const real_t h1 = (1.0/2.0)*n - (2.0/3.0)*pow(n, 2.0) + (37.0/96.0)*pow(n, 3.0) - (1.0/360.0)*pow(n, 4.0);
	const real_t h2 = (1.0/48.0)*pow(n, 2.0) + (1.0/15.0)*pow(n, 3.0) - (437.0/1440.0)*pow(n, 4.0);
	const real_t h3 =(17.0/480.0)*pow(n, 3.0) - (37.0/840.0)*pow(n, 4.0);
	const real_t h4 = (4397.0/161280.0)*pow(n, 4.0);
	
	const real_t zeeta = N / (A1 * k_nolla);
	const real_t eeta = (E - E_nolla) / (A1 * k_nolla);
	const real_t zeeta1_pilkku = h1 * sin( 2.0 * zeeta) * cosh( 2.0 * eeta);
	const real_t zeeta2_pilkku = h2 * sin( 4.0 * zeeta) * cosh( 4.0 * eeta);
	const real_t zeeta3_pilkku = h3 * sin( 6.0 * zeeta) * cosh( 6.0 * eeta);
	const real_t zeeta4_pilkku = h4 * sin( 8.0 * zeeta) * cosh( 8.0 * eeta);
	const real_t eetA1_pilkku = h1 * cos( 2.0 * zeeta) * sinh( 2.0 * eeta);
	const real_t eeta2_pilkku = h2 * cos( 4.0 * zeeta) * sinh( 4.0 * eeta);
	const real_t eeta3_pilkku = h3 * cos( 6.0 * zeeta) * sinh( 6.0 * eeta);
	const real_t eeta4_pilkku = h4 * cos( 8.0 * zeeta) * sinh( 8.0 * eeta);
	const real_t zeeta_pilkku = zeeta - (zeeta1_pilkku + zeeta2_pilkku + zeeta3_pilkku + zeeta4_pilkku);
	const real_t eeta_pilkku = eeta - (eetA1_pilkku + eeta2_pilkku + eeta3_pilkku + eeta4_pilkku);
	const real_t beeta = asin((1.0/cosh(eeta_pilkku)*sin(zeeta_pilkku)));
	const real_t l = asin(tanh(eeta_pilkku)/(cos(beeta)));
	const real_t Q = asinh(tan(beeta));
	real_t Q_pilkku = Q + sqrt(e_toiseen) * atanh(sqrt(e_toiseen) * tanh(Q));
	
	for (int kierros = 1; kierros < 5; kierros++) {
		Q_pilkku = Q + sqrt(e_toiseen) * atanh(sqrt(e_toiseen) * tanh(Q_pilkku));
	}
	
	real_t fii = atan(sinh(Q_pilkku));
	real_t lambda = lambda_nolla + l;
	
	fii = Math::rad2deg(fii);
	lambda = Math::rad2deg(lambda);
	
	return Vector2(lambda,fii);
}

Vector2 ETRS89Projection::global_to_local(const Vector2& coords) const {
	const real_t lev_aste = coords.y;
	const real_t pit_aste = coords.x;
	
	const real_t fii = Math::deg2rad(lev_aste);
	const real_t lambda = Math::deg2rad(pit_aste);
	
	const real_t n = f / (2.0-f);
	const real_t A1 = (a/(1.0+n)) * (1.0 + (pow(n, 2.0)/4.0) + (pow(n, 4.0)/64.0));
	const real_t e_toiseen = (2.0 * f) - pow(f, 2.0);
	const real_t h1_pilkku = (1.0/2.0)*n - (2.0/3.0)*pow(n, 2.0) + (5.0/16.0)*pow(n, 3.0) + (41.0/180.0)*pow(n, 4.0);
	const real_t h2_pilkku = (13.0/48.0)*pow(n, 2.0) - (3.0/5.0)*pow(n, 3.0) + (557.0/1440.0)*pow(n, 4.0);
	const real_t h3_pilkku =(61.0/240.0)*pow(n, 3.0) - (103.0/140.0)*pow(n, 4.0);
	const real_t h4_pilkku = (49561.0/161280.0)*pow(n, 4.0);
	const real_t Q_pilkku = asinh( tan(fii));
	const real_t Q_2pilkku = atanh(sqrt(e_toiseen) * sin(fii));
	const real_t Q = Q_pilkku - sqrt(e_toiseen) * Q_2pilkku;
	const real_t l = lambda - lambda_nolla;
	const real_t beeta = atan(sinh(Q));
	const real_t eeta_pilkku = atanh(cos(beeta) * sin(l));
	const real_t zeeta_pilkku = asin(sin(beeta)/(1.0/cosh(eeta_pilkku)));
	const real_t zeeta1 = h1_pilkku * sin( 2.0 * zeeta_pilkku) * cosh( 2.0 * eeta_pilkku);
	const real_t zeeta2 = h2_pilkku * sin( 4.0 * zeeta_pilkku) * cosh( 4.0 * eeta_pilkku);
	const real_t zeeta3 = h3_pilkku * sin( 6.0 * zeeta_pilkku) * cosh( 6.0 * eeta_pilkku);
	const real_t zeeta4 = h4_pilkku * sin( 8.0 * zeeta_pilkku) * cosh( 8.0 * eeta_pilkku);
	const real_t eeta1 = h1_pilkku * cos( 2.0 * zeeta_pilkku) * sinh( 2.0 * eeta_pilkku);
	const real_t eeta2 = h2_pilkku * cos( 4.0 * zeeta_pilkku) * sinh( 4.0 * eeta_pilkku);
	const real_t eeta3 = h3_pilkku * cos( 6.0 * zeeta_pilkku) * sinh( 6.0 * eeta_pilkku);
	const real_t eeta4 = h4_pilkku * cos( 8.0 * zeeta_pilkku) * sinh( 8.0 * eeta_pilkku);
	const real_t zeeta = zeeta_pilkku + zeeta1 + zeeta2 + zeeta3 + zeeta4;
	const real_t eeta = eeta_pilkku + eeta1 + eeta2 + eeta3 + eeta4;
	
	const real_t N = A1 * zeeta * k_nolla;
	const real_t E = A1 * eeta * k_nolla + E_nolla;
	
	return Vector2((E - _origin.x) * _scale.x, (-N + _origin.y) * _scale.y);
}

void ETRS89Projection::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_origin"), &ETRS89Projection::set_origin);
	ClassDB::bind_method(D_METHOD("get_origin"), &ETRS89Projection::get_origin);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "origin"), "set_origin", "get_origin");
	ClassDB::bind_method(D_METHOD("set_scale"), &ETRS89Projection::set_scale);
	ClassDB::bind_method(D_METHOD("get_scale"), &ETRS89Projection::get_scale);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "scale"), "set_scale", "get_scale");
	ClassDB::bind_method(D_METHOD("local_to_global", "coords"), &ETRS89Projection::local_to_global);
	ClassDB::bind_method(D_METHOD("global_to_local", "coords"), &ETRS89Projection::global_to_local);
}

#define W0VECXY(x,y) vec3(cos(x)*cos(y),sin(x)*cos(y),sin(y))
#define W0VEC(w0) W0VECXY(w0.x, w0.y)
#define GETW0(w0,a) W0VECXY(w0.x + floor(a * gLobes) / (2.0 * gLobes) * 2.0 * M_PI, w0.y)

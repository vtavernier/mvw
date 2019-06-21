//! bool dQuad def=false cat="Rendering" unm="Render as quad"
uniform bool dQuad;

void mainImage(out vec4 O, in vec2 U) {
    O = vec4(vec3(length(U)), 1.0);
    O.rg = U;
    O.b = 0.;
}

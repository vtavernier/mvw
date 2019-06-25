//! bool dQuad def=false cat="Rendering" unm="Render as quad"

//! bool dCoord def=false cat="Debug" unm="Render coordinates"
uniform bool dCoord;

void mainImage(out vec4 O, in vec2 U)
{
    if (dCoord) {
        O = vec4(U.rg, 0., 1.);
    } else {
        O = texture(roundtripInput, U.rg);
    }
}

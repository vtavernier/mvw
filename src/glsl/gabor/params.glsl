//! int gSplats min=1 max=30 def=3 fmt="%d" cat="Gabor noise" unm="Splats"
uniform int gSplats;
//! float gF0 min=0.001 max=100.0 fmt="%2.4f" pow=1.0 cat="Gabor noise" unm="F0" def=1.0 mode=input
uniform float gF0;
//! vec2 gW0 min=0 max=360 fmt="%2.2f" cat="Gabor noise" unm="W0" mod=angle
uniform vec2 gW0;
//! bool dGrid def=false cat="Debug" unm="Show grid"
uniform bool dGrid;

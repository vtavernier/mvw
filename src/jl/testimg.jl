using MVW, Images, Printf, Base.Filesystem

base_path = "../../build/"
s = (1024, 1024)
models = [
  (path="../models/mcguire/bunny/bunny.obj",
   location=[5, 2, 0], target=[0, 0, 0], up=[0, 1, 0],
   rotation=[0, 100], scale=1.225, f0=5.0),
  (path="../models/mcguire/dragon/dragon.obj",
   location=[5, 2, 0], target=[0, 0, 0], up=[0, 1, 0],
   rotation=[0, 0], scale=2, f0=150.0)
]

spawn(base_path, ["-G", "s 0 0 0 1", "-s", "glsl/gabor-noise-solid.glsl", "-p", "glsl/pp-contrast.glsl", "-m", "-q"]) do c
    for model in models
        model_name = basename(dirname(model.path))

        # Set geometry
        println("Loading $(model.path)")
        geometry(c, false, model.path)

        # Camera settings
        setcamera(c, model.location, model.target, model.up)
        setrotation(c, model.rotation)
        setscale(c, model.scale)

        # Noise settings
        setparam(c, "gTilesize", 1.0)
        setparam(c, "gSplats", 3)
        setparam(c, "gF0", model.f0)

        # Contrast correction settings
        setparam(c, "cFilterLod", 1.0)

        println("Saving $(model.path) frames")

        # First: vertical stripes
        for w0x in [0.0, pi/2, pi/4, 2*pi/3]
            for w0y in [0.0, pi/2, pi/4, 2*pi/3]
                println(@sprintf "%2.3f_%2.3f" w0x w0y)

                setparam(c, "gW0", [w0x, w0y])
                save(base_path * "$model_name-f25-$(@sprintf "w%2.3f_%2.3f" w0x w0y)-c1.png", getframe(c; size=s))
            end
        end
    end
end

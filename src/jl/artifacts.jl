using MVW, Images
using Plots: plot, heatmap
using DSP

# Kill the existing viewer if we're restarting
if @isdefined mPN
    finalize(mPN)
end
# Spawn viewer instance with the phasor noise shader
mPN = spawn("../../build", ["-G", "#test\nplane\n", "-s", "glsl/phasor-noise-solid.glsl", "-p", "glsl/pp-phasor.glsl", "-m"]);

# Shared parameters
p = Dict(:gF0 => 10.0, :gTilesize => 0.25, :dQuad => true)
# Get a noise image and the complex phasor field
r = [
    getf(mPN, size=(512,512); p..., dPhase = false),
    getf(mPN, size=(512,512); p..., dPhase = true, dComplexOutput = true),
    getf(mPN, size=(512,512); p..., dPhase = true, dNoiseOverlay = true),
]

# Complex phasor field, [-1;1] range
phasor = 2. .* channelview(r[2])[1:2,:,:] .- 1.;
phasor = Complex.(phasor[1,:,:], phasor[2,:,:]);

# plot(heatmap(channelview(r[1])[1,:,:], aspectratio=1, yflip=true, c=:grays),
#      heatmap(angle.(phasor), aspectratio=1, yflip=true, c=:phase), layout=2)

angleimg(x) = HSV((angle(x) + pi) / 2pi * 360., 1., 1.)
img = clamp01.(r[1])

function sampleline(data::Matrix{T}, x0::Int, y0::Int, x1::Int, y1::Int) where T
    δx = abs(x1 - x0)
    δy = abs(y1 - y0)
    er = 0.0
    vals = Vector{T}()

    if δy < δx
        if x0 > x1
            (x0, x1) = (x1, x0)
            (y0, y1) = (y1, y0)
        end

        δe = abs(δy / δx)

        y = y0
        for x in x0:x1
            append!(vals, data[x, y])
            er += δe
            if er > 0.5
                y  += 1
                er -= 1.0
            end
        end
    else
        if y0 > y1
            (y0, y1) = (y1, y0)
            (x0, x1) = (x1, x0)
        end

        δe = abs(δx / δy)

        x = x0
        for y in y0:y1
            append!(vals, data[x, y])
            er += δe
            if er > 0.5
                x  += 1
                er -= 1.0
            end
        end
    end

    return vals
end

function drawline(ctx, l, color)
    isempty(l) && return
    p = first(l)
    move_to(ctx, p.x, p.y)
    set_source(ctx, color)
    for i = 2:length(l)
        p = l[i]
        line_to(ctx, p.x, p.y)
    end
    stroke(ctx)
end

function plotselected()
    if length(value(lines)) > 0 && length(value(lines)[1]) > 1
        # Get size details
        a = value(lines)[1][1]
        b = value(lines)[1][2]
        d = angle.(phasor)
        s = size(d)

        # Compute points
        x0 = trunc(Int, a.x.val * s[1])
        y0 = s[2] - floor(Int, a.y.val * s[2] + .5)
        x1 = trunc(Int, b.x.val * s[1])
        y1 = s[2] - floor(Int, b.y.val * s[2] + .5)

        noisev = sampleline(2 .* channelview(r[1])[1,:,:] .- 1., x0, y0, x1, y1)
        phasev = sampleline(angle.(phasor), x0, y0, x1, y1)
        Unwrap.unwrap!(phasev)

        display(plot(
            plot(noisev, ylims = (-1,1), m = (:dot), title = "Noise value"),
            plot(phasev, ylims = (-2pi,2pi), m = (:dot), title = "Phasor phase")))
    end
    nothing
end

using Gtk.ShortNames, GtkReactive, Graphics, Colors

if !@isdefined win
    win = Window("Drawing")
    c = canvas(UserUnit)       # create a canvas with user-specified coordinates
    push!(win, c)

    const lines = Signal([])   # the list of lines that we'll draw
    const newline = Signal([]) # the in-progress line (will be added to list above)

    # Add mouse interactions
    const drawing = Signal(false)  # this will be true if we're dragging a new line
end

sigstart = map(c.mouse.buttonpress) do btn
    if btn.button == 1
        push!(drawing, true)   # start extending the line
        push!(lines, [])
        push!(newline, [btn.position])
    end
end

const dummybutton = MouseButton{UserUnit}()
sigextend = map(filterwhen(drawing, dummybutton, c.mouse.motion)) do btn
    v = value(newline)
    if length(v) > 0
        v = [first(v), btn.position]
        push!(newline, v)
    end
end

sigend = map(c.mouse.buttonrelease) do btn
    if btn.button == 1
        push!(drawing, false)  # stop extending the line
        push!(lines, push!(value(lines), value(newline)))
        push!(newline, [])

        plotselected()
    end
end

# Draw on the canvas
redraw = draw(c, lines, newline) do cnvs, lns, newl
    copy!(cnvs, img)
    set_coordinates(cnvs, BoundingBox(0, 1, 0, 1))  # set coords to 0..1 along each axis
    ctx = getgc(cnvs)
    for l in lns
        drawline(ctx, l, colorant"gray")
    end
    drawline(ctx, newl, colorant"white")
end


Gtk.showall(win)

using MVW, Images
using Plots: plot, heatmap
using DSP
using Printf
import Bresenham

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

## Debug version
#mPN = spawn("../../build", ["-G", "#test\nplane\n", "-s", "glsl/gradient.glsl", "-m"]);
#p = Dict(:dQuad => true)
#r = [ getf(mPN; p...), getf(mPN; p...), getf(mPN; p...) ]

# Complex phasor field, [-1;1] range
phasor = 2. .* channelview(r[2])[1:2,:,:] .- 1.;
phasor = Complex.(phasor[1,:,:], phasor[2,:,:]);

# plot(heatmap(channelview(r[1])[1,:,:], aspectratio=1, yflip=true, c=:grays),
#      heatmap(angle.(phasor), aspectratio=1, yflip=true, c=:phase), layout=2)

angleimg(x) = HSV((angle(x) + pi) / 2pi * 360., 1., 1.)
img = clamp01.(r[1])

function sampleline(data::Matrix{T}, x0::Int, y0::Int, x1::Int, y1::Int) where T
    vals = Vector{T}()

    Bresenham.line(x0, y0, x1, y1) do x,y
        f = data[y, x]
        push!(vals, f)
    end

    return vals
end

function samplergb(data::Array{T, 3}, x0::Int, y0::Int, x1::Int, y1::Int) where T
    vals = []

    Bresenham.line(x0, y0, x1, y1) do x,y
        f = data[1:3, y, x]
        push!(vals, f)
    end

    return transpose(hcat(vals...))
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
        x0 = 1 + floor(Int, a.x.val * s[1])
        y0 = 1 + floor(Int, a.y.val * s[2])
        x1 = 1 + floor(Int, b.x.val * s[1])
        y1 = 1 + floor(Int, b.y.val * s[2])

        noisev = samplergb(2 .* channelview(r[1]) .- 1., x0, y0, x1, y1)
        phasev = sampleline(angle.(phasor), x0, y0, x1, y1)
        Unwrap.unwrap!(phasev)

        display(plot(
            plot(noisev, ylims = (-1,1), title = "Noise value",
                 label = ["R", "G", "B"], lc = [:red :green :blue]),
            plot(phasev, ylims = (-2pi,2pi), shape = (:cross), title = "Phasor phase")))
    end
    nothing
end

using Gtk.ShortNames, GtkReactive, Graphics, Colors

win = Window("Phasor noise phase tool", size(img)...)
c = canvas(UserUnit)       # create a canvas with user-specified coordinates
push!(win, c)

const lines = Signal([])   # the list of lines that we'll draw
const newline = Signal([]) # the in-progress line (will be added to list above)

# Add mouse interactions
const drawing = Signal(false)  # this will be true if we're dragging a new line

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
        p0 = first(v)
        p1 = btn.position

        if btn.modifiers & SHIFT == SHIFT
            dx = p1.x.val - p0.x.val
            dy = p1.y.val - p0.y.val
            if abs(dx) > 2 * abs(dy)
                p1 = XY{UserUnit}(p1.x, p0.y)
            elseif abs(dy) > 2 * abs(dx)
                p1 = XY{UserUnit}(p0.x, p1.y)
            else
                d = min(abs(dx), abs(dy))
                p1 = XY{UserUnit}(p0.x.val + d * sign(dx), p0.y.val + d * sign(dy))
            end
        end

        v = [p0, p1]
        push!(newline, v)
    end
end

sigend = map(c.mouse.buttonrelease) do btn
    if btn.button == 1
        push!(drawing, false)  # stop extending the line
        push!(lines, push!(value(lines), value(newline)))
        push!(newline, [])

        try
            plotselected()
        catch e
            push!(lines, [])
            println(e)
        end

        nothing
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

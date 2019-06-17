module MVW
# Import RPC module
include("RPC.jl")

# An abstract connection to an instance of mvw
abstract type AbstractMvw end

# A connection to an existing instance
struct RemoteMvw <: AbstractMvw
    connection::RPC.Connection
end

# A connection to a spawned instance
mutable struct SpawnedMvw <: AbstractMvw
    connection::RPC.Connection
    bind_addr::AbstractString
    spawn_command::Cmd
    process::Base.Process
end

import Random

function random_bind_addr()
    @static if Sys.iswindows()
        return "tcp://127.0.0.1:" * string(Random.rand(32768:65535))
    else
        tmpdir = haskey(ENV, "TMPDIR") ? ENV["TMPDIR"] : "/tmp"
        return "ipc://$tmpdir/mvw_" * Random.randstring(16) * ".sock"
    end
end

# Connect to a remote instance of mvw
function connect(target::AbstractString)
    RemoteMvw(RPC.connect(target))
end

# Spawn a viewer for the given geometry, shader source and postprocess source
function spawn(base::AbstractString, command)
    cd(base) do
        bind_addr = random_bind_addr()
        spawned_command = vcat(["./viewer"], command, ["--bind", bind_addr])
        spawned_command = `$spawned_command`
        process = open(spawned_command)

        spawned = SpawnedMvw(RPC.connect(bind_addr), bind_addr, spawned_command, process)
        finalizer(s -> kill(s.process), spawned)
        spawned
    end
end

function spawn(fn, base::AbstractString, command)
    spawned = spawn(base, command)
    try
        fn(spawned)
    finally
        finalize(spawned)
    end
end

# Ensure the connection is ok on an abstract viewer instance
function connok(mvw::RemoteMvw)
    # nothing to do here, we only assume it's working
    mvw.connection
end

# Ensure the connection is ok on an abstract viewer instance
function connok(mvw::SpawnedMvw)
    # nothing to do here, we only assume it's working
    mvw.connection
end

function dorpc(f, mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...)
    map(mvw) do item
        f(connok(item), args...; kwargs...)
    end
end

getframe(mvw::AbstractMvw, args...; kwargs...) = RPC.getframe(connok(mvw), args...; kwargs...)
getparams(mvw::AbstractMvw, args...; kwargs...) = RPC.getparams(connok(mvw), args...; kwargs...)
getparam(mvw::AbstractMvw, args...; kwargs...) = RPC.getparam(connok(mvw), args...; kwargs...)
setparam(mvw::AbstractMvw, args...; kwargs...) = RPC.setparam(connok(mvw), args...; kwargs...)
getcamera(mvw::AbstractMvw, args...; kwargs...) = RPC.getcamera(connok(mvw), args...; kwargs...)
setcamera(mvw::AbstractMvw, args...; kwargs...) = RPC.setcamera(connok(mvw), args...; kwargs...)
getrotation(mvw::AbstractMvw, args...; kwargs...) = RPC.getrotation(connok(mvw), args...; kwargs...)
setrotation(mvw::AbstractMvw, args...; kwargs...) = RPC.setrotation(connok(mvw), args...; kwargs...)
getscale(mvw::AbstractMvw, args...; kwargs...) = RPC.getscale(connok(mvw), args...; kwargs...)
setscale(mvw::AbstractMvw, args...; kwargs...) = RPC.setscale(connok(mvw), args...; kwargs...)
geometry(mvw::AbstractMvw, args...; kwargs...) = RPC.geometry(connok(mvw), args...; kwargs...)
loaddefaults(mvw::AbstractMvw, args...; kwargs...) = RPC.loaddefaults(connok(mvw), args...; kwargs...)

getframe(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.getframe, mvw, args...; kwargs...)
getparams(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.getparams, mvw, args...; kwargs...)
getparam(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.getparam, mvw, args...; kwargs...)
setparam(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.setparam, mvw, args...; kwargs...)
getcamera(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.getcamera, mvw, args...; kwargs...)
setcamera(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.setcamera, mvw, args...; kwargs...)
getrotation(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.getrotation, mvw, args...; kwargs...)
setrotation(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.setrotation, mvw, args...; kwargs...)
getscale(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.getscale, mvw, args...; kwargs...)
setscale(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.setscale, mvw, args...; kwargs...)
geometry(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.geometry, mvw, args...; kwargs...)
loaddefaults(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...) = dorpc(RPC.loaddefaults, mvw, args...; kwargs...)

function getf(mvw::AbstractMvw; target::AbstractString = "", size::Tuple{Int, Int} = (256, 256), kwargs...)
    if length(kwargs) > 0
        loaddefaults(mvw)

        for (k,v) in kwargs
            setparam(mvw, string(k), v)
        end
    end

    getframe(mvw, target=target, size=size)
end

function getf(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...)
    map(mvw) do item
        getf(item, args...; kwargs...)
    end
end

function getp(mvw::AbstractMvw)
    map(getparams(mvw)) do param
        (Symbol(param["s_name"]), param["value"])
    end
end

function getp(mvw::AbstractArray{AbstractMvw,1}, args...; kwargs...)
    map(mvw) do item
        getp(item, args...; kwargs...)
    end
end

export AbstractMvw, RemoteMvw, SpawnedMvw, connect, spawn, getframe, getparams, getparam, setparam, getcamera, setcamera, getrotation, setrotation, getscale, setscale, geometry, loaddefaults, getf, getp
end # module

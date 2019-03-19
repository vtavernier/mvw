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

# Default bind address for a remote instance of mvw
function default_bind_addr()
    @static if Sys.iswindows()
        return "tcp://127.0.0.1:7178"
    else
        tmpdir = haskey(ENV, "TMPDIR") ? ENV["TMPDIR"] : "/tmp"
        return "ipc://$tmpdir/mvw_default.sock"
    end
end

import Random

function random_bind_addr()
    @static if Sys.iswindows()
        return "tcp://127.0.0.1:" * string(Random.rand(32768:65535))
    else
        tmpdir = haskey(ENV, "TMPDIR") ? ENV["TMPDIR"] : "/tmp"
        return "ipc://$tmpdir/" * Random.randstring(16)
    end
end

# Connect to a remote instance of mvw
function connect(target::AbstractString = default_bind_addr())
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

function spawn(fn, command)
    spawned = spawn(command)
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

export AbstractMvw, RemoteMvw, SpawnedMvw, connect, spawn, getframe, getparams, getparam, setparam, getcamera, setcamera, getrotation, setrotation, getscale, setscale, geometry
end # module

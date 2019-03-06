module MVW
# Import RPC module
include("RPC.jl")

# An abstract connection to an instance of mvw
abstract type AbstractMvw end

# A connection to an existing instance
struct RemoteMvw <: AbstractMvw
    connection::RPC.Connection
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

# Connect to a remote instance of mvw
function connect(target::AbstractString = default_bind_addr())
    RemoteMvw(RPC.connect(target))
end

getframe(mvw::RemoteMvw, args...; kwargs...) = RPC.getframe(mvw.connection, args...; kwargs...)
getparams(mvw::RemoteMvw, args...; kwargs...) = RPC.getparams(mvw.connection, args...; kwargs...)
getparam(mvw::RemoteMvw, args...; kwargs...) = RPC.getparam(mvw.connection, args...; kwargs...)
setparam(mvw::RemoteMvw, args...; kwargs...) = RPC.setparam(mvw.connection, args...; kwargs...)

export connect, getframe, getparams, getparam, setparam
end # module

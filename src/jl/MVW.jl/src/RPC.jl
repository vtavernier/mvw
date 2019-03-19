module RPC
import Images, ZMQ, MsgPack
export Connection, connect, getframe, getparams, getparam, setparam

struct Connection
    socket::ZMQ.Socket
end

function connect(target::AbstractString)
    # Establish connection with REQ socket
    connection = Connection(ZMQ.Socket(ZMQ.REQ))
    ZMQ.connect(connection.socket, target)
    connection
end

function getframe(connection::Connection; target::AbstractString = "", size::Tuple{Int, Int} = (256, 256))
    # Send getframe request
    msg = ZMQ.Message("getframe")
    ZMQ.send(connection.socket, msg, more=true)

    # Send args
    ZMQ.send(connection.socket, MsgPack.pack((target, size)))

    # Fetch response
    (success, details) = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if success
        # Fetch image format
        (width, height) = map(k -> details[k], ["width", "height"])

        # Fetch image data
        image_data = ZMQ.recv(connection.socket, Vector{Float32})
        @assert length(image_data) == width * height * 4

        # Make image object
        Images.colorview(Images.RGBA, PermutedDimsArray(reverse(reshape(image_data, (4, width, height)), dims=3), (1,3,2)))
    else
        error("getframe failed: " * details)
    end
end

function getparams(connection::Connection)
    # Send getparams request
    msg = ZMQ.Message("getparams")
    ZMQ.send(connection.socket, msg)

    # Fetch response
    (success, details) = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if success
        details
    else
        error("getparams failed: " * details)
    end
end

function getparam(connection::Connection, param::AbstractString)
    # Send getparam request
    msg = ZMQ.Message("getparam")
    ZMQ.send(connection.socket, msg; more=true)

    # Send argument
    msg = ZMQ.Message(MsgPack.pack(param))
    ZMQ.send(connection.socket, msg)

    # Fetch response
    (success, details) = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if success
        details
    else
        error("getparam failed: " * details)
    end
end

function setparam(connection::Connection, param::AbstractString, value)
    # Send setparam request
    msg = ZMQ.Message("setparam")
    ZMQ.send(connection.socket, msg; more=true)

    # Send argument
    msg = ZMQ.Message(MsgPack.pack((param, value)))
    ZMQ.send(connection.socket, msg)

    # Fetch response
    response = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if response isa Bool
    else
        (success, details) = response
        error("setparam failed: " * details)
    end
end

function getcamera(connection::Connection)
    # Send getcamera request
    msg = ZMQ.Message("getcamera")
    ZMQ.send(connection.socket, msg)

    # Get result
    (success, details) = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if success
        details
    else
        error("getcamera failed: " * details)
    end
end

function setcamera(connection::Connection, location, target, up)
    # Send setcamera request
    msg = ZMQ.Message("setcamera")
    ZMQ.send(connection.socket, msg; more=true)

    # Send argument
    msg = ZMQ.Message(MsgPack.pack((location, target, up)))
    ZMQ.send(connection.socket, msg)

    # Fetch response
    response = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if response isa Bool
    else
        (success, details) = response
        error("setcamera failed: " * details)
    end
end

function getrotation(connection::Connection)
    # Send getrotation request
    msg = ZMQ.Message("getrotation")
    ZMQ.send(connection.socket, msg)

    # Get result
    (success, details) = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if success
        details
    else
        error("getrotation failed: " * details)
    end
end

function setrotation(connection::Connection, rotation)
    # Send setrotation request
    msg = ZMQ.Message("setrotation")
    ZMQ.send(connection.socket, msg; more=true)

    # Send argument
    msg = ZMQ.Message(MsgPack.pack(rotation))
    ZMQ.send(connection.socket, msg)

    # Fetch response
    response = MsgPack.unpack(ZMQ.recv(connection.socket, Vector{UInt8}))

    if response isa Bool
    else
        (success, details) = response
        error("setrotation failed: " * details)
    end
end
end

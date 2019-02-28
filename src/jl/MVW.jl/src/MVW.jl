module MVW
    import Images, ZMQ
    export Connection, connect, getframe

    struct Connection
        socket::ZMQ.Socket
    end

    function default_bind_addr()
        @static if Sys.iswindows()
            return "tcp://127.0.0.1:7178"
        else
            tmpdir = haskey(ENV, "TMPDIR") ? ENV["TMPDIR"] : "/tmp"
            return "ipc://$tmpdir/mvw_default.sock"
        end
    end

    function connect(target = default_bind_addr())
        # Establish connection with REQ socket
        connection = Connection(ZMQ.Socket(ZMQ.REQ))
        ZMQ.connect(connection.socket, target)
        connection
    end

    function getframe(connection::Connection)
        # Send getframe request
        msg = ZMQ.Message("getframe")
        ZMQ.send(connection.socket, msg)

        # Fetch response
        status = unsafe_string(ZMQ.recv(connection.socket))

        if status == "success"
            # Fetch image format
            (width, height, internal_format) =
                ZMQ.recv(connection.socket, Vector{Int32})

            # Fetch image data
            image_data = ZMQ.recv(connection.socket, Vector{Float32})
            @assert length(image_data) == width * height * 4

            # Make image object
            Images.colorview(Images.RGBA, PermutedDimsArray(reverse(reshape(image_data, (4, width, height)), dims=3), (1,3,2)))
        else
            error("getframe failed: " * unsafe_string(ZMQ.recv(connection.socket)))
        end
    end
end # module

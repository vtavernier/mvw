using Test
using MVW

c = connect()
@test getparams(c) isa Array{Any, 1}



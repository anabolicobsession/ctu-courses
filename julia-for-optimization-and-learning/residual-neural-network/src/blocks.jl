export Block, BasicBlock, Bottleneck

abstract type Block end

struct BasicBlock <: Block
    chain::Chain
    channels::Pair{Integer, Integer}
    stride::Integer
end

Flux.@functor BasicBlock

function BasicBlock(channels::Pair{T, T}, stride::Integer, connection::Function) where {T <: Integer}
    before_connection = Chain(
        Conv((3, 3), channels; stride, pad = 1, bias = false),
        BatchNorm(channels[2], relu),
        Conv((3, 3), channels[2] => channels[2]; pad = 1, bias = false),
        BatchNorm(channels[2]),
    )
    chain = Chain(SkipConnection(before_connection, connection), relu)
    BasicBlock(chain, channels, stride)
end

(bb::BasicBlock)(x) = bb.chain(x)

Base.show(io::IO, bb::BasicBlock) = print(io, typeof(bb), "(", bb.channels[1], " => ", bb.channels[2], ", stride=", bb.stride, ")")

struct Bottleneck <: Block
    chain::Chain
    channels::Pair{Integer, Integer}
    stride::Integer
    expansion::Integer
end

Flux.@functor Bottleneck

function Bottleneck(channels::Pair{T, T}, stride::Integer, connection::Function; expansion::Integer = 1) where {T <: Integer}
    before_connection = Chain(
        Conv((1, 1), channels, bias = false),
        BatchNorm(channels[2], relu),
        Conv((3, 3), channels[2] => channels[2]; stride, pad = 1, bias = false),
        BatchNorm(channels[2], relu),
        Conv((1, 1), channels[2] => channels[2] * expansion, bias = false),
        BatchNorm(channels[2] * expansion),
    )
    chain = Chain(SkipConnection(before_connection, connection), relu)
    Bottleneck(chain, channels, stride, expansion)
end

(bn::Bottleneck)(x) = bn.chain(x)

Base.show(io::IO, bn::Bottleneck) = print(io, typeof(bn), "(", bn.channels[1], " => ", bn.channels[2], ", stride=", bn.stride, ", expansion=", bn.expansion, ")")
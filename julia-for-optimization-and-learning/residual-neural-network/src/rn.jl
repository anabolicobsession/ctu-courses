export RN

struct RN
    entry::Chain
    layers::Chain
    head::Chain
    n_layers::Integer
end

Flux.@functor RN

"""
    RN(; 
        block,
        channels::Vector, 
        strides::Vector, 
        repeats::Vector, 
        classes::Integer, 
        grayscale = false,
        pooling_dims = (7, 7),
        kwargs...
    )

Constructs ResNet, the network which utilizes residual connections to mitigate the
vanishing gradient problem, make identity functions easier to learn and thus build
really deep networks.

`block` accepts **BasicBlock** or **Bottleneck**.
The length of `channels` must be 1 more than the length of `strides` or `repeats`.
`pooling_dims` defines output spatial dimensions of the final pooling.
To use `expansion` other than 1 with **Bottleneck** block, pass the corresponding keyword argument.
Other arguments are self explanatory.

# Example

```julia-repl
julia> RN(
    block = BasicBlock,
    channels = [2, 4, 8], 
    strides = [2, 2], 
    repeats = [2, 2], 
    grayscale = true,
    classes = 10
)
ResNet with 10 layers and 2738 parameters

Entry
    Conv((3, 3), 1 => 2, pad=1, bias=false)
    BatchNorm(2, relu)
Layer1
    BasicBlock(2 => 4, stride=2)
    BasicBlock(4 => 4, stride=1)
Layer2
    BasicBlock(4 => 8, stride=2)
    BasicBlock(8 => 8, stride=1)
Head
    AdaptiveMeanPool((1, 1))
    flatten
    Dense(8 => 10; bias=false)
    BatchNorm(10)
    logsoftmax

julia> RN(
    block = Bottleneck,
    channels = [32, 64, 128, 256], 
    strides = [1, 2, 2], 
    repeats = [4, 4, 4], 
    grayscale = false,
    classes = 20,
    expansion = 4
)
ResNet with 38 layers and 5713864 parameters

Entry
    Conv((3, 3), 3 => 32, pad=1, bias=false)
    BatchNorm(32, relu)
Layer1
    Bottleneck(32 => 64, stride=1, expansion=4)
    Bottleneck(256 => 64, stride=1, expansion=4)
    Bottleneck(256 => 64, stride=1, expansion=4)
    Bottleneck(256 => 64, stride=1, expansion=4)
Layer2
    Bottleneck(256 => 128, stride=2, expansion=4)
    Bottleneck(512 => 128, stride=1, expansion=4)
    Bottleneck(512 => 128, stride=1, expansion=4)
    Bottleneck(512 => 128, stride=1, expansion=4)
Layer3
    Bottleneck(512 => 256, stride=2, expansion=4)
    Bottleneck(1024 => 256, stride=1, expansion=4)
    Bottleneck(1024 => 256, stride=1, expansion=4)
    Bottleneck(1024 => 256, stride=1, expansion=4)
Head
    AdaptiveMeanPool((1, 1))
    flatten
    Dense(1024 => 20; bias=false)
    BatchNorm(20)
    logsoftmax
```
"""
function RN(; 
        block,
        channels::Vector, 
        strides::Vector, 
        repeats::Vector, 
        classes::Integer, 
        grayscale = false,
        kwargs...
    )

    length(channels) - length(strides) == 1 || throw(DomainError(length(channels) - length(strides), "The number of channels must be 1 more than the number of strides"))
    length(strides) == length(repeats) || throw(DomainError(length(strides) - length(repeats), "The number of strides must be the same as the number of repeats"))

    entry = Chain(
        Conv((3, 3), (grayscale ? 1 : 3) => channels[1], pad = 1, bias = false),
        BatchNorm(channels[1], relu)
    )

    layers = []
    in_channels = channels[1]
    expansion = get(Dict(kwargs), :expansion, 1)
    
    for (out_channels, stride, repeat) in zip(channels[2:end], strides, repeats)
        push!(layers, Layer(block, in_channels => out_channels, stride, repeat; kwargs...))
        in_channels = out_channels * expansion
    end
    layers = Chain(layers...)

    head = Chain(
        AdaptiveMeanPool((1, 1)),
        Flux.flatten,
        Dense(channels[end] * expansion => classes, bias = false),
        BatchNorm(classes),
        logsoftmax
    )

    layers_per_block = block == BasicBlock ? 2 : 3
    n_layers = 2 + sum(layers_per_block .* repeats)

    RN(entry, layers, head, n_layers)
end

(rn::RN)(x) = rn.head(rn.layers(rn.entry(x)))

function Base.show(io::IO, rn::RN)
    println(io, "ResNet with ", rn.n_layers, " layers and ", number_of_params(rn), " parameters\n")
    
    println(io, "Entry")
    for layer in rn.entry
        print(io, " " ^ 4)
        println(io, layer)
    end

    for (i, layer) in enumerate(rn.layers)
        println(io, "Layer", i)

        for block in layer.blocks
            print(io, " " ^ 4)
            println(io, block)
        end
    end

    println(io, "Head")
    for layer in rn.head
        print(io, " " ^ 4)
        println(io, layer)
    end
end
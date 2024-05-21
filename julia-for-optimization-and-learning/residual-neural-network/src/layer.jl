struct Layer
    blocks::Chain
end

Flux.@functor Layer

function Layer(block, channels, stride, repeat; kwargs...)
    blocks = []
    expansion = get(Dict(kwargs), :expansion, 1)

    if stride == 1 && channels[1] == channels[2] * expansion
        push!(blocks, block(channels, +, stride; kwargs...))
    else
        downsample = Chain(
            Conv((1, 1), channels[1] => channels[2] * expansion; stride, bias = false),
            BatchNorm(channels[2] * expansion)
        )
        push!(blocks, block(channels, stride, (x_out, x_in) -> x_out + downsample(x_in); kwargs...))
    end

    for _ in 2:repeat
        push!(blocks, block(channels[2] * expansion => channels[2], 1, +; kwargs...))
    end

    Layer(Chain(blocks...))
end

(l::Layer)(x) = l.blocks(x)
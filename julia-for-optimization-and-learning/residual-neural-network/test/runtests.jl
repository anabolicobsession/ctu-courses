using ResNet
using Test
using Flux: unsqueeze

@testset "ResNet.jl" begin
    xs = [
        ones(Float32, 28, 28, 1, 1), 
        ones(Float32, 32, 32, 1, 5),
        ones(Float32, 35, 35, 1, 100),
        ones(Float32, 100, 100, 1, 1)
    ]
    
    m = RN(
        block = BasicBlock,
        channels = [2, 4, 8], 
        strides = [2, 2], 
        repeats = [2, 2], 
        grayscale = true,
        classes = 10
    )
    @testset "$ResNet composed of 4 BasicBlocks with 10 classes, size(x) = $(size(x))" for x in xs
        @test size(m(x)) == (10, size(x, 4))
    end
    
    m = RN(
        block = BasicBlock,
        channels = [2, 4, 8, 16], 
        strides = [1, 2, 2], 
        repeats = [2, 2, 2], 
        grayscale = true,
        classes = 20
    )
    @testset "$ResNet composed of 6 BasicBlock with 20 classes, size(x) = $(size(x))" for x in xs
        @test size(m(x)) == (20, size(x, 4))
    end
    
    m = RN(
        block = Bottleneck,
        channels = [2, 4, 8], 
        strides = [2, 2], 
        repeats = [2, 2], 
        grayscale = true,
        classes = 10,
        expansion = 1
    )
    @testset "$ResNet composed of 4 Bottleneck with expansion = 1, size(x) = $(size(x))" for x in xs
        @test size(m(x)) == (10, size(x, 4))
    end
    
    m = RN(
        block = Bottleneck,
        channels = [2, 4, 8, 16], 
        strides = [1, 2, 2], 
        repeats = [3, 3, 3], 
        grayscale = true,
        classes = 10,
        expansion = 4
    )
    @testset "$ResNet composed of 9 Bottleneck with expansion = 4, size(x) = $(size(x))" for x in xs
        @test size(m(x)) == (10, size(x, 4))
    end
end

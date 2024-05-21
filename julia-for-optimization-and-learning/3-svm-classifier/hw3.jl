# IN SOME FUNCTIONS, IT MAY BE NECESSARY TO ADD KEYWORD ARGUMENTS

computeQ(X, y) = (Xy = X .* y; Xy * Xy')
computeW(X, y, z) = vec(sum(y .* z .* X; dims=1))

function solve_SVM_dual(Q, C; max_epoch=10)
    n = size(Q, 1)
    z = C .* rand(n)

    # simplified function, for which the next statement is true:
    # argmin(f) = argmin(f_dual), where f_dual is the original function
    function f(i, d)
        s = sum(z .* (Q[:, i] .+ Q[i, :]))
        return d * (Q[i, i] * d + s - 2)
    end

    # returns the value d which minimizes the previous function
    function f_argmin(i)
        s = sum(z .* (Q[:, i] .+ Q[i, :]))
        return -(s - 2) / (2 * Q[i, i])
    end

    for _ in 1:max_epoch
        for i in 1:n
            # bounds of function interval
            ds = [-z[i], -z[i] + C]
            
            d_min = f_argmin(i)
            if d_min > min(ds[1], ds[2]) && d_min < max(ds[1], ds[2])
                append!(ds, d_min)
            end

            ys = map(d -> f(i, d), ds)
            z[i] += ds[argmin(ys)]
        end
    end

    return z
end

solve_SVM(X, y, C; kwargs...) = computeW(X, y, solve_SVM_dual(computeQ(X, y), C; kwargs...))

# using Plots

# function plot_SVM(X, y, w; 
#         heatmap_step = 0.02,
#         colours = [:aqua, :plum]
#     )
#     xs = minimum(X[:, 2]):heatmap_step:maximum(X[:, 2])
#     ys = minimum(X[:, 3]):heatmap_step:maximum(X[:, 3])

#     plt = heatmap(xs, ys, (x, y) -> w' * [1, x, y] >= 0;
#         color = colours,
#         opacity = 0.2,
#         cbar = false,
#         axis = false,
#         ticks = false
#     )

#     for (i, class) in enumerate([-1, 1])
#         ids = findall(y .== class)
#         scatter!(plt, X[ids, 2], X[ids, 3];
#             label = class,
#             marker=(4, 0.9, colours[i]),
#         )
#     end
    
#     display(plt)
# end

using Statistics

intercept(X) = cat(ones(size(X, 1)), X; dims=2)

normalize(X; dims=1) = (X .- mean(X; dims)) ./ std(X; dims)

using RDatasets

function iris(C; kwargs...)
    data = dataset("datasets", "iris")
    indices = data.Species .!= "setosa"
    features = data[indices, [:PetalLength, :PetalWidth]]
    X = intercept(normalize(Matrix(features)))
    y = map(s -> s == "versicolor" ? 1 : -1, data.Species[indices])
    return solve_SVM(X, y, C; kwargs...)
end
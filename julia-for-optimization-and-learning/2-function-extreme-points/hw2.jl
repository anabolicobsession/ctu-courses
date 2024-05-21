using Distributions

function optim(f, g, P, x; α=0.01, max_iter=10000)
    for _ in 1:max_iter
        y = x - α*g(x)
        x = P(y)
    end
    return x
end

P(x, x_min, x_max) = min.(max.(x, x_min), x_max)

function has_point(M::AbstractMatrix, p::AbstractVector; ϵ=1e-3)
    size(M, 2) == 0 && return false
    norms = sum((M .- p) .^ 2; dims=1)
    return any(norms .<= ϵ^2)
end

function generate_solutions(f, g, P, x_min, x_max, ndims; n_init_points=4000)
    init_points = rand(Uniform(x_min, x_max), (ndims, n_init_points))
    solutions = zeros(ndims, 0)

    for p in eachcol(init_points)
        found_solution = optim(f, g, x -> P(x, x_min, x_max), p)
        
        if !has_point(solutions, found_solution)
            solutions = hcat(solutions, found_solution)
        end
    end

    return solutions
end

generate_solutions(f, g, P, x_min::Real, x_max::Real; kwargs...) = generate_solutions(f, g, P, x_min, x_max, 1; kwargs...)

generate_solutions(f, g, P, x_min::AbstractVector, x_max::AbstractVector; kwargs...) = generate_solutions(f, g, P, x_min[1], x_max[1], length(x_min); kwargs...)

generate_solutions(f, g, x_min, x_max; kwargs...) = generate_solutions(f, g, P, x_min, x_max; kwargs...)

# If you need to write multiple methods based on input types, it is fine.
f_griewank(x) = 1 + 1/4000 * sum(x.^2) - prod(cos.(x ./ sqrt.(1:length(x))))

# If you manage to write only one method, it is fine.
function g_griewank(x::AbstractVector)
    g = 1/2000 * x
    cosines = cos.(x ./ sqrt.(1:length(x)))

    for i in 1:lastindex(x)
        g[i] += sin(x[i] / sqrt(i)) * prod(cosines[1:end .!= i])
    end

    return g
end

g_griewank(x::Real) = 1/2000*x + sin(x)
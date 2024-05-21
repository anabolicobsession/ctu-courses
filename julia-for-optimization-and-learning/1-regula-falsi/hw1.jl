# IN SOME FUNCTIONS, IT MAY BE NECESSARY TO ADD KEYWORD ARGUMENTS

abstract type BracketingMethod end

struct Bisection <: BracketingMethod end
struct RegulaFalsi <: BracketingMethod end

midpoint(::Bisection, f, a, b) = (a + b) / 2
midpoint(::RegulaFalsi, f, a, b) = (a * f(b) - b * f(a)) / (f(b) - f(a))

function findroot(
    method::BracketingMethod,
    f::Function,
    a::Real,
    b::Real;
    atol::Real = 1e-8,
    maxiter::Integer = 1000
)
    if abs(f(a)) < atol
        return a
    elseif abs(f(b)) < atol
        return b
    end

    if sign(f(a)) == sign(f(b))
        throw(DomainError((f(a), f(b)), "function values of the bounds have the same sign"))
    end

    c = 0

	if a > b
        a, b = b, a
    end

    for _ in 1:maxiter
        c = midpoint(method, f, a, b)

        if sign(f(a)) != sign(f(c))
            b = c
        else
            a = c
        end

        if abs(f(c)) < atol
            return c
        end
    end

    return c
end

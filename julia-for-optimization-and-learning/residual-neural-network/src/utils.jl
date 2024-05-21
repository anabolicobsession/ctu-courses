export number_of_params

using Flux: params

number_of_params(m) = sum(length, params(m))
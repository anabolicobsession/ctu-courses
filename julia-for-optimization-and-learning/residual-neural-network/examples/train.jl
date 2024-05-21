using Flux
using Flux: onehotbatch
using Dates

accuracy(y_out, y) = sum((argmax.(eachcol(y_out)) .- 1) .== y) / length(y)

function train!(model, train_tuple, val_tuple, η = 0.01, epochs = 1, batch_size = 128; verbose = false)
    train_x, train_y = train_tuple
    val_x, val_y = val_tuple
    train_y_oh = onehotbatch(train_y, 0:9)
    val_y_oh = onehotbatch(val_y, 0:9)

    opt_state = Flux.setup(Adam(η), rn)
    loader = Flux.DataLoader((train_x, train_y_oh), batchsize = batch_size, shuffle = true)
    loss_fun = Flux.Losses.logitcrossentropy

    train_losses, train_accs = Float32[], Float32[]
    val_losses, val_accs = Float32[], Float32[]

    for epoch in 1:epochs
        !verbose || println(now(), ": Running epoch ", epoch, "...")

        Flux.trainmode!(model)

        Flux.train!(model, loader, opt_state) do m, x, y
            loss_fun(m(x), y)
        end

        Flux.testmode!(model)

        train_out = rn(train_x)
        val_out = rn(val_x)
        
        push!(train_losses, loss_fun(train_out, train_y_oh))
        push!(val_losses, loss_fun(val_out, val_y_oh))
        push!(train_accs, accuracy(train_out, train_y))
        push!(val_accs, accuracy(val_out, val_y))
    end 

    return (train_losses, train_accs), (val_losses, val_accs)
end
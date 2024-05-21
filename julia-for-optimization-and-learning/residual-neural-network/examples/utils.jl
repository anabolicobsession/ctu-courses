using BSON: @load, @save

function load_if_exists!(model, path)
    if isfile(path)
        @load path model
        return true
    end
    false
end

function save(model, path)
    @save path model
end
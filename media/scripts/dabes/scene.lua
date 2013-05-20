require 'dabes.bound_object'

SceneBinding = {
    new = function(i_params)
        -- Init the actual scene from the binding
        return dab_scene.new(i_params.kind, i_params.pixels_per_meter)
    end
}

Scene = BoundObject.extend({
    binding = SceneBinding
})

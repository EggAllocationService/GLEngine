@group(1) @binding(0)
var color: texture_2d<f32>;
@group(1) @binding(1)
var depth: texture_depth_2d;
@group(1) @binding(2)
var samp: sampler;


@fragment
fn post(@location(0) uv: vec2f) -> @location(0) vec4f {
    let color = textureSample(color, samp, uv);
    return vec4f(1 - color.xyz, 1);
}


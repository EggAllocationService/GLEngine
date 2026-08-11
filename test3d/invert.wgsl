// available to post:
// color: texture_2d
// depth: texture_depth_2d
// samp: sampler
// linear_depth(pos: vec2f) -> depth at UV in world units
// global.time

@fragment
fn post(@location(0) uv: vec2f) -> @location(0) vec4f {
    let d = linear_depth(uv);
    let c = textureSample(color, samp, uv);

    return vec4f(mix(c.xyz, 1 - c.xyx, saturate(d / 5)), 1);
}


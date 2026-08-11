struct RenderUniforms {
    projectionViewMatrix: mat4x4<f32>,
    projectionMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    lightCount: i32,
    time: f32
}

@group(0) @binding(0)
var<uniform> global: RenderUniforms;

@group(1) @binding(0)
var color: texture_2d<f32>;
@group(1) @binding(1)
var depth: texture_depth_2d;
@group(1) @binding(2)
var samp: sampler;

fn linear_depth(pos: vec2f) -> f32 {
    let raw = textureSample(depth, samp, pos);
    let P22 = global.projectionMatrix[2][2];
    let P23 = global.projectionMatrix[3][2];
    return P23 / (raw - P22);
}

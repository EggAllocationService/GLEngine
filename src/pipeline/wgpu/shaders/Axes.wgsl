struct RenderUniforms {
    projectionViewMatrix: mat4x4<f32>,
    projectionMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    lightCount: i32
}
struct ModelData {
    m: mat4x4<f32>
}
@group(0)
@binding(0)
var<uniform> camera: RenderUniforms;

var<immediate> m: ModelData;

struct VertexIn {
    @location(0) pos: vec3f,
    @location(1) color: vec3f
}

struct VertexOut {
    @builtin(position) pos: vec4f,
    @location(0) color: vec3f
}

@vertex
fn vs(i: VertexIn) -> VertexOut {
    var result: VertexOut;
    let MVP = camera.projectionViewMatrix * m.m;
    result.pos = MVP * vec4f(i.pos, 1.0);
    result.color = i.color;
    return result;
}

@fragment
fn fs(i: VertexOut) -> @location(0) vec4f {
    return vec4f(i.color, 1);
}


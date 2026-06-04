struct RenderUniforms {
    projectionViewMatrix: mat4x4<f32>,
    projectionMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    lightCount: i32
}
struct ModelData {
    m: mat4x4<f32>,
    color: vec4f
}
@group(0)
@binding(0)
var<uniform> camera: RenderUniforms;

var<immediate> m: ModelData;

struct VertexIn {
    @location(0) pos: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f
}

@vertex
fn vs(i: VertexIn) -> @builtin(position) vec4f {
    let MVP = camera.projectionViewMatrix * m.m;
    var transformed = MVP * vec4f(i.pos, 1.0);
    transformed.z = 0;
    return transformed;
}

@fragment
fn fs() -> @location(0) vec4f {
    return m.color;
}


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
    @location(1) normal: vec3f,
    @location(2) uv: vec2f
}
struct VertexOut {
    @builtin(position) pos: vec4f,
    @location(1) normal: vec4f
}

@vertex
fn vs(i: VertexIn) -> VertexOut {
    var result: VertexOut;
    let MVP = (camera.projectionViewMatrix * m.m);
    result.pos = MVP * vec4f(i.pos, 1.0);
    result.normal = m.m * vec4f(i.normal, 0.0);
    return result;
}

@fragment
fn fs(i: VertexOut) -> @location(0) vec4f {
    return vec4f(abs(normalize(i.normal)).xyz, 1.0);
}


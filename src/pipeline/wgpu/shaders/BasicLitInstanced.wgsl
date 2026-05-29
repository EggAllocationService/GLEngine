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

@group(1)
@binding(0)
var<storage, read> m: array<ModelData>;

struct VertexIn {
    @location(0) pos: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
    @builtin(instance_index) instance: u32
}
struct VertexOut {
    @builtin(position) pos: vec4f,
    @location(1) normal: vec4f
}

@vertex
fn vs(i: VertexIn) -> VertexOut {
    var result: VertexOut;
    let model = m[i.instance].m;
    let MVP = camera.projectionViewMatrix * model;

    result.pos = MVP * vec4f(i.pos, 1.0);
    result.normal = model * vec4f(i.normal, 0.0);
    return result;
}

@fragment
fn fs(i: VertexOut) -> @location(0) vec4f {
    return abs(normalize(i.normal));
}


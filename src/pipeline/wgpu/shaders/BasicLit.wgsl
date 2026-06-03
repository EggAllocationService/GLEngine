struct RenderUniforms {
    projectionViewMatrix: mat4x4<f32>,
    projectionMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    lightCount: i32
}

struct Material {
    diffuse: vec4f,
    ambient: vec4f,
    specular: vec4f,
    shininess: f32
}

struct LightInfo {
    position: vec4f,
    diffuse: vec4f,
    specular: vec4f,
    ambient: vec4f
}

struct ModelData {
    m: mat4x4<f32>
}

@group(0)
@binding(0)
var<uniform> camera: RenderUniforms;

@group(0)
@binding(1)
var<storage, read> lights: array<LightInfo>;

@group(1)
@binding(0)
var<uniform> material: Material;

var<immediate> m: ModelData;

struct VertexIn {
    @location(0) pos: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f
}
struct VertexOut {
    @builtin(position) pos: vec4f,
    @location(0) worldPos: vec3f,
    @location(1) normal: vec4f
}

@vertex
fn vs(i: VertexIn) -> VertexOut {
    var result: VertexOut;
    result.worldPos = (m.m * vec4f(i.pos, 1.0)).xyz;
    result.pos = camera.projectionViewMatrix * vec4f(result.worldPos, 1.0);
    result.normal = m.m * vec4f(i.normal, 0.0);
    return result;
}

@fragment
fn fs(v: VertexOut) -> @location(0) vec4f {
    var color = vec3f(0);
    let normal = normalize(v.normal.xyz);

    for (var i = 0; i < camera.lightCount; i++) {
        var lightDir = vec3(0.0, 0.0, 0.0);
        if (lights[i].position.w == 0) {
            // directional light
            lightDir = normalize(lights[i].position.xyz);
        } else {
            lightDir = normalize(lights[i].position.xyz - v.worldPos.xyz);
        }

        let diff = lights[i].diffuse.xyz * max(dot(normal, lightDir), 0);
        var result = (lights[i].ambient.xyz * material.diffuse.xyz) + (diff * material.diffuse.xyz);
        if (lights[i].position.w == 1) {
            // point light, calculate attenuation
            let distance = length(lights[i].position.xyz - v.worldPos.xyz);
            result /= (0.5 * distance + (distance * distance));
        }

        color += result;
    }
    color += material.ambient.xyz;

    return vec4f(color, 1);
}


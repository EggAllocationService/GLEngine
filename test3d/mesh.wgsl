enable wgpu_mesh_shader;

struct RenderUniforms {
    projectionViewMatrix: mat4x4<f32>,
    projectionMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    lightCount: i32
}

@group(0)
@binding(0)
var<uniform> camera: RenderUniforms;


const positions = array(
    vec4f(1, 0, 0, 1),
    vec4f(0, 0, 0, 1),
    vec4f(0, 0, 1, 0),
    vec4f(1, 0, 1, 1),
    vec4f(1, 1, 0, 1),
    vec4f(0, 1, 0, 1),
    vec4f(0, 1, 1, 0),
    vec4f(1, 1, 1, 1),
);
struct Vertex {
    @builtin(position) position: vec4<f32>
}
struct Primitive {
    @builtin(triangle_indices) indices: vec3<u32>
}
struct MeshOutput {
    @builtin(vertices) vertices: array<Vertex, 8>,
    @builtin(primitives) primitives: array<Primitive, 12>,
    @builtin(vertex_count) vertex_count: u32,
    @builtin(primitive_count) primitive_count: u32,
}

var<workgroup> mesh_output: MeshOutput;
var<immediate> m: mat4x4<f32>;

@mesh(mesh_output)
@workgroup_size(4, 1, 1)
fn mesh(@builtin(local_invocation_id) idx: vec3u) {
    let mvp = camera.projectionViewMatrix * m;

    mesh_output.vertices[idx.x].position = mvp * positions[idx.x];

    if (idx.x == 0) {
        mesh_output.vertex_count = 4;
        mesh_output.primitive_count = 2;
        mesh_output.primitives[0].indices = vec3u(0, 1, 2);
        mesh_output.primitives[1].indices = vec3u(1, 2, 3);
    }
}

@fragment
fn fs() -> @location(0) vec4f {
    return vec4f(1, 0, 1, 1);
}
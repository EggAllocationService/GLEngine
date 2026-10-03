enable wgpu_mesh_shader;
enable wgpu_int16;

struct RenderUniforms {
    projectionViewMatrix: mat4x4<f32>,
    cameraMatrix: mat4x4<f32>,
    viewMatrix: mat4x4<f32>,
    lightCount: i32,
    time: f32
}

struct Meshlet {
    origin: vec4<f32>,
    coneApex: vec4<f32>,
    coneAxis: vec4<f32>,
    verticesOffset: u32,
    verticesCount: u32,
    indicesOffset: u32,
    indicesCount: u32,
}

struct MeshVertex {
    position: vec3<f32>,
    normal: vec3<f32>,
    uv: vec2<f32>
}

struct Instance {
    transform: mat4x4<f32>
}

@group(0)
@binding(0)
var<uniform> camera: RenderUniforms;

@group(1)
@binding(0)
var<storage, read> meshlets: array<Meshlet>;
@group(1)
@binding(1)
var<storage, read> vertices: array<MeshVertex>;
@group(1)
@binding(2)
var<storage, read> indices: array<u16>;

@group(2)
@binding(0)
var<storage, read> instances: array<Instance>;

struct Vertex {
    @builtin(position) position: vec4<f32>,
    @location(0) normal: vec3<f32>
}
struct Primitive {
    @builtin(triangle_indices) indices: vec3<u32>
}

const TASK_MESHLET_GROUP_SIZE: u32 = 96;
const TASK_PAYLOAD_MESHLET_ARR: u32 = TASK_MESHLET_GROUP_SIZE / 4;

struct TaskPayload {
    mvp: mat4x4<f32>,
    instance: u32,
    meshletOffset: u32,
    meshletCount: u32,
    meshlets: array<u32, TASK_PAYLOAD_MESHLET_ARR>
}
var<task_payload> payload: TaskPayload;

struct TaskScratch {
    meshletCount: atomic<u32>,
    meshlets: array<u32, TASK_MESHLET_GROUP_SIZE>
}

var<workgroup> scratch: TaskScratch;

@task
@payload(payload)
@workgroup_size(32, 1, 1)
fn task(
    @builtin(local_invocation_id) localIdx: vec3<u32>,
    @builtin(global_invocation_id) globalIdx: vec3<u32>,
    @builtin(workgroup_id) workgroupIdx: vec3<u32>
) -> @builtin(mesh_task_size) vec3<u32> {
    let instance = globalIdx.y;
    let baseMeshlet = workgroupIdx.x * TASK_MESHLET_GROUP_SIZE;
    if (localIdx.x == 0) {
        atomicStore(&scratch.meshletCount, 0);
    }
    workgroupBarrier();
    let transform = instances[instance].transform;
    let totalMeshlets = arrayLength(&meshlets);
    let toProcess = min(TASK_MESHLET_GROUP_SIZE, totalMeshlets - baseMeshlet);

    for (var offset = localIdx.x; offset < toProcess; offset += 32) {
        let ml = meshlets[baseMeshlet + offset];
        let cameraForward = camera.cameraMatrix[2].xyz;
        let cameraOrigin = camera.cameraMatrix[3].xyz;
        let meshletOrigin = (transform * vec4f(ml.origin.xyz, 1.0)).xyz;
        let meshletApex = (transform * vec4f(ml.coneApex.xyz, 1.0)).xyz;
        let meshletAxis= normalize((transform * vec4f(ml.coneAxis.xyz, 0.0)).xyz);
        let meshletDir = meshletOrigin - cameraOrigin;

        let normalConeCulled = dot(normalize(meshletApex - cameraOrigin), meshletAxis) > ml.coneAxis.w;
        let frustrumCulled = dot(normalize(cameraForward), normalize(meshletDir)) < cos(70.0 * 3.14159/180.0);

        if (!frustrumCulled && !normalConeCulled) {
            let idx = atomicAdd(&scratch.meshletCount, 1);
            scratch.meshlets[idx] = offset;
        }
    }
    workgroupBarrier();


    if (localIdx.x < TASK_PAYLOAD_MESHLET_ARR) {
        let idxBase = localIdx.x * 4;
        let idxs = vec4u(scratch.meshlets[idxBase + 0], scratch.meshlets[idxBase + 1], scratch.meshlets[idxBase + 2], scratch.meshlets[idxBase + 3]);
        payload.meshlets[localIdx.x] = pack4xU8(idxs);
    }

    if (localIdx.x == 0) {
        payload.instance = instance;
        payload.meshletOffset = baseMeshlet;
        payload.meshletCount = atomicLoad(&scratch.meshletCount);
        payload.mvp = camera.projectionViewMatrix *transform;
    }

    return vec3u(payload.meshletCount, 1, 1);
}


struct MeshOutput {
    @builtin(vertices) vertices: array<Vertex, 64>,
    @builtin(primitives) primitives: array<Primitive, 64>,
    @builtin(vertex_count) vertex_count: u32,
    @builtin(primitive_count) primitive_count: u32,
}

var<workgroup> mesh_output: MeshOutput;

@mesh(mesh_output)
@payload(payload)
@workgroup_size(32, 1, 1)
fn mesh(@builtin(workgroup_id) globalIdx: vec3u, @builtin(local_invocation_id) localIdx: vec3u) {
    // fetch meshlet id from packed ids in payload
    let payloadIdx = globalIdx.x / 4;
    let payloadOffset = globalIdx.x % 4;
    let meshlet = payload.meshletOffset + unpack4xU8(payload.meshlets[payloadIdx])[payloadOffset];

    let transform = instances[payload.instance].transform;
    let mvp = payload.mvp;

    let meshletData = meshlets[meshlet];
    let v = meshletData.verticesCount;

    for (var idx = localIdx.x; idx < v; idx += 32) {
        let v = vertices[meshletData.verticesOffset + idx];
        mesh_output.vertices[idx].position = mvp * vec4f(v.position, 1.0);
        mesh_output.vertices[idx].normal = (transform * vec4f(v.normal, 0.0)).xyz;
    }

    let numTriangles = meshletData.indicesCount / 3;
    for (var tri = localIdx.x; tri < numTriangles; tri += 32) {
        let base = tri * 3;
        mesh_output.primitives[tri].indices = vec3u(
            u32(indices[meshletData.indicesOffset + base + 0]),
            u32(indices[meshletData.indicesOffset + base + 1]),
            u32(indices[meshletData.indicesOffset + base + 2])
        );
    }

    if (localIdx.x == 0) {
        mesh_output.vertex_count = meshletData.verticesCount;
        mesh_output.primitive_count = meshletData.indicesCount / 3;
    }
}

@fragment
fn fs(@location(0) normal: vec3f) -> @location(0) vec4f {
    return vec4f(abs(normalize(normal)), 1);
}
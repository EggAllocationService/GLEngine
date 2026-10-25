struct UIImmediate {
    transform: mat4x4<f32>,
    color: vec4<f32>
}

var<immediate> imm: UIImmediate;

struct VertexIn {
    @location(0) pos: vec2<f32>
}

struct VertexOut {
    @builtin(position) pos: vec4<f32>,
    @location(0) color: vec4<f32>
}

@vertex
fn vs(i: VertexIn) -> VertexOut {
    var result: VertexOut;
    result.pos = imm.transform * vec4<f32>(i.pos, 0.0, 1.0);
    result.color = imm.color;
    return result;
}

@fragment
fn fs(v: VertexOut) -> @location(0) vec4<f32> {
    return v.color;
}


struct VertexOut {
    @builtin(position) pos: vec4f,
    @location(0) uv: vec2f

}

const POSITIONS = array(
    vec4f(-1.0, -1.0, 0, 1),
    vec4f(-1.0, 1.0, 0, 1),
    vec4f(1.0, -1.0, 0, 1),
    vec4f(1.0, 1.0, 0, 1)
);

const UVs = array(
    vec2f(0, 1),
    vec2f(0, 0),
    vec2f(1, 1),
    vec2f(1, 0)
);

@vertex
fn vs(@builtin(vertex_index) idx: u32) -> VertexOut {
    var result: VertexOut;
    result.pos = POSITIONS[idx];
    result.uv = UVs[idx];
    return result;
}
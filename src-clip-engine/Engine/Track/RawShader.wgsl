@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;

@vertex
fn vs_main(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 6>(
        vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
        vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
    );
    return vec4f(pos[idx], 0.0, 1.0);
}

@fragment
fn fs_main(@builtin(position) pos: vec4f) -> @location(0) vec4f {
    let size = vec2f(textureDimensions(myTexture));
    let uv = pos.xy / size;

    let color = textureSample(myTexture, mySampler, uv);

    return vec4f(color.rgb, 1.0);
}
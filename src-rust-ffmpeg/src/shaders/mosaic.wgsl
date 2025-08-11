struct Params{
    size: f32
};

@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> params: Params;

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
    // let textureSize = vec2f(textureDimensions(myTexture));
    let textureSize = vec2f(800.0, 600.0);
    let uv = pos.xy / textureSize;
    let pixelSize = vec2(params.size, params.size);

    // 计算马赛克采样点
    let blockUV = floor(uv * textureSize / pixelSize) * pixelSize / textureSize;

    let color = textureSample(myTexture, mySampler, blockUV);
    return vec4f(color.rgb, 1.0);
}

@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> imageSize: vec2f;     // 图像尺寸

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
    let uv = pos.xy / imageSize;
    let pixelSize = vec2(8.0, 8.0);

    // 计算马赛克采样点
    let blockUV = floor(uv * imageSize / pixelSize) * pixelSize / imageSize;

    let color = textureSample(myTexture, mySampler, blockUV);
    return vec4f(color.rgb, 1.0);
}

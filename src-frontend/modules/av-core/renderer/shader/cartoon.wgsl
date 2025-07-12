@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> imageSize: vec2f;

fn quantize(color: vec3f, levels: f32) -> vec3f {
    return floor(color * levels) / levels;
}

fn sobel(uv: vec2f) -> f32 {
    let texelSize = 1.0 / imageSize;
    
    let gx = 
        -1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(-1, -1)).r +
        1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(1, -1)).r +
        -2.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(-1,  0)).r +
        2.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(1,  0)).r +
        -1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(-1,  1)).r +
        1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(1,  1)).r;

    let gy = 
        -1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(-1, -1)).r +
        -2.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(0, -1)).r +
        -1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(1, -1)).r +
        1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(-1, 1)).r +
        2.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(0, 1)).r +
        1.0 * textureSample(myTexture, mySampler, uv + texelSize * vec2f(1, 1)).r;

    return sqrt(gx * gx + gy * gy);
}

@fragment
fn fs_main(@builtin(position) pos: vec4f) -> @location(0) vec4f {
    let uv = pos.xy / imageSize;
    let color = textureSample(myTexture, mySampler, uv);
    let edge = sobel(uv);
    let toonColor = quantize(color.rgb, 4.0); // 把颜色压成4级
    let finalColor = mix(toonColor, vec3f(0.0), step(0.4, edge)); // edge > 0.3 → 画黑边
    return vec4f(finalColor, 1.0);
}

@vertex
fn vs_main(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 6>(
    vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
    vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
    );
    return vec4f(pos[idx], 0.0, 1.0);
}
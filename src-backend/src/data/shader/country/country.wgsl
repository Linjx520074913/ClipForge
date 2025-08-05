@group(0) @binding(0) var inputSampler: sampler;
@group(0) @binding(1) var inputTexture: texture_2d<f32>;

fn saturate(color: vec3<f32>, amount: f32) -> vec3<f32> {
    let gray = dot(color, vec3<f32>(0.299, 0.587, 0.114));
    return mix(vec3<f32>(gray), color, amount);
}

fn contrast(color: vec3<f32>, contrastAmount: f32) -> vec3<f32> {
    return (color - 0.5) * contrastAmount + 0.5;
}

@fragment
fn fs_main(@builtin(position) pos: vec4<f32>) -> @location(0) vec4<f32> {
    let size = vec2<f32>(textureDimensions(inputTexture));
    let uv = pos.xy / size;

    var color = textureSample(inputTexture, inputSampler, uv).rgb;

    // 调整色调，提升红绿，降低蓝色，暖色调
    color.r = min(color.r * 1.15, 1.0);
    color.g = min(color.g * 1.05, 1.0);
    color.b = color.b * 0.85;

    // 调整饱和度（0.9左右，略微降低）
    color = saturate(color, 0.9);

    // 轻微提升对比度
    color = contrast(color, 1.1);

    // 柔和光晕，基于距离中心的淡黄色叠加
    let center = vec2<f32>(0.5, 0.5);
    let dist = distance(uv, center);
    let vignette = smoothstep(0.7, 0.4, dist); // 中心区域光晕

    let warmLight = vec3<f32>(1.0, 0.9, 0.6) * vignette * 0.15;

    color += warmLight;

    return vec4<f32>(color, 1.0);
}

@vertex
fn vs_main(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 6>(
    vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
    vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
    );
    return vec4f(pos[idx], 0.0, 1.0);
}

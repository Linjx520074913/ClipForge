@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> imageSize: vec2f;

@vertex
fn vs_main(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 6>(
        vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
        vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
    );
    return vec4f(pos[idx], 0.0, 1.0);
}

fn hash(p: vec2f) -> f32 {
    // 简单伪随机函数
    let h = dot(p, vec2f(127.1, 311.7));
    return fract(sin(h) * 43758.5453123);
}

@fragment
fn fs_main(@builtin(position) pos: vec4f) -> @location(0) vec4f {
    let uv = pos.xy / imageSize;

    // 归一化坐标 -0.5 ~ 0.5
    let normUV = uv - vec2f(0.5, 0.5);

    // 基于 x 坐标计算波浪偏移
    let wave = sin(normUV.x * 30.0) * 0.02;

    // 波浪坐标变形
    let displacedUV = uv + vec2f(0.0, wave);

    // 采样变形后的纹理颜色
    let color = textureSample(myTexture, mySampler, displacedUV);

    // 给颜色叠加一个简单渐变（根据 y 坐标）
    let gradient = vec3f(uv.y, 1.0 - uv.y, 0.5);

    return vec4f(color.rgb * gradient, 1.0);
}
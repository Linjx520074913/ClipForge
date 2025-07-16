struct Params{
    strength: f32
};

@group(0) @binding(0) var mySampler: sampler;
@group(0) @binding(1) var myTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> params: Params;
/**
定义的变量都要显示用到，不然编译器会以为变量无用，就直接优化掉了，就会出现 
Number of entries (3) did not match the expected number of entries (2) for [BindGroupLayoutInternal (unlabeled)].
Expected layout: [{ binding: 0, visibility: ShaderStage::Fragment, sampler: {type: SamplerBindingType::Filtering} }, { binding: 1, visibility: ShaderStage::Fragment, texture: {sampleType: TextureSampleType::Float, viewDimension: TextureViewDimension::e2D, multisampled: 0} }]
 - While validating [BindGroupDescriptor] against [BindGroupLayout (unlabeled)]
 - While calling [Device].CreateBindGroup([BindGroupDescriptor]
 比如上面的 params，我在测试的时候，一开始只声明了这个变量，但是 shader 代码中并没有用到，就会报错
**/
fn quantize(color: vec3f, levels: f32) -> vec3f {
    return floor(color * levels) / levels;
}

fn sobel(size: f32, uv: vec2f) -> f32 {
    let texelSize = 1.0 / size;
    
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
    let size = vec2f(textureDimensions(myTexture));
    let uv = pos.xy / size;
    let color = textureSample(myTexture, mySampler, uv);
    let edge = sobel(size.x, uv);
    let toonColor = quantize(color.rgb, 4.0); // 把颜色压成4级
    let finalColor = mix(toonColor, vec3f(0.0), step(params.strength, edge)); // edge > 0.3 → 画黑边
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


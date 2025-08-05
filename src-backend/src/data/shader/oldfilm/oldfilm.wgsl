struct OldFilmUniforms{
    uSepia: f32,
    uNoise: f32,
    uNoiseSize: f32,
    uScratch: f32,
    uScratchDensity: f32,
    uScratchWidth: f32,
    uVignetting: f32,
    uVignettingAlpha: f32,
    uVignettingBlur: f32,
    time: f32
};

@group(0) @binding(0) var inputSampler: sampler;
@group(0) @binding(1) var inputTexture: texture_2d<f32>;
@group(0) @binding(2) var<uniform> params: OldFilmUniforms;

@fragment
fn fs_main(@builtin(position) pos: vec4f) -> @location(0) vec4f {
    let size = vec2f(textureDimensions(inputTexture));
    let uv = pos.xy / size;

    let uInputSize = vec4f(size.x, size.y, 1 / size.x, 1 / size.y);
    
    var color: vec4f = textureSample(inputTexture, inputSampler, uv);
    if(params.uSepia > 0.){
        color = vec4f(sepia(color.rgb), color.a);
    }

    let coord: vec2f = uv * uInputSize.xy / size;

    if(params.uVignetting > 0.){
        color *= vec4<f32>(vec3<f32>(vignette(color.rgb, coord, size)), color.a);
    }

    if(params.uScratchDensity > params.time && params.uScratch != 0.){
        color = vec4<f32>(scratch(color.rgb, coord, params.time, size), color.a);
    }

    if(params.uNoise > 0. && params.uNoiseSize > 0.){
        color += vec4<f32>(vec3<f32>(noise(uv, params.time, size)), color.a);
    }

    return color;
}

@vertex
fn vs_main(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 6>(
    vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
    vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
    );
    return vec4f(pos[idx], 0.0, 1.0);
}

const SQRT_2: f32 = 1.414213;
const SEPIA_RGB: vec3<f32> = vec3<f32>(112.0 / 255.0, 66.0 / 255.0, 20.0 / 255.0);

fn overlay(src: vec3<f32>, dst: vec3<f32>) -> vec3<f32>
{
    // if (dst <= 0.5) then: 2 * src * dst
    // if (dst > 0.5) then: 1 - 2 * (1 - dst) * (1 - src)

    return vec3<f32>(
        select((1.0 - 2.0 * (1.0 - dst.x) * (1.0 - src.x)), (2.0 * src.x * dst.x), (dst.x <= 0.5)), 
        select((1.0 - 2.0 * (1.0 - dst.y) * (1.0 - src.y)), (2.0 * src.y * dst.y), (dst.y <= 0.5)),
        select((1.0 - 2.0 * (1.0 - dst.z) * (1.0 - src.z)), (2.0 * src.z * dst.z), (dst.z <= 0.5))
    );
}

fn sepia(co: vec3<f32>) -> vec3<f32>
{
    let gray: f32 = (co.x + co.y + co.z) / 3.0;
    let grayscale: vec3<f32> = vec3<f32>(gray);
    let color = overlay(SEPIA_RGB, grayscale);
    return grayscale + params.uSepia * (color - grayscale);
}

fn vignette(co: vec3<f32>, coord: vec2<f32>, uDimensions: vec2f) -> f32
{   
    let outter: f32 = SQRT_2 - params.uVignetting * SQRT_2;
    var dir: vec2<f32> = vec2<f32>(vec2<f32>(0.5) - coord);
    dir.y *= uDimensions.y / uDimensions.x;
    let darker: f32 = clamp((outter - length(dir) * SQRT_2) / ( 0.00001 + params.uVignettingBlur * SQRT_2), 0.0, 1.0);
    return darker + (1.0 - darker) * (1.0 - params.uVignettingAlpha);
}

fn modulo(x: f32, y: f32) -> f32
{
    return x - y * floor(x/y);
}

fn scratch(co: vec3<f32>, coord: vec2<f32>, time: f32, uDimensions: vec2f) -> vec3<f32>
{
    var color = co;

    let phase: f32 = time * 256.0;
    let s: f32 = modulo(floor(phase), 2.0);
    let dist: f32 = 1.0 / params.uScratchDensity;
    let d: f32 = distance(coord, vec2<f32>(time * dist, abs(s - time * dist)));

    if (d < time * 0.6 + 0.4)
    {
        let period: f32 = params.uScratchDensity * 10.0;

        let xx: f32 = coord.x * period + phase;
        let aa: f32 = abs(modulo(xx, 0.5) * 4.0);
        let bb: f32 = modulo(floor(xx / 0.5), 2.0);
        let yy: f32 = (1.0 - bb) * aa + bb * (2.0 - aa);

        let kk: f32 = 2.0 * period;
        let dw: f32 = params.uScratchWidth / uDimensions.x * (0.75 + time);
        let dh: f32 = dw * kk;

        var tine: f32 = (yy - (2.0 - dh));

        if (tine > 0.0) {
            let _sign: f32 = sign(params.uScratch);

            tine = s * tine / period + params.uScratch + 0.1;
            tine = clamp(tine + 1.0, 0.5 + _sign * 0.5, 1.5 + _sign * 0.5);

            color *= tine;
        }
    }

    return color;
}

fn rand(p: vec2f) -> f32 {
    let dotVal = dot(p, vec2f(12.9898, 78.233));
    return fract(sin(dotVal) * 43758.5453);
}

fn noise(coord: vec2<f32>, time: f32, size: vec2f) -> f32
{
    var pixelCoord: vec2<f32> = coord * size;
    pixelCoord.x = floor(pixelCoord.x / params.uNoiseSize);
    pixelCoord.y = floor(pixelCoord.y / params.uNoiseSize);
    return (rand(pixelCoord * params.uNoiseSize * time) - 0.5) * params.uNoise;
}




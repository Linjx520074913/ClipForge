struct VHSParam{
    time: f32
};


const PI: f32 = 3.14159265;

@group(0) @binding(0) var samp: sampler;
@group(0) @binding(1) var tex: texture_2d<f32>;
@group(0) @binding(2) var<uniform> params: VHSParam;


@fragment
fn fs_main(@builtin(position) pos: vec4f) -> @location(0) vec4f {
    let size = vec2f(textureDimensions(tex));
    let uv = pos.xy / size;

    var uvn = uv;

    // tape wave
    uvn.x += (noise(vec2f(uvn.y, params.time)) - 0.5) * 0.005;
    uvn.x += (noise(vec2f(uvn.y * 100.0, params.time * 10.0)) - 0.5) * 0.01;

    // tape crease
    let tcPhase = clamp((sin(uvn.y * 8.0 - params.time * PI * 1.2) - 0.92) * noise(vec2f(params.time)), 0.0, 0.01) * 10.0;
    let tcNoise = max(noise(vec2f(uvn.y * 100.0, params.time * 10.0)) - 0.5, 0.0);
    uvn.x = uvn.x - tcNoise * tcPhase;

    // switching noise
    let snPhase = smoothstep(0.03, 0.0, uvn.y);
    uvn.y += snPhase * 0.3;
    uvn.x += snPhase * ((noise(vec2f(uv.y * 100.0, params.time * 10.0)) - 0.5) * 0.2);

    var col = tex2D(uvn);
    col = col * (1.0 - tcPhase);
    col = mix(col, col.yzx, snPhase);

    // bloom
    for (var x = -4.0; x < 2.5; x = x + 1.0) {
        col = col + vec3(
        tex2D(uvn + vec2f(x - 0.0, 0.0) * 0.007).x,
        tex2D(uvn + vec2f(x - 2.0, 0.0) * 0.007).y,
        tex2D(uvn + vec2f(x - 4.0, 0.0) * 0.007).z
        ) * 0.1;
    }
    col = col * 0.6;

    // ac beat
    col = col * (1.0 + clamp(noise(vec2f(0.0, uv.y + params.time * 0.2)) * 0.6 - 0.25, 0.0, 0.1));

    return vec4(col, 1.0);
}

@vertex
fn vs_main(@builtin(vertex_index) idx: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 6>(
    vec2f(-1.0, -1.0), vec2f(1.0, -1.0), vec2f(-1.0, 1.0),
    vec2f(-1.0, 1.0), vec2f(1.0, -1.0), vec2f(1.0, 1.0)
    );
    return vec4f(pos[idx], 0.0, 1.0);
}

fn hash(v: vec2f) -> f32 {
    return fract(sin(dot(v, vec2f(89.44, 19.36))) * 22189.22);
}

fn iHash(v: vec2f, r: vec2f) -> f32 {
    let p = floor(v * r);
    let h00 = hash(p / r);
    let h10 = hash((p + vec2f(1.0, 0.0)) / r);
    let h01 = hash((p + vec2f(0.0, 1.0)) / r);
    let h11 = hash((p + vec2f(1.0, 1.0)) / r);
    let ip = smoothstep(vec2f(0.0), vec2f(1.0), fract(v * r));
    return mix(mix(h00, h10, ip.x), mix(h01, h11, ip.x), ip.y);
}

fn noise(v: vec2f) -> f32 {
  var sum = 0.0;
  for (var i = 1; i < 9; i = i + 1) {
    let f = pow(2.0, f32(i));
    sum = sum + iHash(v + vec2f(f32(i)), vec2f(2.0 * f)) / f;
  }
  return sum;
}

fn tex2D(p: vec2f) -> vec3f {
  var col = textureSample(tex, samp, p).xyz;
  if (abs(p.x - 0.5) > 0.5) {
    col = vec3f(0.1);
  }
  return col;
}

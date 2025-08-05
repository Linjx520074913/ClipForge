@group(0) @binding(0)
var input_texture: texture_2d<f32>;

@group(0) @binding(1)
var output_buffer: storage_buffer<array<f32>>;

@compute @workgroup_size(8, 8)
fn main(@builtin(global_invocation_id) GlobalInvocationID: vec3<u32>) {
    let dims = textureDimensions(input_texture);
    if (GlobalInvocationID.x >= dims.x || GlobalInvocationID.y >= dims.y) {
        return;
    }

    let pixel: vec4<f32> = textureLoad(input_texture, vec2<i32>(GlobalInvocationID.xy), 0);
    let r = pixel.r;

    let index = GlobalInvocationID.y * dims.x + GlobalInvocationID.x;
    output_buffer[index] = r;
}

// Compiled with column-major matrices enabled in the compilation API
//
// ------------------------- vertex shader -------------------------

struct MeshVertex {
    float3 position;
    float  uv_x;
    float3 normal;
    float  uv_y;
    float4 color;
};

//struct VSPushConstants {
//    float4x4 world_matrix;
//    MeshVertex* vertex_address;
//};

struct GlobalBuffer {
    float4x4 projection;
    float4x4 view;
    float4x4 model;
};

[[vk::binding(0,0)]] ConstantBuffer<GlobalBuffer> global_buffer;

//struct VSInput {
//    uint vertex_index : SV_VertexID;
//};
struct VSInput {
    MeshVertex vertex;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color;
    float2 uv;
};

//[[vk::push_constant]] VSPushConstants vertex_push_constants;

[shader("vertex")]
VSOutput vertex_main(VSInput input) {
    VSOutput output;

    // Get the vertex data from the device address
    // MeshVertex vertex = vertex_push_constants.vertex_address[input.vertex_index];

    output.position = mul(global_buffer.projection, mul(global_buffer.view, mul(global_buffer.model, float4(input.vertex.position, 1.0f))));
    output.color = input.vertex.color;;
    output.uv.x = input.vertex.uv_x;
    output.uv.y = input.vertex.uv_y;

    return output;
}

// ------------------------- pixel shader -------------------------

struct PSInput {
    float4 color;
    float2 uv;
};

struct PSOutput {
    float4 color;
};

[[vk::binding(0,1)]] Sampler2D texture;

[shader("pixel")]
PSOutput pixel_main(PSInput input) {
    PSOutput output;
    output.color = texture.Sample(input.uv);

    return output;
}


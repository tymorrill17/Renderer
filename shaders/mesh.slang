// ------------------------- vertex shader -------------------------

struct MeshVertex {
    float3 position;
    float  uv_x;
    float3 normal;
    float  uv_y;
    float4 color;
};

struct VSPushConstants {
    float4x4 world_matrix;
    MeshVertex* vertex_address;
};

struct GlobalBuffer {
    float4x4 projection;
    float4x4 view;
    float4x4 model;
};

[[vk::binding(0,0)]] ConstantBuffer<GlobalBuffer> global_buffer;

struct VSInput {
    uint vertex_index : SV_VertexID;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color;

    float4 debug_model_pos;
    float4 debug_view_model_pos;
    float4 debug_proj_view_model_pos;
};

[[vk::push_constant]] VSPushConstants vertex_push_constants;

[shader("vertex")]
VSOutput vertex_main(VSInput input) {
    VSOutput output;

    // Get the vertex data from the device address
    MeshVertex vertex = vertex_push_constants.vertex_address[input.vertex_index];

    output.position = mul(vertex_push_constants.world_matrix, float4(vertex.position, 1.0f));
    //output.position = float4(vertex.position, 1.0f);
    output.color = vertex.color;
    return output;
}

struct VSInput2 {
    MeshVertex vertex;
};

[shader("vertex")]
VSOutput vertex_main2(VSInput2 input) {
    VSOutput output;

    output.position = mul(global_buffer.projection, mul(global_buffer.view, mul(global_buffer.model, float4(input.vertex.position, 1.0f))));
    output.color = input.vertex.color;;

    output.debug_model_pos = mul(global_buffer.model, float4(input.vertex.position, 1.0f));
    output.debug_view_model_pos = mul(global_buffer.view, output.debug_model_pos);
    output.debug_proj_view_model_pos = mul(global_buffer.projection, output.debug_view_model_pos);

    return output;
}

// ------------------------- pixel shader -------------------------

struct PSInput {
    float4 color;
};

struct PSOutput {
    float4 color;
};

[shader("pixel")]
PSOutput pixel_main(PSInput input) {
    PSOutput output;
    output.color = input.color;

    return output;
}


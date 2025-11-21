// ------------------------- vertex shader -------------------------

struct MeshVertex {
	float3 position;
	float uv_x;
	float3 normal;
	float uv_y;
	float4 color;
};

struct VSPushConstants {
    float4x4 world_matrix;
    MeshVertex* vertex_address;
};

struct VSInput {
    uint vertex_index : SV_VertexID;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color;
    float2 uv;
};

[[vk::push_constant]] VSPushConstants vertex_push_constants;

[shader("vertex")]
VSOutput vertex_main(VSInput input) {
    VSOutput output;

    // Get the vertex data from the device address
	MeshVertex vertex = vertex_push_constants.vertex_address[input.vertex_index];

	output.position = mul(vertex_push_constants.world_matrix, float4(vertex.position, 1.0f));
	output.color = vertex.color;
	output.uv.x = vertex.uv_x;
	output.uv.y = vertex.uv_y;

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


// 最大処理するライト数
#define MAX_LIGHTS 64

struct PointLight
{
    float3 color;
    float padding;
    float2 pos;
    float intensity;
    float radius;
};

// ライト情報の定数バッファ
cbuffer PointLightBuffer : register(b0)
{
    PointLight Lights[MAX_LIGHTS];
    uint LightCount;
    float3 padding;
};

cbuffer CameraConstBuffer : register(b1)
{
    float2 CameraPos;
    float2 CameraZoom;
}

// 出力するライティングマップ (UAV)
RWTexture2D<float4> LightMap : register(u0);


// スレッドグループのサイズ（8x8 ピクセル単位で並列処理）
[numthreads(8,8, 1)]
void cs_main(uint3 id : SV_DispatchThreadID)
{
    int2 pixelCoord = id.xy;
    
    //float2 pixelPos = floor(float2(pixelCoord) / 8) * 8;
    float2 pixelPos = float2(pixelCoord);
            
    float3 totalLight = float3(0.25, 0.25, 0.25);

    // 各ピクセルごとに全ライトの影響を計算
    for (uint i = 0; i < LightCount; ++i)
    { 
        float dist = length((pixelPos - Lights[i].pos) / CameraZoom);
        float brightness = Lights[i].intensity / (1.0 + dist * dist / Lights[i].radius);
        totalLight += Lights[i].color * brightness;
    }

    // ライトマップに出力
    //LightMap[pixelCoord] = float4(pixelPos.x / 320, pixelPos.y / 180, 0.0, 1.0);
    LightMap[pixelCoord] = float4(totalLight, 1.0);
}
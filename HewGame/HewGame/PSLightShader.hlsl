//--------------------------------------------------------------------------------------
// ピクセルシェーダー
//--------------------------------------------------------------------------------------

cbuffer SystenBuffer : register(b0)
{
    int2 resolution;
    int2 screen;
};

cbuffer WndBuffer : register(b1)
{
    int2 rect;
    float2 pos;
};

// ピクセルの情報の構造体（受け取り用）
struct PS_IN
{
    // float4型　→　float型が４つの構造体
    float4 pos : SV_POSITION; // ピクセルの画面上の座標
    float4 col : COLOR0;
    float2 tex : TEXCOORD;
};

//グローバル変数の宣言
Texture2D myTexture : register(t0); //テクスチャ―

//ライティングマップ
Texture2D LightMap : register(t1);

//※C言語側からデータを渡されたときにセットされる変数
SamplerState mySampler : register(s0); //サンプラー

// ピクセルシェーダーのエントリポイント
float4 ps_main(PS_IN input) : SV_Target
{
    //float2 uv = floor(input.tex * 256) / 256;
    
    float4 color = myTexture.Sample(mySampler,input.tex);
    
    //決定した色をreturn する
    float4 baseColor = color * input.col;
    
    //ライティングの影響を取得する
    float2 uv = input.pos.xy / screen;
    float2 pixelPos = uv * rect + pos; 
    float3 lighting = LightMap.Sample(mySampler, pixelPos / resolution).rgb;
   
    return float4(baseColor.rgb * lighting, baseColor.a);
}
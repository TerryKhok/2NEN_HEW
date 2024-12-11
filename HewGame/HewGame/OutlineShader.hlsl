//--------------------------------------------------------------------------------------
// ピクセルシェーダー
//--------------------------------------------------------------------------------------

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

//※C言語側からデータを渡されたときにセットされる変数
SamplerState mySampler : register(s0); //サンプラー

// ピクセルシェーダーのエントリポイント
float4 ps_main(PS_IN input) : SV_Target
{
    //float4 color = myTexture.Sample(mySampler, input.tex);
    ////決定した色をreturn する
    //return color * input.col;
    
    float2 texSize = float2(1.0 / 1024, 1.0 / 768); // Replace with texture dimensions
    float4 center = myTexture.Sample(mySampler, input.tex).rgb;

    float4 edges = abs(center -
        myTexture.Sample(mySampler, input.tex + float2(texSize.x, 0))) +
        abs(center - myTexture.Sample(mySampler, input.tex + float2(-texSize.x, 0))) +
        abs(center - myTexture.Sample(mySampler, input.tex + float2(0, texSize.y))) +
        abs(center - myTexture.Sample(mySampler, input.tex + float2(0, -texSize.y)));

    float intensity = dot(edges, float4(0.333, 0.333, 0.333, 1.0f));
    return float4(intensity, intensity, intensity, 1.0); // Highlight in grayscale
}
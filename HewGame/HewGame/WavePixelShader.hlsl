//グローバル変数の宣言
Texture2D myTexture : register(t0); //テクスチャ―

//ライティングマップ
Texture2D NoiseMap : register(t1);

//※C言語側からデータを渡されたときにセットされる変数
SamplerState mySampler : register(s0); //サンプラー

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
    float2 tex : TEXCOORD0;
};


float4 ps_main(PS_IN input) : SV_TARGET
{  
    //float2 pixelPos = (input.pos.xy / float2(960, 720)) * rect + pos;
    //float4 wave = NoiseMap.Sample(mySampler, pixelPos / float2(1920, 1080));
    
    float4 wave = NoiseMap.Sample(mySampler, input.pos.xy / screen);
        
    // テクスチャの色を取得
    float4 color = myTexture.Sample(mySampler, input.tex);
    
    float4 finalColor = color * input.col;
    finalColor.rgb *= wave.a;
      
    return finalColor;
}

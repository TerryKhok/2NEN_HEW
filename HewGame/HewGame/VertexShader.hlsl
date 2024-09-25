//--------------------------------------------------------------------------------------
// 頂点シェーダー
//--------------------------------------------------------------------------------------

cbuffer ConstBuffer : register(b0)
{
    //ワールド変換行列
    matrix world;
    //プロジェクション変換行列
    matrix projection;
    //UV座標移動行列
    matrix tex;
    //色情報
    float4 color;
}

// 頂点のデータを表す構造体（受け取り用）
struct VS_IN
{
    float4 pos : POSITION;
    float4 col : COLOR0;
    float2 tex : TEX;
};

// 頂点のデータを表す構造体（送信用） 
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 tex : TEXCOORD;
};

 
// 頂点シェーダーのエントリポイント 
VS_OUT vs_main(VS_IN input)
{
    VS_OUT output;
 
    output.pos = mul(input.pos, world);
    output.pos = mul(output.pos, projection);
    output.col = input.col * color;
    
    //UV座標を移動する
    float4 uv;
    //行列掛け算のためfloat4型に移す
    uv.xy = input.tex;
    uv.z = 0.0f;
    uv.w = 1.0f;
    uv = mul(uv, tex);
    output.tex = uv.xy;
    
    return output;
}
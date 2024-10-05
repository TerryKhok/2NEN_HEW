//--------------------------------------------------------------------------------------
// 頂点シェーダー
//--------------------------------------------------------------------------------------

cbuffer ConstBuffer : register(b0)
{
    //ワールド変換行列
    matrix world;
    //カメラ変換
    matrix view;
    //プロジェクション変換行列
    matrix projection;
    // UV scaling to divide the texture
    float2 uvScale;
    // UV offset to animate texture
    float2 uvOffset;
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
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);

    output.col = input.col * color;
    
    output.tex = input.tex * uvScale + uvOffset;
      
    return output;
}
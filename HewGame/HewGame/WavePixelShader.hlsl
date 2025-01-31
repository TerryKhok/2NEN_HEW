Texture2D tex : register(t0);
SamplerState samp : register(s0);

cbuffer TimeBuffer : register(b0)
{
    float time;
    float strength;
    float noiseScale;
    float persistence;
};

// スムーズな補間関数
float smoothFade(float t)
{
    return t * t * (3.0 - 2.0 * t);
}

// シームレスな乱数関数
float random(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

// スムーズなPerlinノイズ
float smoothNoise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    
    float2 u = smoothFade(f);

    float n00 = random(i);
    float n01 = random(i + float2(0, 1));
    float n10 = random(i + float2(1, 0));
    float n11 = random(i + float2(1, 1));

    return lerp(lerp(n00, n10, u.x), lerp(n01, n11, u.x), u.y);
}

// フラクタルノイズ (オクターブごとにスケールを変えて加算)
float fractalNoise(float2 uv)
{
    float total = 0.0;
    float frequency = noiseScale;
    float amplitude = 1.0;
    float maxValue = 0.0;

    for (int i = 0; i < 4; i++)
    {
        total += smoothNoise(uv * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence; // 振幅を減衰
        frequency *= 2.0; // 周波数を倍に
    }

    return total / maxValue; // 正規化
}

// ピクセルの情報の構造体（受け取り用）
struct PS_IN
{
    // float4型　→　float型が４つの構造体
    float4 pos : SV_POSITION; // ピクセルの画面上の座標
    float4 col : COLOR0;
    float2 tex : TEXCOORD;
};

float4 ps_main(PS_IN input) : SV_TARGET
{
    // 時間経過で変化するノイズ
    float2 noiseUV = input.tex * noiseScale + float2(time * 0.1, time * 0.1);
    float noise = fractalNoise(noiseUV);

    // ノイズをUVに適用して揺れを加える
    float2 distortedUV = input.tex + (noise - 0.5) * strength;

    // テクスチャの色を取得
    float4 color = tex.Sample(samp, distortedUV);
    return color * input.col;
}

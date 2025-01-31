Texture2D tex : register(t0);
SamplerState samp : register(s0);

cbuffer TimeBuffer : register(b0)
{
    float time;
    float strength;
    float noiseScale;
    float persistence;
};

// �X���[�Y�ȕ�Ԋ֐�
float smoothFade(float t)
{
    return t * t * (3.0 - 2.0 * t);
}

// �V�[�����X�ȗ����֐�
float random(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

// �X���[�Y��Perlin�m�C�Y
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

// �t���N�^���m�C�Y (�I�N�^�[�u���ƂɃX�P�[����ς��ĉ��Z)
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
        amplitude *= persistence; // �U��������
        frequency *= 2.0; // ���g����{��
    }

    return total / maxValue; // ���K��
}

// �s�N�Z���̏��̍\���́i�󂯎��p�j
struct PS_IN
{
    // float4�^�@���@float�^���S�̍\����
    float4 pos : SV_POSITION; // �s�N�Z���̉�ʏ�̍��W
    float4 col : COLOR0;
    float2 tex : TEXCOORD;
};

float4 ps_main(PS_IN input) : SV_TARGET
{
    // ���Ԍo�߂ŕω�����m�C�Y
    float2 noiseUV = input.tex * noiseScale + float2(time * 0.1, time * 0.1);
    float noise = fractalNoise(noiseUV);

    // �m�C�Y��UV�ɓK�p���ėh���������
    float2 distortedUV = input.tex + (noise - 0.5) * strength;

    // �e�N�X�`���̐F���擾
    float4 color = tex.Sample(samp, distortedUV);
    return color * input.col;
}

//--------------------------------------------------------------------------------------
// ���_�V�F�[�_�[
//--------------------------------------------------------------------------------------

cbuffer ConstBuffer : register(b0)
{
    //���[���h�ϊ��s��
    matrix world;
    //�v���W�F�N�V�����ϊ��s��
    matrix projection;
    //UV���W�ړ��s��
    matrix tex;
    //�F���
    float4 color;
}

// ���_�̃f�[�^��\���\���́i�󂯎��p�j
struct VS_IN
{
    float4 pos : POSITION;
    float4 col : COLOR0;
    float2 tex : TEX;
};

// ���_�̃f�[�^��\���\���́i���M�p�j 
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 tex : TEXCOORD;
};

 
// ���_�V�F�[�_�[�̃G���g���|�C���g 
VS_OUT vs_main(VS_IN input)
{
    VS_OUT output;
 
    output.pos = mul(input.pos, world);
    output.pos = mul(output.pos, projection);
    output.col = input.col * color;
    
    //UV���W���ړ�����
    float4 uv;
    //�s��|���Z�̂���float4�^�Ɉڂ�
    uv.xy = input.tex;
    uv.z = 0.0f;
    uv.w = 1.0f;
    uv = mul(uv, tex);
    output.tex = uv.xy;
    
    return output;
}
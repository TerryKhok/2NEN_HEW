//--------------------------------------------------------------------------------------
// ���_�V�F�[�_�[
//--------------------------------------------------------------------------------------

cbuffer ConstBuffer : register(b0)
{
    //���[���h�ϊ��s��
    matrix world;
    //�J�����ϊ�
    matrix view;
    //�v���W�F�N�V�����ϊ��s��
    matrix projection;
    // UV scaling to divide the texture
    float2 uvScale;
    // UV offset to animate texture
    float2 uvOffset;
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
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);

    output.col = input.col * color;
    
    output.tex = input.tex * uvScale + uvOffset;
      
    return output;
}
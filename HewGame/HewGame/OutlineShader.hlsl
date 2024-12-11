//--------------------------------------------------------------------------------------
// �s�N�Z���V�F�[�_�[
//--------------------------------------------------------------------------------------

// �s�N�Z���̏��̍\���́i�󂯎��p�j
struct PS_IN
{
    // float4�^�@���@float�^���S�̍\����
    float4 pos : SV_POSITION; // �s�N�Z���̉�ʏ�̍��W
    float4 col : COLOR0;
    float2 tex : TEXCOORD;
};

//�O���[�o���ϐ��̐錾
Texture2D myTexture : register(t0); //�e�N�X�`���\

//��C���ꑤ����f�[�^��n���ꂽ�Ƃ��ɃZ�b�g�����ϐ�
SamplerState mySampler : register(s0); //�T���v���[

// �s�N�Z���V�F�[�_�[�̃G���g���|�C���g
float4 ps_main(PS_IN input) : SV_Target
{
    //float4 color = myTexture.Sample(mySampler, input.tex);
    ////���肵���F��return ����
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
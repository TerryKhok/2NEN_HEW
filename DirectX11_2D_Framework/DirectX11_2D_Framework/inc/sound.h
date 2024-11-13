#pragma once



struct WaveData
{
    WAVEFORMATEX wfex;
    BYTE* data;
    DWORD dataSize;
};

class Sound
{
private:
    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasterVoice;
    IXAudio2SourceVoice* pSourceVoice;
    WaveData waveData;
    bool isPlaying = false;                          // �Đ����t���O

    bool Init();    // XAudio2�̏�����
    void Cleanup(); // ���\�[�X�̉��

public:
    Sound();    // �R���X�g���N�^
    ~Sound();   // �f�X�g���N�^

    bool LoadWavFile(const char* filename);     // WAV�t�@�C����ǂݍ���
    bool IsPlaying();                           // �Đ������ǂ������m�F����
    void SoundPlay(float volume, bool loop);    // ���ʂ��w�肵�čĐ�
    void SoundStop();                           // �Đ���~
};
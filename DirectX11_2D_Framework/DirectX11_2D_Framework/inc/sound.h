#pragma once

#pragma comment ( lib, "winmm.lib" )
#include<mmsystem.h>

struct WaveData
{
    WAVEFORMATEX m_wavFormat;
    char* m_soundBuffer;
    DWORD m_size;

    ~WaveData() { free(m_soundBuffer); }
};

class Sound
{
private:
    IXAudio2* pXAudio2;
    IXAudio2MasteringVoice* pMasteringVoice;
    IXAudio2SourceVoice* pSourceVoice;
    //WaveData waveData;
    bool isPlaying = false;                          // �Đ����t���O

    bool Init();    // XAudio2�̏�����
    void Cleanup(); // ���\�[�X�̉��

public:
    Sound();    // �R���X�g���N�^
    ~Sound();   // �f�X�g���N�^

    bool LoadWaveFile(const std::wstring& wFilePath, WaveData* outData);     // WAV�t�@�C����ǂݍ���
    bool IsPlaying();                           // �Đ������ǂ������m�F����
    bool PlayWaveSound(const std::wstring& wFileName, WaveData* outData, bool loop);    // ���ʂ��w�肵�čĐ�
    void SoundStop();                           // �Đ���~
};
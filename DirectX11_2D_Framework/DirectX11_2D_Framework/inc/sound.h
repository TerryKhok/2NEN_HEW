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
    bool isPlaying = false;                          // 再生中フラグ

    bool Init();    // XAudio2の初期化
    void Cleanup(); // リソースの解放

public:
    Sound();    // コンストラクタ
    ~Sound();   // デストラクタ

    bool LoadWaveFile(const std::wstring& wFilePath, WaveData* outData);     // WAVファイルを読み込む
    bool IsPlaying();                           // 再生中かどうかを確認する
    bool PlayWaveSound(const std::wstring& wFileName, WaveData* outData, bool loop);    // 音量を指定して再生
    void SoundStop();                           // 再生停止
};
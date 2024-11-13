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
    bool isPlaying = false;                          // 再生中フラグ

    bool Init();    // XAudio2の初期化
    void Cleanup(); // リソースの解放

public:
    Sound();    // コンストラクタ
    ~Sound();   // デストラクタ

    bool LoadWavFile(const char* filename);     // WAVファイルを読み込む
    bool IsPlaying();                           // 再生中かどうかを確認する
    void SoundPlay(float volume, bool loop);    // 音量を指定して再生
    void SoundStop();                           // 再生停止
};
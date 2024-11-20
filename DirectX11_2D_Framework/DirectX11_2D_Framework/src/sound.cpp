

Sound::Sound() : pXAudio2(nullptr), pMasterVoice(nullptr), pSourceVoice(nullptr), waveData{}
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        LOG_ERROR("CoInitEx error");
        return;
    }
    Init();
}


Sound::~Sound()
{
    Cleanup();
    //CoUninitialize();
}


bool Sound::Init()
{
    if (FAILED(XAudio2Create(&pXAudio2, 0))) 
    {
        std::cerr << "XAudio2の初期化に失敗しました。" << std::endl;
        return false;
    }
    if (FAILED(pXAudio2->CreateMasteringVoice(&pMasterVoice)))
    {
        std::cerr << "マスターボイスの作成に失敗しました。" << std::endl;
        return false;
    }
    return true;
}


bool Sound::LoadWavFile(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "ファイルを開けませんでした: " << filename << std::endl;
        return false;
    }

    // RIFFチャンクやWAVEヘッダーを読み込み
    char chunkId[4];
    file.read(chunkId, 4);
    if (std::memcmp(chunkId, "RIFF", 4) != 0) 
    {
        std::cerr << "無効なファイルフォーマットです" << std::endl;
        return false;
    }

    file.seekg(20, std::ios::beg);
    file.read(reinterpret_cast<char*>(&waveData.wfex), sizeof(WAVEFORMATEX));

    // データチャンクを探し、データを取得
    while (true) 
    {
        file.read(chunkId, 4);
        if (std::memcmp(chunkId, "data", 4) == 0) 
        {
            file.read(reinterpret_cast<char*>(&waveData.dataSize), sizeof(DWORD));
            waveData.data = new BYTE[waveData.dataSize];
            file.read(reinterpret_cast<char*>(waveData.data), waveData.dataSize);
            break;
        }
        else 
        {
            file.seekg(sizeof(DWORD), std::ios::cur);
        }
    }

    file.close();

    // ソースボイスの作成
    if (FAILED(pXAudio2->CreateSourceVoice(&pSourceVoice, &waveData.wfex))) 
    {
        std::cerr << "ソースボイスの作成に失敗しました。" << std::endl;
        return false;
    }

    return true;
}


bool Sound::IsPlaying()
{
    if (pSourceVoice == nullptr) return false;

    XAUDIO2_VOICE_STATE state;
    pSourceVoice->GetState(&state);
    return isPlaying && (state.BuffersQueued > 0);
}


void Sound::SoundPlay(float volume, bool loop)
{
    if (pSourceVoice == nullptr) 
    {
        std::cerr << "音声データが読み込まれていません。" << std::endl;
        return;
    }

    // 音量の設定（0.0f = 無音、1.0f = 最大音量）
    volume = (std::max)(0.0f, (std::min)(volume, 1.0f));
    pSourceVoice->SetVolume(volume);

    // バッファの設定
    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = waveData.dataSize;
    buffer.pAudioData = waveData.data;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    // ループの設定
    if (loop) 
    {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;  // 無限ループ
    }

    // バッファをセットし再生開始
    pSourceVoice->SubmitSourceBuffer(&buffer);
    pSourceVoice->Start(0);

    // 再生中フラグを立てる
    isPlaying = true;
}



void Sound::SoundStop()
{
    if (pSourceVoice) 
    {
        pSourceVoice->Stop(0);               // 再生停止
        pSourceVoice->FlushSourceBuffers();  // バッファをクリア
        isPlaying = false;                   // 再生中フラグをリセット
    }
}



void Sound::Cleanup()
{
    if (pSourceVoice) 
    {
        pSourceVoice->DestroyVoice();
        pSourceVoice = nullptr;
    }

    if (pMasterVoice) 
    {
        pMasterVoice->DestroyVoice();
        pMasterVoice = nullptr;
    }

    if (pXAudio2) 
    {
        pXAudio2->Release();
        pXAudio2 = nullptr;
    }

    if (waveData.data) 
    {
        delete[] waveData.data;
        waveData.data = nullptr;
    }
}


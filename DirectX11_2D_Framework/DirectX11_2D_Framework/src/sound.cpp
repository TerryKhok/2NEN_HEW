

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
        std::cerr << "XAudio2�̏������Ɏ��s���܂����B" << std::endl;
        return false;
    }
    if (FAILED(pXAudio2->CreateMasteringVoice(&pMasterVoice)))
    {
        std::cerr << "�}�X�^�[�{�C�X�̍쐬�Ɏ��s���܂����B" << std::endl;
        return false;
    }
    return true;
}


bool Sound::LoadWavFile(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "�t�@�C�����J���܂���ł���: " << filename << std::endl;
        return false;
    }

    // RIFF�`�����N��WAVE�w�b�_�[��ǂݍ���
    char chunkId[4];
    file.read(chunkId, 4);
    if (std::memcmp(chunkId, "RIFF", 4) != 0) 
    {
        std::cerr << "�����ȃt�@�C���t�H�[�}�b�g�ł�" << std::endl;
        return false;
    }

    file.seekg(20, std::ios::beg);
    file.read(reinterpret_cast<char*>(&waveData.wfex), sizeof(WAVEFORMATEX));

    // �f�[�^�`�����N��T���A�f�[�^���擾
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

    // �\�[�X�{�C�X�̍쐬
    if (FAILED(pXAudio2->CreateSourceVoice(&pSourceVoice, &waveData.wfex))) 
    {
        std::cerr << "�\�[�X�{�C�X�̍쐬�Ɏ��s���܂����B" << std::endl;
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
        std::cerr << "�����f�[�^���ǂݍ��܂�Ă��܂���B" << std::endl;
        return;
    }

    // ���ʂ̐ݒ�i0.0f = �����A1.0f = �ő剹�ʁj
    volume = (std::max)(0.0f, (std::min)(volume, 1.0f));
    pSourceVoice->SetVolume(volume);

    // �o�b�t�@�̐ݒ�
    XAUDIO2_BUFFER buffer = { 0 };
    buffer.AudioBytes = waveData.dataSize;
    buffer.pAudioData = waveData.data;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    // ���[�v�̐ݒ�
    if (loop) 
    {
        buffer.LoopCount = XAUDIO2_LOOP_INFINITE;  // �������[�v
    }

    // �o�b�t�@���Z�b�g���Đ��J�n
    pSourceVoice->SubmitSourceBuffer(&buffer);
    pSourceVoice->Start(0);

    // �Đ����t���O�𗧂Ă�
    isPlaying = true;
}



void Sound::SoundStop()
{
    if (pSourceVoice) 
    {
        pSourceVoice->Stop(0);               // �Đ���~
        pSourceVoice->FlushSourceBuffers();  // �o�b�t�@���N���A
        isPlaying = false;                   // �Đ����t���O�����Z�b�g
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


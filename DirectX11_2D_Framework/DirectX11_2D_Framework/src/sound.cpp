#include "../inc/sound.h"

Sound::Sound()
{
    /*HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        LOG_ERROR("CoInitEx error");
        return;
    }*/
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
    if (FAILED(pXAudio2->CreateMasteringVoice(&pMasteringVoice)))
    {
        std::cerr << "�}�X�^�[�{�C�X�̍쐬�Ɏ��s���܂����B" << std::endl;
        return false;
    }
    return true;
}


bool Sound::LoadWaveFile(const std::wstring& wFilePath, WaveData* outData)
{

	// ���g�����Ă���̗�����A��U������Ƃ�
	// (����Ȃ��ƁA���Ƃ̒��g�̃T�E���h�o�b�t�@�[������ꍇ�A���������[�N����)
	if (outData)
	{
		free(outData->m_soundBuffer);
	}
	// nullptr�������烊�^�[������
	else
	{
		std::cerr << "nullptr�������烊�^�[������B" << std::endl;
		return false;
	}


	HMMIO mmioHandle = nullptr;

	// �`�����N���
	MMCKINFO chunkInfo{};

	// RIFF�`�����N�p
	MMCKINFO riffChunkInfo{};


	// WAV�t�@�C�����J��
	mmioHandle = mmioOpenW(
		(LPWSTR)wFilePath.data(),
		NULL,
		MMIO_READ
	);

	if (!mmioHandle)
	{
		// Wav�t�@�C�����J���܂���ł���
		std::cerr << "Wav�t�@�C�����J���܂���ł����B" << std::endl;
		return false;
	}

	// RIFF�`�����N�ɐN�����邽�߂�fccType��WAVE��ݒ������
	riffChunkInfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	// RIFF�`�����N�ɐN������
	if (mmioDescend(
		mmioHandle,		//MMIO�n���h��
		&riffChunkInfo,	//�擾�����`�����N�̏��
		nullptr,		//�e�`�����N
		MMIO_FINDRIFF	//�擾���̎��
	) != MMSYSERR_NOERROR)
	{
		// ���s
		// Riff�`�����N�ɐN�����s���܂���
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "Riff�`�����N�ɐN�����s���܂����B" << std::endl;
		return false;
	}

	// �N����̃`�����N��"fmt "�Ƃ��Đݒ肷��
	chunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(
		mmioHandle,
		&chunkInfo,
		&riffChunkInfo,
		MMIO_FINDCHUNK
	) != MMSYSERR_NOERROR)
	{
		// fmt�`�����N���Ȃ��ł�
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "fmt�`�����N���Ȃ��ł��B" << std::endl;
		return false;
	}

	// fmt�f�[�^�̓ǂݍ���
	DWORD readSize = mmioRead(
		mmioHandle,						//�n���h��
		(HPSTR)&outData->m_wavFormat,	// �ǂݍ��ݗp�o�b�t�@
		chunkInfo.cksize				//�o�b�t�@�T�C�Y
	);

	if (readSize != chunkInfo.cksize)
	{
		// �ǂݍ��݃T�C�Y����v���Ă��܂���
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "�ǂݍ��݃T�C�Y����v���Ă��܂���B" << std::endl;
		return false;
	}

	// �t�H�[�}�b�g�`�F�b�N
	if (outData->m_wavFormat.wFormatTag != WAVE_FORMAT_PCM)
	{
		// Wave�t�H�[�}�b�g�G���[�ł�
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "Wave�t�H�[�}�b�g�G���[�ł��B" << std::endl;
		return false;
	}

	// fmt�`�����N��ޏo����
	if (mmioAscend(mmioHandle, &chunkInfo, 0) != MMSYSERR_NOERROR)
	{
		// fmt�`�����N�ޏo���s
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "fmt�`�����N�ޏo���s�B" << std::endl;
		return false;
	}

	// data�`�����N�ɐN��
	chunkInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(mmioHandle, &chunkInfo, &riffChunkInfo, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		// data�`�����N�N�����s
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "data�`�����N�N�����s�B" << std::endl;
		return false;
	}
	// �T�C�Y�ۑ�
	outData->m_size = chunkInfo.cksize;

	// data�`�����N�ǂݍ���
	outData->m_soundBuffer = new char[chunkInfo.cksize];
	readSize = mmioRead(mmioHandle, (HPSTR)outData->m_soundBuffer, chunkInfo.cksize);
	if (readSize != chunkInfo.cksize)
	{
		// data�`�����N�ǂݍ��ݎ��s
		mmioClose(mmioHandle, MMIO_FHOPEN);
		delete[] outData->m_soundBuffer;
		std::cerr << "data�`�����N�ǂݍ��ݎ��s�B" << std::endl;
		return false;
	}

	// �t�@�C�������
	mmioClose(mmioHandle, MMIO_FHOPEN);

	return true;
}



bool Sound::IsPlaying()
{
    if (pSourceVoice == nullptr) return false;

    XAUDIO2_VOICE_STATE state;
    pSourceVoice->GetState(&state);
    return isPlaying && (state.BuffersQueued > 0);
}

bool Sound::PlayWaveSound(const std::wstring& wFileName, WaveData* outData, bool loop)
{
	if (!LoadWaveFile(wFileName, outData))
	{
		//Wave�t�@�C���ǂݍ��ݎ��s
		return false;
	}

	//=======================
	// SourceVoice�̍쐬
	//=======================
	WAVEFORMATEX waveFormat{};

	// �g�`�t�H�[�}�b�g�̐ݒ�
	memcpy(&waveFormat, &outData->m_wavFormat, sizeof(outData->m_wavFormat));

	// 1�T���v��������̃o�b�t�@�T�C�Y���Z�o
	waveFormat.wBitsPerSample = outData->m_wavFormat.nBlockAlign * 8 / outData->m_wavFormat.nChannels;

	// �\�[�X�{�C�X�̍쐬 �����ł̓t�H�[�}�b�g�̂ݓn���Ă���
	HRESULT result = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&waveFormat);
	if (FAILED(result))
	{
		// SourceVoice�쐬���s
		return false;
	}

	//================================
	// �g�`�f�[�^(���f�[�^�{��)���\�[�X�{�C�X�ɓn��
	//================================
	XAUDIO2_BUFFER xAudio2Buffer{};
	xAudio2Buffer.pAudioData = (BYTE*)outData->m_soundBuffer;
	xAudio2Buffer.Flags = XAUDIO2_END_OF_STREAM;
	xAudio2Buffer.AudioBytes = outData->m_size;

	// �O�����Z�q��p���āA���[�v���邩�ۂ��̐ݒ������
	xAudio2Buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

	pSourceVoice->SubmitSourceBuffer(&xAudio2Buffer);

	// ���ۂɉ���炷
	pSourceVoice->Start();

	return true;
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

    if (pMasteringVoice) 
    {
        pMasteringVoice->DestroyVoice();
        pMasteringVoice = nullptr;
    }

    if (pXAudio2) 
    {
        pXAudio2->Release();
        pXAudio2 = nullptr;
    }

    //CoUninitialize();
}


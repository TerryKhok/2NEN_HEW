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
		std::cerr << "XAudio2の初期化に失敗しました。" << std::endl;
		return false;
	}
	if (FAILED(pXAudio2->CreateMasteringVoice(&pMasteringVoice)))
	{
		std::cerr << "マスターボイスの作成に失敗しました。" << std::endl;
		return false;
	}
	for (int i = 0; i < SOUND_LABEL_MAX; i++)
	{
		LoadWaveFile(soundFile[i].filename, &waveData[i], m_pSourceVoice[i],m_buffer[i], soundFile[i].bLoop);
	}

	return true;
}

bool Sound::LoadWaveFile(const std::wstring& wFilePath, WaveData* outData, IXAudio2SourceVoice* pSourceVoice, XAUDIO2_BUFFER& audioBuffer,bool loop)
{

	// 中身入ってるもの来たら、一旦解放しとく
	// (じゃないと、もとの中身のサウンドバッファーがある場合、メモリリークする)
	if (outData)
	{
		free(outData->m_soundBuffer);
	}
	// nullptrが来たらリターンする
	else
	{
		std::cerr << "nullptrが来たらリターンする。" << std::endl;
		return false;
	}


	HMMIO mmioHandle = nullptr;

	// チャンク情報
	MMCKINFO chunkInfo{};

	// RIFFチャンク用
	MMCKINFO riffChunkInfo{};


	// WAVファイルを開く
	mmioHandle = mmioOpenW(
		(LPWSTR)wFilePath.data(),
		NULL,
		MMIO_READ
	);

	if (!mmioHandle)
	{
		// Wavファイルを開けませんでした
		std::cerr << "Wavファイルを開けませんでした。" << std::endl;
		return false;
	}

	// RIFFチャンクに侵入するためにfccTypeにWAVEを設定をする
	riffChunkInfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	// RIFFチャンクに侵入する
	if (mmioDescend(
		mmioHandle,		//MMIOハンドル
		&riffChunkInfo,	//取得したチャンクの情報
		nullptr,		//親チャンク
		MMIO_FINDRIFF	//取得情報の種類
	) != MMSYSERR_NOERROR)
	{
		// 失敗
		// Riffチャンクに侵入失敗しました
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "Riffチャンクに侵入失敗しました。" << std::endl;
		return false;
	}

	// 侵入先のチャンクを"fmt "として設定する
	chunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(
		mmioHandle,
		&chunkInfo,
		&riffChunkInfo,
		MMIO_FINDCHUNK
	) != MMSYSERR_NOERROR)
	{
		// fmtチャンクがないです
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "fmtチャンクがないです。" << std::endl;
		return false;
	}

	// fmtデータの読み込み
	DWORD readSize = mmioRead(
		mmioHandle,						//ハンドル
		(HPSTR)&outData->m_wavFormat,	// 読み込み用バッファ
		chunkInfo.cksize				//バッファサイズ
	);

	if (readSize != chunkInfo.cksize)
	{
		// 読み込みサイズが一致していません
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "読み込みサイズが一致していません。" << std::endl;
		return false;
	}

	// フォーマットチェック
	if (outData->m_wavFormat.wFormatTag != WAVE_FORMAT_PCM)
	{
		// Waveフォーマットエラーです
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "Waveフォーマットエラーです。" << std::endl;
		return false;
	}

	// fmtチャンクを退出する
	if (mmioAscend(mmioHandle, &chunkInfo, 0) != MMSYSERR_NOERROR)
	{
		// fmtチャンク退出失敗
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "fmtチャンク退出失敗。" << std::endl;
		return false;
	}

	// dataチャンクに侵入
	chunkInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(mmioHandle, &chunkInfo, &riffChunkInfo, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		// dataチャンク侵入失敗
		mmioClose(mmioHandle, MMIO_FHOPEN);
		std::cerr << "dataチャンク侵入失敗。" << std::endl;
		return false;
	}
	// サイズ保存
	outData->m_size = chunkInfo.cksize;

	// dataチャンク読み込み
	outData->m_soundBuffer = new char[chunkInfo.cksize];
	readSize = mmioRead(mmioHandle, (HPSTR)outData->m_soundBuffer, chunkInfo.cksize);
	if (readSize != chunkInfo.cksize)
	{
		// dataチャンク読み込み失敗
		mmioClose(mmioHandle, MMIO_FHOPEN);
		delete[] outData->m_soundBuffer;
		std::cerr << "dataチャンク読み込み失敗。" << std::endl;
		return false;
	}

	// ファイルを閉じる
	mmioClose(mmioHandle, MMIO_FHOPEN);

	//if (!LoadWaveFile(wFileName, outData))
	//{
	//	//Waveファイル読み込み失敗
	//	return false;
	//}

	//=======================
	// SourceVoiceの作成
	//=======================
	WAVEFORMATEX waveFormat{};
	if (pSourceVoice) {
		pSourceVoice->DestroyVoice();
		pSourceVoice = nullptr;
	}

	// 波形フォーマットの設定
	memcpy(&waveFormat, &outData->m_wavFormat, sizeof(outData->m_wavFormat));

	// 1サンプル当たりのバッファサイズを算出
	waveFormat.wBitsPerSample = outData->m_wavFormat.nBlockAlign * 8 / outData->m_wavFormat.nChannels;

	// ソースボイスの作成 ここではフォーマットのみ渡っている
	HRESULT result = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&waveFormat);
	if (FAILED(result))
	{
		// SourceVoice作成失敗
		return false;
	}

	//================================
	// 波形データ(音データ本体)をソースボイスに渡す
	//================================
	audioBuffer.pAudioData = (BYTE*)outData->m_soundBuffer;
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
	audioBuffer.AudioBytes = outData->m_size;

	// 三項演算子を用いて、ループするか否かの設定をする
	audioBuffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

	return true;
}


bool Sound::PlayWaveSound(SOUND_LABEL label, float volume)
{
	IXAudio2SourceVoice*& pSV = m_pSourceVoice[(int)label];

	WAVEFORMATEX waveFormat{};
	if (pSV != nullptr)
	{
		pSV->DestroyVoice();
		pSV = nullptr;
	}

	// 波形フォーマットの設定
	memcpy(&waveFormat, &waveData[(int)label].m_wavFormat, sizeof(waveData[(int)label].m_wavFormat));

	// 1サンプル当たりのバッファサイズを算出
	waveFormat.wBitsPerSample = waveData[(int)label].m_wavFormat.nBlockAlign * 8 / waveData[(int)label].m_wavFormat.nChannels;

	pXAudio2->CreateSourceVoice(&pSV, (WAVEFORMATEX*)&waveFormat);
	pSV->SubmitSourceBuffer(&(m_buffer[(int)label]));

	// 音量の設定（0.0f = 無音、1.0f = 最大音量）
	volume = (std::max)(0.0f, (std::min)(volume, 1.0f));
	pSV->SetVolume(volume);

	// 実際に音を鳴らす
	pSV->Start();

	return true;
}

bool Sound::IsPlaying(SOUND_LABEL label)
{
	if (m_pSourceVoice[(int)label] == nullptr) return false;

	XAUDIO2_VOICE_STATE state;
	m_pSourceVoice[(int)label]->GetState(&state);
	return isPlaying && (state.BuffersQueued > 0);
}

void Sound::SoundStop(SOUND_LABEL label)
{
	if (m_pSourceVoice[(int)label] == NULL) return;

	XAUDIO2_VOICE_STATE xa2state;
	m_pSourceVoice[(int)label]->GetState(&xa2state);
	if (xa2state.BuffersQueued)
	{
		m_pSourceVoice[(int)label]->Stop(0); // 再生停止
		m_pSourceVoice[(int)label]->FlushSourceBuffers();  // バッファをクリア
		isPlaying = false;                   // 再生中フラグをリセット
	}
}



void Sound::Cleanup()
{
	for (int i = 0; i < SOUND_LABEL_MAX; i++)
	{
		if (m_pSourceVoice[i])
		{
			m_pSourceVoice[i]->Stop(0);
			m_pSourceVoice[i]->FlushSourceBuffers();
			m_pSourceVoice[i]->DestroyVoice();			// オーディオグラフからソースボイスを削除
		}
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

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

typedef enum
{
	BGM_Game01 = 0,
	BGM_Game02,
	BGM_MainMenu,
	SFX_Jump,
	SFX_Land,
	SFX_Walk01,

	SOUND_LABEL_MAX,
} SOUND_LABEL;


class Sound
{
private:
	typedef struct
	{
		const std::wstring filename;	// �����t�@�C���܂ł̃p�X��ݒ�
		bool bLoop;			// true�Ń��[�v�B�ʏ�BGM��ture�ASE��false�B
	} PARAM;

	PARAM soundFile[SOUND_LABEL_MAX] =
	{
		{L"../../HewGame/HewGame/asset/sound/bgm/BGM_Game01.wav",true},
		{L"../../HewGame/HewGame/asset/sound/bgm/BGM_Game02.wav",true},
		{L"../../HewGame/HewGame/asset/sound/bgm/BGM_MainMenu.wav",true},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Jump.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Land.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Walk01.wav",false},
	};
	IXAudio2* pXAudio2 = NULL;
	IXAudio2MasteringVoice* pMasteringVoice = NULL;
	WaveData waveData[SOUND_LABEL_MAX];
	IXAudio2SourceVoice* m_pSourceVoice[SOUND_LABEL_MAX];
	XAUDIO2_BUFFER m_buffer[SOUND_LABEL_MAX];
	
	//WAVEFORMATEXTENSIBLE m_wfx[SOUND_LABEL_MAX]; // WAV�t�H�[�}�b�g
	//BYTE* m_DataBuffer[SOUND_LABEL_MAX];

	bool isPlaying = false;                          // �Đ����t���O

	bool Init();    // XAudio2�̏�����
	void Cleanup(); // ���\�[�X�̉��

public:
	Sound();    // �R���X�g���N�^
	~Sound();   // �f�X�g���N�^

	bool LoadWaveFile(const std::wstring& wFilePath, WaveData* outData, IXAudio2SourceVoice* m_pSourceVoice, XAUDIO2_BUFFER& audioBuffer,bool loop);     // WAV�t�@�C����ǂݍ���
	bool PlayWaveSound(SOUND_LABEL label, float volume);    // ���ʂ��w�肵�čĐ�
	bool IsPlaying(SOUND_LABEL label);                           // �Đ������ǂ������m�F����
	void SoundStop(SOUND_LABEL label);                           // �Đ���~
};

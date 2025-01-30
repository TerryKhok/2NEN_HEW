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
	SFX_Walk02,
	SFX_Walk03,
	SFX_Walk04,
	SFX_Cancel,
	SFX_Clear,
	SFX_Confirm,
	SFX_Death,
	SFX_Hit,
	SFX_Open,
	SFX_Select,
	SFX_Exit,

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


#if defined(_DEBUG)
	PARAM soundFile[SOUND_LABEL_MAX] =
	{
		{L"../../HewGame/HewGame/asset/sound/bgm/BGM_Game01.wav",true},
		{L"../../HewGame/HewGame/asset/sound/bgm/BGM_Game02.wav",true},
		{L"../../HewGame/HewGame/asset/sound/bgm/BGM_MainMenu.wav",true},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Jump.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Land.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Walk01.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Walk02.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Walk03.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Walk04.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Cancel.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Clear.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Confirm.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Death.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Hit.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Open.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Select.wav",false},
		{L"../../HewGame/HewGame/asset/sound/se/SFX_Exit.wav",false},
	};
#else
	PARAM soundFile[SOUND_LABEL_MAX] =
	{
		{L"asset/sound/bgm/BGM_Game01.wav",true},
		{L"asset/sound/bgm/BGM_Game02.wav",true},
		{L"asset/sound/bgm/BGM_MainMenu.wav",true},
		{L"asset/sound/se/SFX_Jump.wav",false},
		{L"asset/sound/se/SFX_Land.wav",false},
		{L"asset/sound/se/SFX_Walk01.wav",false},
		{L"asset/sound/se/SFX_Walk02.wav",false},
		{L"asset/sound/se/SFX_Walk03.wav",false},
		{L"asset/sound/se/SFX_Walk04.wav",false},
		{L"asset/sound/se/SFX_Cancel.wav",false},
		{L"asset/sound/se/SFX_Clear.wav",false},
		{L"asset/sound/se/SFX_Confirm.wav",false},
		{L"asset/sound/se/SFX_Death.wav",false},
		{L"asset/sound/se/SFX_Hit.wav",false},
		{L"asset/sound/se/SFX_Open.wav",false},
		{L"asset/sound/se/SFX_Select.wav",false},
		{L"asset/sound/se/SFX_Exit.wav",false},
	};
#endif
	

	IXAudio2* pXAudio2 = NULL;
	IXAudio2MasteringVoice* pMasteringVoice = NULL;
	WaveData waveData[SOUND_LABEL_MAX];
	IXAudio2SourceVoice* m_pSourceVoice[SOUND_LABEL_MAX];
	XAUDIO2_BUFFER m_buffer[SOUND_LABEL_MAX];
	
	//WAVEFORMATEXTENSIBLE m_wfx[SOUND_LABEL_MAX]; // WAV�t�H�[�}�b�g
	//BYTE* m_DataBuffer[SOUND_LABEL_MAX];

	bool isPlaying = false;                          // �Đ����t���O

	Sound();    // �R���X�g���N�^
	~Sound();   // �f�X�g���N�^

	bool Init();    // XAudio2�̏�����
	void Cleanup(); // ���\�[�X�̉��
public:
	//�C���X�^���X�擾
	static Sound& Get()
	{
		static Sound sound;
		return sound;
	}

	bool LoadWaveFile(const std::wstring& wFilePath, WaveData* outData, IXAudio2SourceVoice* m_pSourceVoice, XAUDIO2_BUFFER& audioBuffer,bool loop);     // WAV�t�@�C����ǂݍ���
	bool PlayWaveSound(SOUND_LABEL label, float volume);    // ���ʂ��w�肵�čĐ�
	bool IsPlaying(SOUND_LABEL label);                           // �Đ������ǂ������m�F����
	void SoundStop(SOUND_LABEL label);                           // �Đ���~
};

#include "Sample_Animation.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
//  �V�[���̑J�ڃT���v��
// 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//����ȃN���X�����邱�Ƃ�m�点�邽�߂ɐ�ɒ�`����
class SampleScene_Loading;
class SampleScene02;

//=============================================================================
//���V�[���̓o�^��main.cpp�łƂ肠��������Ă܂�
//=============================================================================


class SampleScene_Title : public Scene
{
	//���[�h���ɂ��邱�Ƃ�����(�I�[�o���C�h)
	void Load() override
	{
		//�I�u�W�F�N�g����(���O�A�e�N�X�`���w��)
		Instantiate("HartR", L"asset/pic/hartR.png");
	}

	//�X�V����(�I�[�o���C�h)
	void Update() override
	{
		//�X�y�[�X�L�[���������Ƃ�
		if (Input::Get().KeyTrigger(VK_SPACE))
 		{
			//���[�h���̃V�[���ɑJ��(����) ���Ȃ��Ă�����
			SceneManager::LoadScene<SampleScene_Loading>();

			//�V�[���̃��[�f�B���O(�񓯊�)
			SceneManager::LoadingScene<SampleScene_Box2D>();
			//���[�h�����V�[���ɑJ��
			SceneManager::ChangeScene();
		}
	}
};

class SampleScene_Loading : public Scene
{
	//�I�u�W�F�N�g�|�C���^�[
	GameObject* arrow = nullptr;

	void Load()
	{
		arrow = Instantiate("Arrow", L"asset/pic/rollArrow.png");
	}

	void Update()
	{
		//��]�����Ă邾��
		static float angle = 0.0f;
		angle -= 0.1f;
		if (abs(angle) > 360.0f)
			angle = 0.0f;
		
		arrow->transform.angle.z = angle;
	}
};
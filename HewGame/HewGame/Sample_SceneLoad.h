
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
//  �V�[���̑J�ڃT���v��
// 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//����ȃN���X�����邱�Ƃ�m�点�邽�߂ɐ�ɒ�`����
class LoadScene;
class SampleScene02;

//=============================================================================
//���V�[���̓o�^��main.cpp�łƂ肠��������Ă܂�
//=============================================================================


class SampleScene01 : public Scene
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
			SceneManager::LoadScene<LoadScene>();

			//�V�[���̃��[�f�B���O(�񓯊�)
			SceneManager::LoadingScene<Box2D_SampleScene>();
			//���[�h�����V�[���ɑJ��
			SceneManager::ChangeScene();
		}
	}
};

class LoadScene : public Scene
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

class SampleScene02 : public Scene
{
	void Load()
	{
		//�I�u�W�F�N�g����
		auto object = Instantiate("HartR", L"asset/pic/hartR.png");

		//���O�̕ύX
		object->SetName("HartG");

		//���ʂɏ������d������
		ObjectManager::Find("HartG")->GetComponent<Transform>()->gameobject->GetComponent<Renderer>()->SetTexture(L"asset/pic/hartG.png");

		//�^���I�ɏd��������\��(�����̃X���[�v)
		std::this_thread::sleep_for(std::chrono::seconds(4));
	}

	void Update()
	{
		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			//�V�[���̐؂�ւ�(����)
			SceneManager::LoadScene<SampleScene01>();

			return; //�����[�h��̏������΂������ꍇ��return���g��

			//�����s����Ȃ�
			LOG("after LoadScene!!!");
		}
	}
};

//================================================================
//�V�[���̑J�ڃT���v��
//================================================================


class LoadScene;
class SampleScene02;


class SampleScene01 : public Scene
{
	//���[�h���ɂ��邱�Ƃ�����(�I�[�o���C�h)
	void Load() override
	{
		//�I�u�W�F�N�g����
		auto object = Instantiate("HartR", L"asset/pic/hartR.png");
	}

	//�X�V����(�I�[�o���C�h)
	void Update() override
	{
		if (Input::Get().KeyTrigger(VK_SPACE))
 		{
			//���[�h���̃V�[���ɑJ��(����)
			SceneManager::LoadScene<LoadScene>();
			//�V�[���̃��[�f�B���O(�񓯊�)
			SceneManager::LoadingScene<SampleScene02>();
			//���[�h�����V�[���̐؂�ւ�
			SceneManager::ChangeScene();
		}
	}
};

class LoadScene : public Scene
{
	GameObject* object = nullptr;

	void Load()
	{
		object = Instantiate("HartR", L"asset/pic/rollArrow.png");
	}

	void Update()
	{
		//��]�����Ă邾��
		static float angle = 0.0f;
		angle += 0.1f;
		if (angle > 360.0f)
			angle = 0.0f;
		
		object->transform.angle.z -= 0.1f;
	}
};

class SampleScene02 : public Scene
{
	void Load()
	{
		auto object = Instantiate("HartR", L"asset/pic/hartG.png");
		//�^���I�ɏd��������\��(�����̃X���[�v)
		std::this_thread::sleep_for(std::chrono::seconds(4));
	}

	void Update()
	{
		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			//�V�[���̐؂�ւ�(����)
			SceneManager::LoadScene<SampleScene01>();
		}
	}
};
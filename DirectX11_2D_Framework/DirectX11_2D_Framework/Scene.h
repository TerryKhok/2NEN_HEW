#pragma once

class Scene
{
	friend class Window;
	friend class SceneManager;

protected:
	//�p���ȊO�����֎~
	Scene() {}

	//�I�u�W�F�N�g����
	GameObject* Instantiate();
	//�I�u�W�F�N�g����(���O�w��)
	GameObject* Instantiate(std::string _name);
	//�I�u�W�F�N�g����(���O,�e�N�X�`���w��)
	GameObject* Instantiate(std::string _name, const wchar_t* _texPath);
private:
	//�V�[���̃��[�h�����i�I�u�W�F�N�g�̐����j
	virtual void Load() {}
	//�����������i�V�[���J�ڌ�ɌĂяo���j
	virtual void Init() {}
	//���t���[���Ăяo��
	virtual void Update() {}
	//������������
	virtual void Uninit() {}
};


class SceneManager final
{
	friend class Window;

private:
	//�����֎~
	SceneManager();

	//�ŏ��̃V�[���̓ǂݍ���
	static void Init();
	//���ւ̃V�[���̐؂�ւ�
	static void NextScene();
public:
	//�V�[���̓ǂݍ��݁E�؂�ւ�(����)
	template<typename T>
	static void LoadScene()
	{
		std::string sceneName = typeid(T).name();

		//�񓯊��ɃV�[�������[�h���Ă���ꍇ
		if (async)
		{
			LOG("no loding %s,other scene loding now", sceneName.substr(6).c_str());
		}

		//�V�[�����o�^�ς݂��ǂ���
		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end()) {

			//�V�������X�g�ɕς���
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			//�Ή������V�[���̃��[�h����
			it->second();
			//�V�[���؂�ւ�
			NextScene();
			
			LOG_NL;
			LOG("Now Switching to %s",sceneName.substr(6).c_str());
			return;
		}

		//�V�[����������Ȃ������ꍇ
		LOG_WARNING("couldn't find the scene, so registered it.");
		RegisterScene<T>();
		LoadScene<T>();
	}

	//�V�[���̓ǂݍ���(�񓯊�)
	template<typename T>
	static void LoadingScene()
	{
		//���[�h��
		if (async) return;

		std::string sceneName = typeid(T).name();

		//�V�[�����o�^����Ă��邩
		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end()) {
			async = true;
			loading = true;

			// Start scene loading asynchronously
			LOG_NL;
			LOG("Starting scene loading...%s", sceneName.substr(6).c_str());

			//�X���b�h�𗧂Ă�
			std::future<void> sceneFuture = std::async(std::launch::async, [&]()
				{
					Box2D::WorldManager::ChengeNextWorld();
					RenderManager::ChangeNextRenderList();
					ObjectManager::ChangeNextObjectList();
					it->second();
				});
			//�X���b�h���I���܂Ń��[�v������
			Window::WindowUpdate(sceneFuture, loading);

			return;
		}

		//�V�[�����o�^����Ă��Ȃ������ꍇ
		LOG_WARNING("couldn't find the scene, so registered it.");
		RegisterScene<T>();
		LoadingScene<T>();
	}

	//�V�[���̐؂�ւ�(���[�h���I����Ă��炶��Ȃ��Ɣ��f����Ȃ�)
	static void ChangeScene()
	{
		//�񓯊��Ń��[�h���I����Ă���ꍇ
		if (async && !loading)
		{
			//�V�[���̐؂�ւ�
			NextScene();

			//���[�h���Ă��������X�g�ɐ؂�ւ���
			RenderManager::LinkNextRenderList();
			ObjectManager::LinkNextObjectList();
			Box2D::WorldManager::LinkNextWorld();

			async = false;

			LOG("Now Switching to the New Scene...");
		}
	}

	//�V�[���N���X�����X�g�ɓo�^����
	template<typename T>
	static void RegisterScene() {
		std::string sceneName = typeid(T).name();

		//�V�[�������ɂ���ꍇ
		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end())
		{
			LOG("%s is already registered.", sceneName.substr(6).c_str());
			return;
		}
			
		//���[�h�֐����쐬
		std::function<void(void)> createFn = [&]()
			{
				m_nextScene.reset(new T());
				m_nextScene->Load();
			};

		//�o�^
		m_sceneList[sceneName] = createFn;
	}

private:
	//�V�[�����X�g
	static std::unordered_map<std::string, std::function<void()>> m_sceneList;
	//���̃V�[��
	static std::unique_ptr<Scene> m_currentScene;
	//���̃V�[��
	static std::unique_ptr<Scene> m_nextScene;
	//�񓯊��p�ϐ�
	static bool async;
	static bool loading;
};

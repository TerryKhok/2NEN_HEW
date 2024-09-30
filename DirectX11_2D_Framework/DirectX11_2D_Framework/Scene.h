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


class SceneManager
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

		if (async)
		{
			std::string warning = "no loding" + sceneName + ",other scene loding now";
			LOG(warning.c_str());
		}

		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end()) {
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			it->second();
			NextScene();
			LOG("scene loaded!");
			return;
		}

		LOG_WARNING("no matching scene is found");
	}

	//�V�[���̓ǂݍ���(�񓯊�)
	template<typename T>
	static void LoadingScene()
	{
		if (async) return;

		std::string sceneName = typeid(T).name();

		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end()) {
			async = true;
			loading = true;

			std::future<void> sceneFuture = std::async(std::launch::async, [&]()
				{
					RenderManager::ChangeNextRenderList();
					ObjectManager::ChangeNextObjectList();
					it->second();
				});
			Window::WindowUpdate(sceneFuture, loading);

			return;
		}

		LOG_WARNING("no matching scene is found");
	}

	//�V�[���̐؂�ւ�(���[�h���I����Ă��炶��Ȃ��Ɣ��f����Ȃ�)
	static void ChangeScene()
	{
		if (async && !loading)
		{
			NextScene();

			RenderManager::LinkNextRenderList();
			ObjectManager::LinkNextObjectList();

			async = false;

			LOG("Now Switching to the New Scene...");
		}
	}

	//�V�[���N���X�����X�g�ɓo�^����
	template<typename T>
	static void RegisterScene() {
		std::string sceneName = typeid(T).name();

		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end())
		{
			std::string error = sceneName + " scene is exist";
			assert(false && error.c_str());
		}
			
		std::function<void(void)> createFn = [&]()
			{
				m_nextScene.reset(new T());
				m_nextScene->Load();
			};

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

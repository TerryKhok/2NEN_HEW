#pragma once


class Scene
{
	friend class Window;
	friend class SceneManager;

protected:
	//�p���ȊO�����֎~
	Scene() {}
	//�p���ȊO�폜�֎~
	virtual ~Scene() = default;

	//�I�u�W�F�N�g����
	GameObject* Instantiate();
	//�I�u�W�F�N�g����(���O�w��)
	GameObject* Instantiate(std::string _name);
	//�I�u�W�F�N�g����(���O,�e�N�X�`���w��)
	GameObject* Instantiate(std::string _name, const wchar_t* _texPath);
	//�I�u�W�F�N�g�̍폜(�|�C���^�w��)
	void DeleteObject(GameObject* _object);
	//�I�u�W�F�N�g�̍폜(���O�w��)
	inline void DeleteObject(std::string _name);
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
	SceneManager() = delete;

	//�ŏ��̃V�[���̓ǂݍ���
	static void Init();
	//�V�[���̔j��
	static void Uninit();
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

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			//�V�������X�g�ɕς���
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			//�Ή������V�[���̃��[�h����
			it->second();
			//�V�[���؂�ւ�
			NextScene();
			//�V�[��������
			TRY_CATCH_LOG(m_currentScene->Init());

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::EnableWorldUpdate();
			Box2D::WorldManager::ResumeWorldUpdate();
#endif	
			LOG_NL;
			LOG("Now Switching to %s",sceneName.substr(6).c_str());

			throw "";
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
					//�ǉ����V�����ύX����
					Box2D::WorldManager::ChengeNextWorld();
					RenderManager::ChangeNextRenderList();
					ObjectManager::ChangeNextObjectList();
					//��������E�B���h�E��\�����Ȃ�
					Window::WindowSubLoadingBegin();
					//�V�[���̃��[�h����
					it->second();
					//�Â����[���h���폜����
					Box2D::WorldManager::DeleteOldWorld();
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

			//���݂���T�u�E�B���h�E�I�u�W�F�N�g���B��
			Window::WindowSubHide();

#ifdef BOX2D_UPDATE_MULTITHREAD
			//���[���h�̍X�V���ꎞ��~
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			ObjectManager::LinkNextObjectList();
			Box2D::WorldManager::LinkNextWorld();

#ifdef BOX2D_UPDATE_MULTITHREAD
			//���[���h�̍X�V���ĊJ
			Box2D::WorldManager::ResumeWorldUpdate();
#endif
			//���ׂẴE�B���h�E��\��
			Window::WindowSubLoadingEnd();

			async = false;

			//�V�[��������
			TRY_CATCH_LOG(m_currentScene->Init());

			LOG("Now Switching to the New Scene...");

			throw "";
		}
	}

	//�V�[���N���X�����X�g�ɓo�^����
	template<typename T>
	static void RegisterScene() {
		std::string sceneName = typeid(T).name();

		if (firstScene.empty())
		{
			firstScene = sceneName;
		}

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
				//�f�X�g���N�^�o�^
				std::unique_ptr<Scene, void(*)(Scene*)> scene(new T, [](Scene* p) {delete p; });
				m_nextScene = std::move(scene);
#ifdef DEBUG_TRUE
				try{
					m_nextScene->Load();
				}
				//��O�L���b�`(nullptr�Q�ƂƂ�)
				catch (const std::exception& e) {
					LOG_ERROR(e.what());
				}
#else
				try{
					m_nextScene->Load();
				}
				catch(...){}
#endif
			};

		//�o�^
		m_sceneList[sceneName] =createFn;
	}

	//���݂̃V�[���������[�h(�񓯊�)
	//��������߂��Ȃ��̂ŌĂяo���Ă����̌�̏����͌p������
	static void ReloadCurrentScene()
	{
		//�V�[�����o�^�ς݂��ǂ���
		auto it = m_sceneList.find(typeid(*m_currentScene.get()).name());
		if (it != m_sceneList.end()) {

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			//�V�������X�g�ɕς���
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			//�Ή������V�[���̃��[�h����
			it->second();
			//�V�[���؂�ւ�
			NextScene();
			//�V�[��������
			TRY_CATCH_LOG(m_currentScene->Init());

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::EnableWorldUpdate();
			Box2D::WorldManager::ResumeWorldUpdate();
#endif	
		}
	}

private:
	//���߂̃V�[��
	static std::string firstScene;
	//�V�[�����X�g
	static std::unordered_map<std::string,std::function<void()>> m_sceneList;
	//���̃V�[��
	static std::unique_ptr<Scene, void(*)(Scene*)> m_currentScene;
	//���̃V�[��
	static std::unique_ptr<Scene, void(*)(Scene*)> m_nextScene;
	//�񓯊��p�ϐ�
	static bool async;
	static bool loading;
};

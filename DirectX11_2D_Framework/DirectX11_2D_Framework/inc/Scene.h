#pragma once

#define REGISTER_SCENE_LIST_PATH "sceneref.scrf"

static constexpr int REGISTER_SCENE_TYPE_HEADER = 'h';
static constexpr int REGISTER_SCENE_TYPE_FILE	= 'f';

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
	virtual void UnInit() {}

	//�I�v�V�����F �f�}���O�����O�֐��ɂ��A�R���p�C���ɂ���Ă͂��������o�͂�������B
	static std::string demangle(const char* name) {
#ifdef __GNUG__
		int status = -1;
		std::unique_ptr<char, void(*)(void*)> res{
			abi::__cxa_demangle(name, nullptr, nullptr, &status),
			std::free
		};
		return (status == 0) ? res.get() : name;
#else
		return name;  // Fallback if demangling is not available
#endif
	}
public:
	// Get the type name of the derived class
	virtual std::string getType() const {
		return demangle(typeid(*this).name());
	}
};

class SampleScene : public Scene
{
	void Load() override
	{
		auto object = Instantiate("object");
	}
};

class FileScene : public Scene
{
	friend class SceneManager;

	FileScene(std::filesystem::path _path) :path(_path) {}

	void Load() override
	{
		std::ifstream ifs(path);
		{
			SERIALIZE_INPUT archive(ifs);
	
			int index = 0;
			archive(CEREAL_NVP(index));
			if (index != SceneFileIndex)
			{
				LOG_ERROR("this is not scene file");
				return;
			}

			size_t size;
			archive(size);
			for (size_t i = 0; i < size; ++i)
			{
				GameObject* object = new GameObject;
				archive(*object);
				ObjectManager::AddObject(object);
			}
		}
	}

	std::string getType() const override{
		return path.stem().string();
	}

private:
	std::filesystem::path path;
};

class SceneManager final
{
	friend class Window;
	friend class ImGuiApp;

private:
	//�����֎~
	SceneManager() = delete;

	//�ŏ��̃V�[���̓ǂݍ���
	static void Init();
	//�V�[���̔j��
	static void UnInit();
	//���ւ̃V�[���̐؂�ւ�
	static void NextScene();
	//�����V�[���؂�ւ�
	static void LoadScene(std::string _sceneName)
	{
		//�񓯊��ɃV�[�������[�h���Ă���ꍇ
		if (async)
		{
			LOG("no loading %s,other scene loading now", _sceneName.c_str());
		}

		//�V�[�����o�^�ς݂��ǂ���
		auto it = m_sceneList.find(_sceneName);
		if (it != m_sceneList.end()) {

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif

#ifdef DEBUG_TRUE
			ImGuiApp::InvalidSelectedObject();
#endif
			TextureAssets::ChangeNextTextureLib();

			//�V�������X�g�ɕς���
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			SFTextManager::GenerateList();
			//�Ή������V�[���̃��[�h����
			it->second();
			//�V�[���؂�ւ�
			NextScene();
			//�V�[��������
			TRY_CATCH_LOG(m_currentScene->Init());

			TextureAssets::LinkNextTextureLib();

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::EnableWorldUpdate();
			Box2D::WorldManager::ResumeWorldUpdate();
#endif	
			LOG("Now Switching to %s", _sceneName.c_str());
		}
	}
public:
	//�V�[���̓ǂݍ��݁E�؂�ւ�(����)
	template<typename T>
	static void LoadScene()
	{
		std::string sceneName = typeid(T).name();

		//�񓯊��ɃV�[�������[�h���Ă���ꍇ
		if (async)
		{
			LOG("no loading %s,other scene loading now", sceneName.substr(6).c_str());
		}

		//�V�[�����o�^�ς݂��ǂ���
		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end()) {

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif

#ifdef DEBUG_TRUE
			ImGuiApp::InvalidSelectedObject();
#endif
			TextureAssets::ChangeNextTextureLib();

			//�V�������X�g�ɕς���
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			SFTextManager::GenerateList();
			//�Ή������V�[���̃��[�h����
			it->second();
			//�V�[���؂�ւ�
			NextScene();
			//�V�[��������
			TRY_CATCH_LOG(m_currentScene->Init());

			TextureAssets::LinkNextTextureLib();

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::EnableWorldUpdate();
			Box2D::WorldManager::ResumeWorldUpdate();
#endif	
			LOG("Now Switching to %s", sceneName.substr(6).c_str());

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
			LOG("Starting scene loading...%s", sceneName.c_str());

			//�X���b�h�𗧂Ă�
			std::future<void> sceneFuture = std::async(std::launch::async, [&]()
				{
					TextureAssets::ChangeNextTextureLib();

					//�ǉ����V�����ύX����
					Box2D::WorldManager::ChangeNextWorld();
					RenderManager::ChangeNextRenderList();
					ObjectManager::ChangeNextObjectList();
					SFTextManager::ChangeNextTextList();
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
#ifdef DEBUG_TRUE
			ImGuiApp::InvalidSelectedObject();
#endif
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

			SFTextManager::LinkNextTextList();

			TextureAssets::LinkNextTextureLib();

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

		RegisterSceneNode registerSceneNode(typeid(T).name(), typeid(T).name(), REGISTER_SCENE_TYPE_HEADER);
		m_registerScenePath.push_back(std::move(registerSceneNode));
			
		//���[�h�֐����쐬
		std::function<void(void)> createFn = [&]()
			{
				currentScenePath = "null";

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
		m_sceneList[sceneName] = std::move(createFn);
	}

	//���݂̃V�[���������[�h(�񓯊�)
	//��������߂��Ȃ��̂ŌĂяo���Ă����̌�̏����͌p������
	static void ReloadCurrentScene()
	{
		//�V�[�����o�^�ς݂��ǂ���
		auto it = m_sceneList.find(m_currentScene->getType());
		if (it != m_sceneList.end()) {

#ifdef DEBUG_TRUE
			ImGuiApp::InvalidSelectedObject();
#endif

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			//�V�������X�g�ɕς���
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			SFTextManager::GenerateList();
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

	static void SaveScene(std::filesystem::path& _path);
	static void RegisterScene(std::filesystem::path& _path);

private:
	//���߂̃V�[��
	static std::string firstScene;
	//���݂̃V�[���̃p�X
	static std::filesystem::path currentScenePath;
	//�V�[�����X�g
	static std::unordered_map<std::string,std::function<void()>> m_sceneList;

#ifdef DEBUG_TRUE
	struct RegisterSceneNode
	{
		RegisterSceneNode(std::string _path,std::string _name,int _type):
			path(_path),name(_name),type(_type){}
		std::string path;
		std::string name;
		int type;
	};
	static std::vector<RegisterSceneNode> m_registerScenePath;
#endif

	//���̃V�[��
	static std::unique_ptr<Scene, void(*)(Scene*)> m_currentScene;
	//���̃V�[��
	static std::unique_ptr<Scene, void(*)(Scene*)> m_nextScene;
	//�񓯊��p�ϐ�
	static bool async;
	static bool loading;
};


class AssemblyScene final
{
	template<typename T>
	friend class ReflectionScene;

	friend class ImGuiApp;

	class IReflection final
	{
	public:
		~IReflection() = default;

		
		IReflection(std::function<void(void)>&& _register)
			:registerScene(_register){};

		std::function<void(void)> registerScene;
	};
private:
	static inline std::map<std::string, IReflection> assemblies;

public:
	static void RegisterScene(const std::string& _comName)
	{
		auto iter = assemblies.find(_comName);
		if (iter != assemblies.end())
		{
			iter->second.registerScene();
		}
	}
};

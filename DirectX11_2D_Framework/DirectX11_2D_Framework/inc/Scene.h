#pragma once

#define REGISTER_SCENE_LIST_PATH "sceneref.scrf"

static constexpr int REGISTER_SCENE_TYPE_HEADER = 'h';
static constexpr int REGISTER_SCENE_TYPE_FILE	= 'f';

class Scene
{
	friend class Window;
	friend class SceneManager;

protected:
	//継承以外生成禁止
	Scene() {}
	//継承以外削除禁止
	virtual ~Scene() = default;

	//オブジェクト生成
	GameObject* Instantiate();
	//オブジェクト生成(名前指定)
	GameObject* Instantiate(std::string _name);
	//オブジェクト生成(名前,テクスチャ指定)
	GameObject* Instantiate(std::string _name, const wchar_t* _texPath);
	//オブジェクトの削除(ポインタ指定)
	void DeleteObject(GameObject* _object);
	//オブジェクトの削除(名前指定)
	inline void DeleteObject(std::string _name);
private:
	//シーンのロード処理（オブジェクトの生成）
	virtual void Load() {}
	//初期化処理（シーン遷移後に呼び出す）
	virtual void Init() {}
	//毎フレーム呼び出す
	virtual void Update() {}
	//かたずけ処理
	virtual void UnInit() {}

	//オプション： デマングリング関数により、コンパイラによってはより美しい出力が得られる。
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
	//生成禁止
	SceneManager() = delete;

	//最初のシーンの読み込み
	static void Init();
	//シーンの破棄
	static void UnInit();
	//次へのシーンの切り替え
	static void NextScene();
	//同期シーン切り替え
	static void LoadScene(std::string _sceneName)
	{
		//非同期にシーンをロードしている場合
		if (async)
		{
			LOG("no loading %s,other scene loading now", _sceneName.c_str());
		}

		//シーンが登録済みかどうか
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

			//新しいリストに変える
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			SFTextManager::GenerateList();
			//対応したシーンのロード処理
			it->second();
			//シーン切り替え
			NextScene();
			//シーン初期化
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
	//シーンの読み込み・切り替え(同期)
	template<typename T>
	static void LoadScene()
	{
		std::string sceneName = typeid(T).name();

		//非同期にシーンをロードしている場合
		if (async)
		{
			LOG("no loading %s,other scene loading now", sceneName.substr(6).c_str());
		}

		//シーンが登録済みかどうか
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

			//新しいリストに変える
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			SFTextManager::GenerateList();
			//対応したシーンのロード処理
			it->second();
			//シーン切り替え
			NextScene();
			//シーン初期化
			TRY_CATCH_LOG(m_currentScene->Init());

			TextureAssets::LinkNextTextureLib();

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::EnableWorldUpdate();
			Box2D::WorldManager::ResumeWorldUpdate();
#endif	
			LOG("Now Switching to %s", sceneName.substr(6).c_str());

			throw "";
		}

		//シーンが見つからなかった場合
		LOG_WARNING("couldn't find the scene, so registered it.");
		RegisterScene<T>();
		LoadScene<T>();
	}

	//シーンの読み込み(非同期)
	template<typename T>
	static void LoadingScene()
	{
		//ロード中
		if (async) return;

		std::string sceneName = typeid(T).name();

		//シーンが登録されているか
		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end()) {
			async = true;
			loading = true;

			// Start scene loading asynchronously
			LOG("Starting scene loading...%s", sceneName.c_str());

			//スレッドを立てる
			std::future<void> sceneFuture = std::async(std::launch::async, [&]()
				{
					TextureAssets::ChangeNextTextureLib();

					//追加先を新しく変更する
					Box2D::WorldManager::ChangeNextWorld();
					RenderManager::ChangeNextRenderList();
					ObjectManager::ChangeNextObjectList();
					SFTextManager::ChangeNextTextList();
					//生成するウィンドウを表示しない
					Window::WindowSubLoadingBegin();
					//シーンのロード処理
					it->second();
					//古いワールドを削除する
					Box2D::WorldManager::DeleteOldWorld();
				});
			//スレッドが終わるまでループさせる
			Window::WindowUpdate(sceneFuture, loading);

			return;
		}

		//シーンが登録されていなかった場合
		LOG_WARNING("couldn't find the scene, so registered it.");
		RegisterScene<T>();
		LoadingScene<T>();
	}

	//シーンの切り替え(ロードが終わってからじゃないと反映されない)
	static void ChangeScene()
	{
		//非同期でロードが終わっている場合
		if (async && !loading)
		{
#ifdef DEBUG_TRUE
			ImGuiApp::InvalidSelectedObject();
#endif
			//シーンの切り替え
			NextScene();

			//ロードしておいたリストに切り替える
			RenderManager::LinkNextRenderList();

			//現在あるサブウィンドウオブジェクトを隠す
			Window::WindowSubHide();

#ifdef BOX2D_UPDATE_MULTITHREAD
			//ワールドの更新を一時停止
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			ObjectManager::LinkNextObjectList();
			Box2D::WorldManager::LinkNextWorld();

			SFTextManager::LinkNextTextList();

			TextureAssets::LinkNextTextureLib();

#ifdef BOX2D_UPDATE_MULTITHREAD
			//ワールドの更新を再開
			Box2D::WorldManager::ResumeWorldUpdate();
#endif
			//すべてのウィンドウを表示
			Window::WindowSubLoadingEnd();

			async = false;

			//シーン初期化
			TRY_CATCH_LOG(m_currentScene->Init());

			LOG("Now Switching to the New Scene...");

			throw "";
		}
	}

	//シーンクラスをリストに登録する
	template<typename T>
	static void RegisterScene() {
		std::string sceneName = typeid(T).name();

		if (firstScene.empty())
		{
			firstScene = sceneName;
		}

		//シーンが既にある場合
		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end())
		{
			LOG("%s is already registered.", sceneName.substr(6).c_str());
			return;
		}

		RegisterSceneNode registerSceneNode(typeid(T).name(), typeid(T).name(), REGISTER_SCENE_TYPE_HEADER);
		m_registerScenePath.push_back(std::move(registerSceneNode));
			
		//ロード関数を作成
		std::function<void(void)> createFn = [&]()
			{
				currentScenePath = "null";

				//デストラクタ登録
				std::unique_ptr<Scene, void(*)(Scene*)> scene(new T, [](Scene* p) {delete p; });
				m_nextScene = std::move(scene);
#ifdef DEBUG_TRUE
				try{
					m_nextScene->Load();
				}
				//例外キャッチ(nullptr参照とか)
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

		//登録
		m_sceneList[sceneName] = std::move(createFn);
	}

	//現在のシーンをリロード(非同期)
	//※処理を戻さないので呼び出してもその後の処理は継続する
	static void ReloadCurrentScene()
	{
		//シーンが登録済みかどうか
		auto it = m_sceneList.find(m_currentScene->getType());
		if (it != m_sceneList.end()) {

#ifdef DEBUG_TRUE
			ImGuiApp::InvalidSelectedObject();
#endif

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			//新しいリストに変える
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
			SFTextManager::GenerateList();
			//対応したシーンのロード処理
			it->second();
			//シーン切り替え
			NextScene();
			//シーン初期化
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
	//初めのシーン
	static std::string firstScene;
	//現在のシーンのパス
	static std::filesystem::path currentScenePath;
	//シーンリスト
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

	//今のシーン
	static std::unique_ptr<Scene, void(*)(Scene*)> m_currentScene;
	//次のシーン
	static std::unique_ptr<Scene, void(*)(Scene*)> m_nextScene;
	//非同期用変数
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

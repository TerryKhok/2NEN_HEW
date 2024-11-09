#pragma once


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
	virtual void Uninit() {}
};


class SceneManager final
{
	friend class Window;

private:
	//生成禁止
	SceneManager() = delete;

	//最初のシーンの読み込み
	static void Init();
	//シーンの破棄
	static void Uninit();
	//次へのシーンの切り替え
	static void NextScene();
public:
	//シーンの読み込み・切り替え(同期)
	template<typename T>
	static void LoadScene()
	{
		std::string sceneName = typeid(T).name();

		//非同期にシーンをロードしている場合
		if (async)
		{
			LOG("no loding %s,other scene loding now", sceneName.substr(6).c_str());
		}

		//シーンが登録済みかどうか
		auto it = m_sceneList.find(sceneName);
		if (it != m_sceneList.end()) {

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			//新しいリストに変える
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
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
			LOG_NL;
			LOG("Now Switching to %s",sceneName.substr(6).c_str());

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
			LOG_NL;
			LOG("Starting scene loading...%s", sceneName.substr(6).c_str());

			//スレッドを立てる
			std::future<void> sceneFuture = std::async(std::launch::async, [&]()
				{
					//追加先を新しく変更する
					Box2D::WorldManager::ChengeNextWorld();
					RenderManager::ChangeNextRenderList();
					ObjectManager::ChangeNextObjectList();
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
			
		//ロード関数を作成
		std::function<void(void)> createFn = [&]()
			{
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
		m_sceneList[sceneName] =createFn;
	}

	//現在のシーンをリロード(非同期)
	//※処理を戻さないので呼び出してもその後の処理は継続する
	static void ReloadCurrentScene()
	{
		//シーンが登録済みかどうか
		auto it = m_sceneList.find(typeid(*m_currentScene.get()).name());
		if (it != m_sceneList.end()) {

#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::DisableWorldUpdate();
			Box2D::WorldManager::PauseWorldUpdate();
#endif
			//新しいリストに変える
			RenderManager::GenerateList();
			ObjectManager::GenerateList();
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

private:
	//初めのシーン
	static std::string firstScene;
	//シーンリスト
	static std::unordered_map<std::string,std::function<void()>> m_sceneList;
	//今のシーン
	static std::unique_ptr<Scene, void(*)(Scene*)> m_currentScene;
	//次のシーン
	static std::unique_ptr<Scene, void(*)(Scene*)> m_nextScene;
	//非同期用変数
	static bool async;
	static bool loading;
};

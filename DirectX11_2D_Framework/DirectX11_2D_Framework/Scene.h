#pragma once

class Scene
{
	friend class Window;
	friend class SceneManager;

protected:
	//継承以外生成禁止
	Scene() {}

	//オブジェクト生成
	GameObject* Instantiate();
	//オブジェクト生成(名前指定)
	GameObject* Instantiate(std::string _name);
	//オブジェクト生成(名前,テクスチャ指定)
	GameObject* Instantiate(std::string _name, const wchar_t* _texPath);
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


class SceneManager
{
	friend class Window;

private:
	//生成禁止
	SceneManager();

	//最初のシーンの読み込み
	static void Init();
	//次へのシーンの切り替え
	static void NextScene();
public:
	//シーンの読み込み・切り替え(同期)
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

	//シーンの読み込み(非同期)
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

	//シーンの切り替え(ロードが終わってからじゃないと反映されない)
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

	//シーンクラスをリストに登録する
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
	//シーンリスト
	static std::unordered_map<std::string, std::function<void()>> m_sceneList;
	//今のシーン
	static std::unique_ptr<Scene> m_currentScene;
	//次のシーン
	static std::unique_ptr<Scene> m_nextScene;
	//非同期用変数
	static bool async;
	static bool loading;
};

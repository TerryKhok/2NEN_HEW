
std::string SceneManager::firstScene;
std::unordered_map<std::string, std::function<void()>> SceneManager::m_sceneList;
std::unique_ptr<Scene, void(*)(Scene*)> SceneManager::m_currentScene(nullptr, [](Scene* p) {delete p; });
std::unique_ptr<Scene, void(*)(Scene*)> SceneManager::m_nextScene(nullptr, [](Scene* p) {delete p; });
bool SceneManager::async = false;
bool SceneManager::loading = false;

GameObject* Scene::Instantiate()
{
    GameObject* object = new GameObject();
    return ObjectManager::AddObject(object);
}

GameObject* Scene::Instantiate(std::string _name)
{
    GameObject* object = new GameObject(_name);
    return ObjectManager::AddObject(object);
}

GameObject* Scene::Instantiate(std::string _name, const wchar_t* _texPath)
{
    GameObject* object = new GameObject(_name);
    object->AddComponent<Renderer>(_texPath);
    return ObjectManager::AddObject(object);
}

void Scene::DeleteObject(GameObject* _object)
{
    DeleteObject(_object->name);
}

void Scene::DeleteObject(std::string _name)
{
    auto& list = ObjectManager::m_currentList;
    auto iter = list->find(_name);
    if (iter != list->end())
    {
#ifdef DEBUG_TRUE
        PointerRegistryManager::deletePointer(iter->second.get());
#endif
        list->erase(iter);
    }
}

void SceneManager::Init()
{
    if (m_sceneList.empty())
    {
        MessageBoxA(NULL, "シーンが一つも登録されていません", "エラー", MB_ICONERROR | MB_OK);
        PostQuitMessage(0);
    }

    //ロード
    m_sceneList[firstScene]();

    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr);

    m_currentScene->Init();
}

void SceneManager::UnInit()
{
    m_currentScene.reset();
    m_nextScene.reset();
}

void SceneManager::NextScene()
{
    //シーンのかたずけ
    TRY_CATCH_LOG(m_currentScene->UnInit());

    //古いオブジェクトの移動情報を一応消しておく
    Box2DBodyManager::m_moveBodyObjects.clear();

    //レンダーカメラ設定初期化
    RenderManager::renderZoom = { 1.0f,1.0f };
    RenderManager::renderOffset = { 0.0f,0.0f };
    
    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr); 
}


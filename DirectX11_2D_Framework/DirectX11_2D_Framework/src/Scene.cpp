
std::string SceneManager::firstScene;
std::filesystem::path SceneManager::currentScenePath = "null";
std::unordered_map<std::string, std::function<void()>> SceneManager::m_sceneList;

#ifdef DEBUG_TRUE
std::vector<SceneManager::RegisterSceneNode> SceneManager::m_registerScenePath;
#endif

std::unique_ptr<Scene, void(*)(Scene*)> SceneManager::m_currentScene(nullptr, [](Scene* p) {delete p; });
std::unique_ptr<Scene, void(*)(Scene*)> SceneManager::m_nextScene(nullptr, [](Scene* p) {delete p; });
bool SceneManager::async = false;
bool SceneManager::loading = false;

GameObject* Scene::Instantiate()
{
    GameObject* object = new GameObject();
    ObjectManager::AddObject(object);
    return object;
}

GameObject* Scene::Instantiate(std::string _name)
{
    GameObject* object = new GameObject(_name);
    ObjectManager::AddObject(object);
    return object;
}

GameObject* Scene::Instantiate(std::string _name, const wchar_t* _texPath)
{
    GameObject* object = new GameObject(_name);
    object->AddComponent<Renderer>(_texPath);
    ObjectManager::AddObject(object);
    return object;
}

void Scene::DeleteObject(GameObject* _object)
{
    DeleteObject(_object->name);
}

void Scene::DeleteObject(std::string _name)
{
    auto& list = ObjectManager::m_currentList;
    auto iter = list->second.find(_name);
    if (iter != list->second.end())
    {
#ifdef DEBUG_TRUE
        PointerRegistryManager::deletePointer(list->first[iter->second].get());
#endif
        //list->erase(iter);
        ObjectManager::DeleteObject(iter);
    }
}

void SceneManager::Init()
{
    std::ifstream fin;
    fin.open(REGISTER_SCENE_LIST_PATH, std::ios::in | std::ios::binary);

    bool open = fin.is_open();
    if (open)
    {
        //プレイヤーの数を読み込む
        int sceneNum = 0;
        fin.read((char*)&sceneNum, sizeof(sceneNum));

        for (int i = 0; i < sceneNum; i++)
        {
            int type = 0;
            fin.read((char*)&type, sizeof(type));
            size_t pathSize = 0;
            fin.read((char*)&pathSize, sizeof(pathSize));
            std::string path;
            path.resize(pathSize);
            fin.read((char*)path.c_str(), pathSize);
            switch (type)
            {
            case REGISTER_SCENE_TYPE_HEADER:
                AssemblyScene::RegisterScene(path);
                break;
            case REGISTER_SCENE_TYPE_FILE:
            {
                std::filesystem::path filepath(path);
                SceneManager::RegisterScene(filepath);
                break;
            }
            default:
                LOG_ERROR("this register scene list file type Not compatible");
                break;
            }
        }
    }
    else
    {
        SceneManager::RegisterScene<SampleScene>();
    }
    
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

#ifdef DEBUG_TRUE
    //登録しているパスを保存する
    std::ofstream fout;
    fout.open(REGISTER_SCENE_LIST_PATH, std::ios::out | std::ios::binary);
    assert(fout.is_open() && "アニメーションファイル書き込みに失敗しました");

    int sceneNum = (int)m_registerScenePath.size();
    fout.write((char*)&sceneNum, sizeof(sceneNum));

    for (auto& node : m_registerScenePath)
    {
        auto& type = node.type;
        fout.write((char*)&type, sizeof(type));
        auto& path = node.path;
        size_t pathSize = path.size();
        fout.write((char*)&pathSize, sizeof(pathSize));
        fout.write((char*)path.c_str(), pathSize);
    }
#endif
}

void SceneManager::NextScene()
{
    //シーンのかたずけ
    TRY_CATCH_LOG(m_currentScene->UnInit());

    //レンダーカメラ設定初期化
    RenderManager::renderZoom = { 1.0f,1.0f };
    RenderManager::renderOffset = { 0.0f,0.0f };
    
    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr); 
}


void SceneManager::SaveScene(std::filesystem::path& _path)
{
    std::ofstream ofs(_path);
    {
        SERIALIZE_OUTPUT archive(ofs);

        int index = SceneFileIndex;
        archive(CEREAL_NVP(index));

        archive(::cereal::make_nvp("objectNum", ObjectManager::m_currentList->first.size()));
        for (auto& object : ObjectManager::m_currentList->first)
        {
            archive(*object);
        }
    }
}


void SceneManager::RegisterScene(std::filesystem::path& _path)
{
    std::string sceneName = _path.stem().string();

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

#ifdef DEBUG_TRUE
    RegisterSceneNode registerSceneNode(_path.string(), sceneName, REGISTER_SCENE_TYPE_FILE);
    m_registerScenePath.push_back(std::move(registerSceneNode));
#endif

    //ロード関数を作成
    std::function<void(void)> createFn = std::bind([](std::filesystem::path scenePath)
        {
            currentScenePath = scenePath;

            //デストラクタ登録
            std::unique_ptr<Scene, void(*)(Scene*)> scene(new FileScene(scenePath), [](Scene* p) {delete p; });
            m_nextScene = std::move(scene);
#ifdef DEBUG_TRUE
            try {
                m_nextScene->Load();
            }
            //例外キャッチ(nullptr参照とか)
            catch (const std::exception& e) {
                LOG_ERROR(e.what());
            }
#else
            try {
                m_nextScene->Load();
            }
            catch (...) {}
#endif
        },_path);

    //登録
    m_sceneList[sceneName] = std::move(createFn);
}


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
        //�v���C���[�̐���ǂݍ���
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
        MessageBoxA(NULL, "�V�[��������o�^����Ă��܂���", "�G���[", MB_ICONERROR | MB_OK);
        PostQuitMessage(0);
    }

    //���[�h
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
    //�o�^���Ă���p�X��ۑ�����
    std::ofstream fout;
    fout.open(REGISTER_SCENE_LIST_PATH, std::ios::out | std::ios::binary);
    assert(fout.is_open() && "�A�j���[�V�����t�@�C���������݂Ɏ��s���܂���");

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
    //�V�[���̂�������
    TRY_CATCH_LOG(m_currentScene->UnInit());

    //�����_�[�J�����ݒ菉����
    RenderManager::renderZoom = { 1.0f,1.0f };
    RenderManager::renderOffset = { 0.0f,0.0f };
    
    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr); 
}

void SceneManager::LoadSceneNotThrow(std::string _sceneName)
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

void SceneManager::LoadScene(std::string _sceneName)
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

        throw "";
    }
}

void SceneManager::LoadScene(std::stringstream& _buffer)
{
#ifdef BOX2D_UPDATE_MULTITHREAD
    Box2D::WorldManager::DisableWorldUpdate();
    Box2D::WorldManager::PauseWorldUpdate();
#endif

#ifdef DEBUG_TRUE
    ImGuiApp::InvalidSelectedObject();
#endif
    //�V�������X�g�ɕς���
    RenderManager::GenerateList();
    ObjectManager::GenerateList();
    SFTextManager::GenerateList();

    {
        SERIALIZE_INPUT archive(_buffer);

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

    ObjectManager::ProceedObjectComponent();

#ifdef BOX2D_UPDATE_MULTITHREAD
    Box2D::WorldManager::EnableWorldUpdate();
    Box2D::WorldManager::ResumeWorldUpdate();
#endif	

    throw "";
}

void SceneManager::LoadingScene(std::string _sceneName)
{
    //���[�h��
    if (async) return;

    //�V�[�����o�^����Ă��邩
    auto it = m_sceneList.find(_sceneName);
    if (it != m_sceneList.end()) {
        async = true;
        loading = true;

        // Start scene loading asynchronously
        LOG("Starting scene loading...%s", _sceneName.c_str());

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
}

void SceneManager::ChangeScene()
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


void SceneManager::ReloadCurrentScene()
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

void SceneManager::SaveScene(std::stringstream& _buffer)
{
    {
        SERIALIZE_OUTPUT archive(_buffer);

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

    //�V�[�������ɂ���ꍇ
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

    //���[�h�֐����쐬
    std::function<void(void)> createFn = std::bind([](std::filesystem::path scenePath)
        {
            currentScenePath = scenePath;

            //�f�X�g���N�^�o�^
            std::unique_ptr<Scene, void(*)(Scene*)> scene(new FileScene(scenePath), [](Scene* p) {delete p; });
            m_nextScene = std::move(scene);
#ifdef DEBUG_TRUE
            try {
                m_nextScene->Load();
                ObjectManager::ProceedObjectComponent();
            }
            //��O�L���b�`(nullptr�Q�ƂƂ�)
            catch (const std::exception& e) {
                LOG_ERROR(e.what());
            }
#else
            try {
                m_nextScene->Load();
                ObjectManager::ProceedObjectComponent();
            }
            catch (...) {}
#endif
        },_path);

    //�o�^
    m_sceneList[sceneName] = std::move(createFn);
}

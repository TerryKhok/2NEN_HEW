#include "Scene.h"

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

    auto it = m_sceneList.begin();
    it->second();

    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr);

    m_currentScene->Init();
}

void SceneManager::Uninit()
{
    m_currentScene.reset();
    m_nextScene.reset();
}

void SceneManager::NextScene()
{
#ifdef DEBUG_TRUE
    try
    {
        m_currentScene->Uninit();
    }
    //例外キャッチ(nullptr参照とか)
    catch (const std::exception& e) {
        LOG_ERROR(e.what());
    }
#else
    m_currentScene->Uninit();
#endif

    //古いオブジェクトの移動情報を一応消しておく
    Box2DBodyManager::moveFunctions.clear();
    
    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr);

#ifdef DEBUG_TRUE
    try
    {
        m_currentScene->Init();
    }
    //例外キャッチ(nullptr参照とか)
    catch (const std::exception& e) {
        LOG_ERROR(e.what());
    }
#else
    m_currentScene->Init();
#endif
}


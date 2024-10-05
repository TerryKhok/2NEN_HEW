#include "Scene.h"

std::unordered_map<std::string, std::pair<int, std::function<void()>>> SceneManager::m_sceneList;
std::unique_ptr<Scene> SceneManager::m_currentScene = nullptr;
std::unique_ptr<Scene> SceneManager::m_nextScene = nullptr;
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

void SceneManager::Init()
{
    if (m_sceneList.empty())
    {
        assert(false && "sceneList is empty!");
    }

    auto it = m_sceneList.begin();
    it->second.second();

    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr);

    m_currentScene->Init();
}

void SceneManager::NextScene()
{
    m_currentScene->Uninit();

    m_currentScene = std::move(m_nextScene);
    //m_nextScene.reset(nullptr);

    m_currentScene->Init();
}


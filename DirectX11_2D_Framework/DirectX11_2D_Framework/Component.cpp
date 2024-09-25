#include "Component.h"


GameObject* Component::Instantiate()
{
    GameObject* object = new GameObject();
    return ObjectManager::AddObject(object);
}

GameObject* Component::Instantiate(std::string _name)
{
    GameObject* object = new GameObject(_name);
    return ObjectManager::AddObject(object);
}

GameObject* Component::Instantiate(std::string _name, const wchar_t* _texPath)
{
    GameObject* object = new GameObject(_name);
    object->AddComponent<Renderer>(_texPath);
    return ObjectManager::AddObject(object);
}
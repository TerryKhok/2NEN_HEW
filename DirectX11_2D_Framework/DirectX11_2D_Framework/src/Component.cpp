
GameObject* Component::Instantiate()
{
    GameObject* object = new GameObject();
    ObjectManager::AddObject(object);
    return object;
}

GameObject* Component::Instantiate(std::string _name)
{
    GameObject* object = new GameObject(_name);
    ObjectManager::AddObject(object);
    return object;
}

GameObject* Component::Instantiate(std::string _name, const wchar_t* _texPath)
{
    GameObject* object = new GameObject(_name);
    object->AddComponent<Renderer>(_texPath);
    ObjectManager::AddObject(object);
    return object;
}

void Component::DeleteObject(GameObject* _object)
{
    DeleteObject(_object->name);
}

void Component::DeleteObject(std::string _name)
{
    ObjectManager::pDeleteObject(_name);
}


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

void Component::DeleteObject(GameObject* _object)
{
    DeleteObject(_object->name);
}

void Component::DeleteObject(std::string _name)
{
    auto& list = ObjectManager::m_currentList;
    auto iter = list->find(_name);
    if (iter != list->end())
    {
        auto& b2bodyMoveList = Box2DBodyManager::moveFunctions;
        auto it = b2bodyMoveList.find(_name); 
        if (it != b2bodyMoveList.end())
        {
            it->second();
            b2bodyMoveList.erase(it);
        }

#ifdef DEBUG_TRUE
        PointerRegistryManager::deletePointer(iter->second.get());
#endif
        list->erase(iter);

        if (m_this->GetName() == _name)
        {
            throw "";
        }
    }
}
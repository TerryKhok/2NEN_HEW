#include "Object.h"

VSConstantBuffer GameObject::m_cb = {};

thread_local ObjectManager::ObjectList* ObjectManager::m_currentList = m_objectList.get();
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_objectList = std::unique_ptr<ObjectList>(new ObjectList());
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_nextObjectList = std::unique_ptr<ObjectList>(new ObjectList());

GameObject::GameObject(std::string _name)
{
	name = _name;
	transform.gameobject = this;
}

GameObject::~GameObject()
{
	for (auto& component : m_componentList)
	{
		component.second->Delete();
	}

	m_componentList.clear();
}

VSConstantBuffer& GameObject::GetContantBuffer()
{
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	m_cb.world = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, 1.0f);
	m_cb.world *= DirectX::XMMatrixRotationZ(transform.angle.z);
	m_cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, 0.5f);
	m_cb.world = DirectX::XMMatrixTranspose(m_cb.world);
	m_cb.projection = DirectX::XMMatrixOrthographicLH(
		SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);
	m_cb.projection = DirectX::XMMatrixTranspose(m_cb.projection);

	return m_cb;
}

void GameObject::UpdateComponent()
{
	for (auto& component : m_componentList)
	{
		component.second->Update();
	}
}


void GameObject::SetActive(bool _active)
{
	for (auto& component : m_componentList)
	{
		component.second->SetActive(_active);
	}
}

void GameObject::SetLayer(const LAYER _layer)
{
	for (auto& component : m_componentList)
	{
		component.second->SetLayer(_layer);
	}
	//最後に変更する
	m_layer = _layer;
	for (auto& component : m_componentList)
	{
		component.second->SetLayer(_layer);
	}
}

void GameObject::SetName(const std::string _name)
{
	ObjectManager::ChangeObjectName(name, _name);
	name = _name;
}

const std::string GameObject::GetName() const
{
	return name;
}

const LAYER GameObject::GetLayer() const
{
	return m_layer;
}

template<>
Renderer* GameObject::AddComponent<Renderer>()
{
	if (ExistComponent<Renderer>()) return nullptr;

	Component* component = nullptr;
	component = new Renderer(this);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.insert(std::make_pair(typeid(Renderer).name(), 
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

	Renderer* render = dynamic_cast<Renderer*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast faild", typeid(Renderer).name());
	}

	return render;
}


template<>
Renderer* GameObject::AddComponent<Renderer>(const wchar_t* _texPath)
{
	if (ExistComponent<Renderer>()) return nullptr;

	Component* component = nullptr;
	component = new Renderer(this, _texPath);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.insert(std::make_pair(typeid(Renderer).name(),
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

	Renderer* render = dynamic_cast<Renderer*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast faild", typeid(Renderer).name());
	}

	return render;
}

template<>
Transform* GameObject::GetComponent()
{
	return &transform;
}

GameObject* ObjectManager::Find(std::string _name)
{
	for (auto& node : m_currentList->second)
	{
		if (node->GetName() == _name)
		{
			return node.get();
		}
	}

	return nullptr;

	LOG("not found %s gameObject", _name.c_str());
}

void ObjectManager::Uninit()
{
	for (auto& object : m_objectList->second)
	{
		object.reset();
	}
}

void ObjectManager::UpdateObjectComponent()
{
	for (auto& object : m_objectList->second)
	{
		object->UpdateComponent();
	}
}

void ObjectManager::GenerateList()
{
	m_objectList.reset(new ObjectList());
	m_currentList = m_objectList.get();
}

GameObject* ObjectManager::AddObject(GameObject* _gameObject)
{
	int count = 1;
	auto& name = _gameObject->name;
	auto& nameSet = m_currentList->first;
	std::string uniqueName = name;

	// Check if the name already exists in the set, and generate a new one if necessary
	while (nameSet.find(uniqueName) != nameSet.end()) {
		uniqueName = name + "_" + std::to_string(count++);
	}

	// Insert the object and update the set with the new unique name
	nameSet.insert(uniqueName);

#ifdef DEBUG_TRUE
	if (name != uniqueName)
	{
		LOG("%s name existed, so we changed %s.", name.c_str(), uniqueName.c_str());
	}
#endif

	name = uniqueName;

	//デストラクタをスマートポインタに登録
	m_currentList->second.push_back(std::unique_ptr<GameObject, void(*)(GameObject*)>
		(_gameObject, [](GameObject* p){delete p;}));

	auto it = m_currentList->second.end() - 1;
	return it->get();
}

void ObjectManager::ChangeNextObjectList()
{
	m_currentList = m_nextObjectList.get();
}

void ObjectManager::LinkNextObjectList()
{
	m_objectList = std::move(m_nextObjectList);

	m_nextObjectList.reset(new ObjectList());
}

void ObjectManager::ChangeObjectName(std::string _before, std::string _after)
{
	auto& list = m_currentList->first;
	auto iter = list.find(_before);
	if (iter != list.end())
	{
		list.erase(iter);
		list.insert(_after);
	}
}

#include "Object.h"

VSConstantBuffer GameObject::m_cb = {};

thread_local ObjectManager::ObjectList* ObjectManager::m_currentList = m_objectList.get();
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_objectList = std::unique_ptr<ObjectList>(new ObjectList());
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_nextObjectList = std::unique_ptr<ObjectList>(new ObjectList());
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_eraseObjectList = std::unique_ptr<ObjectList>(new ObjectList());

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
	m_cb.world = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z);
	m_cb.world *= DirectX::XMMatrixRotationZ(static_cast<float>(transform.angle.z.Get()));
	m_cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
	m_cb.world = DirectX::XMMatrixTranspose(m_cb.world);

	/*GameObject::m_cb.projection = DirectX::XMMatrixOrthographicLH(
		SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);
	GameObject::m_cb.projection = DirectX::XMMatrixTranspose(GameObject::m_cb.projection);
	*/

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
Box2DBody* GameObject::AddComponent(b2BodyDef* _bodyDef)
{
	if (ExistComponent<Box2DBody>()) return nullptr;

	Component* component = nullptr;
	component = new Box2DBody(this, _bodyDef);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.insert(std::make_pair(typeid(Box2DBody).name(),
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

	Box2DBody* render = dynamic_cast<Box2DBody*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast faild", typeid(Box2DBody).name());
	}

	return render;
}

template<>
Box2DBody* GameObject::AddComponent<Box2DBody>()
{
	if (ExistComponent<Box2DBody>()) return nullptr;

	Component* component = nullptr;
	component = new Box2DBody(this);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.insert(std::make_pair(typeid(Box2DBody).name(),
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

	Box2DBody* render = dynamic_cast<Box2DBody*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast faild", typeid(Box2DBody).name());
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
	auto iter = m_currentList->find(_name);
	if (iter != m_currentList->end())
	{
		return iter->second.get();
	}

	LOG("not found %s gameObject", _name.c_str());
	return nullptr;
}


void ObjectManager::Uninit()
{
	for (auto& [key, value] : *m_objectList)
	{
		value.reset();
	}
}

void ObjectManager::UpdateObjectComponent()
{
	for (auto& [key, value] : *m_objectList)
	{
		value->UpdateComponent();
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
	std::string uniqueName = name;

	while (m_currentList->find(uniqueName) != m_currentList->end()){
		uniqueName = name + "_" + std::to_string(count++);
	}

#ifdef DEBUG_TRUE
	if (name != uniqueName)
	{
		LOG("%s name existed, so we changed %s.", name.c_str(), uniqueName.c_str());
	}
#endif

	name = uniqueName;

	//デストラクタと一緒にスマートポインタに登録
	m_currentList->emplace(name, std::unique_ptr<GameObject, void(*)(GameObject*)>
		(_gameObject, [](GameObject* p) {delete p; }));
	
	return m_currentList->find(name)->second.get();
}

void ObjectManager::ChangeNextObjectList()
{
	m_eraseObjectList.reset();
	m_nextObjectList.reset(new ObjectList());
	m_currentList = m_nextObjectList.get();
}

void ObjectManager::LinkNextObjectList()
{
	m_eraseObjectList = std::move(m_objectList);

	m_objectList = std::move(m_nextObjectList);

	m_currentList = m_objectList.get();
}

void ObjectManager::ChangeObjectName(std::string _before, std::string _after)
{
	std::unique_ptr<GameObject, void(*)(GameObject*)> object(nullptr, [](GameObject* p) {delete p; });
	auto iter = m_currentList->find(_before);
	if (iter != m_currentList->end())
	{
		object = std::move(iter->second);
		m_currentList->erase(iter);
	}

	m_currentList->emplace(_after, std::move(object));
}

void ObjectManager::CleanAllObjectList()
{
	m_objectList.reset();
	m_nextObjectList.reset();
	m_eraseObjectList.reset();
}

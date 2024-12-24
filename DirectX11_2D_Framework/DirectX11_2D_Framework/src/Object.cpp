
VSObjectConstantBuffer GameObject::m_cb = {};

thread_local void(*ObjectManager::pDeleteObject)(std::string) = &ObjectManager::DeleteObjectDelay;
thread_local ObjectManager::ObjectList* ObjectManager::m_currentList = m_objectList.get();
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_objectList = std::unique_ptr<ObjectList>(new ObjectList());
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_nextObjectList = std::unique_ptr<ObjectList>(new ObjectList());
std::unique_ptr<ObjectManager::ObjectList> ObjectManager::m_eraseObjectList = std::unique_ptr<ObjectList>(new ObjectList());
std::vector<std::string> ObjectManager::m_delayEraseObjectName;

GameObject::GameObject(std::string _name)
{
	name = _name;
	transform.gameobject = this;
}

GameObject::~GameObject()
{
	for (auto& component : m_componentList.first)
	{
#ifdef DEBUG_TRUE
		//コンポーネントのSafePointerをNullptrにする
		PointerRegistryManager::deletePointer(component.get());
#endif 
		component->Delete();
	}

	m_componentList.first.clear();
	m_componentList.second.clear();
}

VSObjectConstantBuffer& GameObject::GetConstantBuffer()
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
	for (auto& component : m_componentList.first)
	{
		component->Update();
	}
}


void GameObject::SetActive(bool _active)
{
	if (active != _active) active = _active; else return;

	for (auto& component : m_componentList.first)
	{
		component->SetActive(_active);
	}
	//更新関数ポインターを設定
	pUpdate = _active ? &GameObject::UpdateComponent : &GameObject::Void;

#ifdef DEBUG_TRUE
	if (!active)isSelected = SELECT_NONE;
#endif
}

void GameObject::SetName(const std::string _name)
{
	ObjectManager::ChangeObjectName(name, _name);
}

const std::string GameObject::GetName() const
{
	return name;
}

template<>
SAFE_TYPE(Renderer) GameObject::AddComponent<Renderer>()
{
	if (ExistComponent<Renderer>()) return GetComponent<Renderer>();

	Component* component = nullptr;
	component = new Renderer(this);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(Renderer).name()] = m_componentList.first.size() - 1;

	Renderer* render = dynamic_cast<Renderer*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(Renderer).name());
	}

	return render;
}


template<>
SAFE_TYPE(Renderer) GameObject::AddComponent<Renderer>(const wchar_t* _texPath)
{
	if (ExistComponent<Renderer>()) return GetComponent<Renderer>();

	Component* component = nullptr;
	component = new Renderer(this, _texPath);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(Renderer).name()] = m_componentList.first.size() - 1;

	Renderer* render = dynamic_cast<Renderer*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(Renderer).name());
	}

	return render;
}

template<>
SAFE_TYPE(Renderer) GameObject::AddComponent(Animator* _animator)
{
	if (ExistComponent<Renderer>()) return GetComponent<Renderer>();

	Component* component = nullptr;
	component = new Renderer(this, _animator);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(Renderer).name()] = m_componentList.first.size() - 1;

	Renderer* render = dynamic_cast<Renderer*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(Renderer).name());
	}

	return render;
}

template<>
SAFE_TYPE(Animator) GameObject::AddComponent<Animator>()
{
	if (ExistComponent<Animator>()) return GetComponent<Animator>();

	Component* component = nullptr;
	component = new Animator(this);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(Animator).name()] = m_componentList.first.size() - 1;

	Animator* animator = dynamic_cast<Animator*>(component);
	if (animator == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(Animator).name());
	}

	return animator;
}

template<>
SAFE_TYPE(Box2DBody) GameObject::AddComponent<Box2DBody>()
{
	if (ExistComponent<Box2DBody>()) return GetComponent<Box2DBody>();

	Component* component = nullptr;
	component = new Box2DBody(this);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(Box2DBody).name()] = m_componentList.first.size() - 1;

	Box2DBody* render = dynamic_cast<Box2DBody*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(Box2DBody).name());
	}

	return render;
}

template<>
SAFE_TYPE(Box2DBody) GameObject::AddComponent(b2BodyDef* _bodyDef)
{
	if (ExistComponent<Box2DBody>()) return GetComponent<Box2DBody>();

	Component* component = nullptr;
	component = new Box2DBody(this, _bodyDef);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(Box2DBody).name()] = m_componentList.first.size() - 1;

	Box2DBody* render = dynamic_cast<Box2DBody*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(Box2DBody).name());
	}

	return render;
}

template<>
SAFE_TYPE(SubWindow) GameObject::AddComponent<SubWindow>()
{
	if (ExistComponent<SubWindow>()) return GetComponent<SubWindow>();

	Component* component = nullptr;
	component = new SubWindow(this);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(SubWindow).name()] = m_componentList.first.size() - 1;

	SubWindow* render = dynamic_cast<SubWindow*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(SubWindow).name());
	}

	return render;
}

template<>
SAFE_TYPE(SubWindow) GameObject::AddComponent(const char* _windowName)
{
	if (ExistComponent<SubWindow>()) return GetComponent<SubWindow>();

	Component* component = nullptr;
	component = new SubWindow(this, _windowName);
	component->m_this = this;

	//リストに追加(デストラクタ登録)
	m_componentList.first.emplace_back(
		std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
	m_componentList.second[typeid(SubWindow).name()] = m_componentList.first.size() - 1;

	SubWindow* render = dynamic_cast<SubWindow*>(component);
	if (render == nullptr)
	{
		LOG_WARNING("%s component down_cast failed", typeid(SubWindow).name());
	}

	return render;
}

template<>
void GameObject::RemoveComponent<Renderer>()
{
	RemoveComponent<Animator>();

	auto iter = m_componentList.second.find(typeid(Renderer).name());
	if (iter != m_componentList.second.end())
	{
		auto& component = m_componentList.first[iter->second];
#ifdef DEBUG_TRUE
		//コンポーネントのSafePointerをNullptrにする
		PointerRegistryManager::deletePointer(component.get());
#endif 
		auto& vector = m_componentList.first;
		auto& map = m_componentList.second;

		component->Delete();
		size_t index = iter->second;
		size_t lastIndex = vector.size() - 1;

		// Move the last entity to the position of the entity to be removed
		if (index != lastIndex) {
			vector[index] = std::move(vector[lastIndex]);
			map[vector[index]->getType().c_str()] = index; // Update map for the moved entity
		}

		// Remove the last element and update map
		vector.pop_back();
		map.erase(iter);
	}
}

template<>
SAFE_TYPE(Transform) GameObject::GetComponent()
{
	return &transform;
}

template<>
bool GameObject::TryGetComponent(Transform** _output)
{
	*_output = &transform;
	return true;
}

SAFE_TYPE(GameObject) ObjectManager::Find(const std::string& _name)
{
	auto iter = m_currentList->second.find(_name);
	if (iter != m_currentList->second.end())
	{
		return m_currentList->first[iter->second].get();
	}

	LOG("not found %s gameObject", _name.c_str());
	return nullptr;
}


void ObjectManager::UnInit()
{
	for (auto& object: m_objectList->first)
	{
		object.reset();
	}
}

void ObjectManager::UpdateObjectComponent()
{
	for (auto& object : m_objectList->first)
	{
		//参照を剥がして関数ポインタに直接アクセス
		//(しかし、関数を介して関数ポインタにアクセスするのと速度はあまり変わらない)
		(object.get()->*object->pUpdate)();
	}

	for (auto& name : m_delayEraseObjectName)
	{
		DeleteObject(name);
	}
	m_delayEraseObjectName.clear();
}

void ObjectManager::GenerateList()
{
	m_objectList.reset(new ObjectList());
	m_currentList = m_objectList.get();
}

void ObjectManager::AddObject(GameObject* _gameObject)
{
	int count = 1;
	auto& name = _gameObject->name;
	std::string uniqueName = name;

	while (m_currentList->second.find(uniqueName) != m_currentList->second.end()){
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
	m_currentList->first.push_back(std::unique_ptr<GameObject, void(*)(GameObject*)>
		(_gameObject, [](GameObject* p) {delete p; }));

	m_currentList->second[uniqueName] = m_currentList->first.size() - 1;
	
	//return m_currentList->first.back().get();
}

void ObjectManager::AddObject(std::filesystem::path& _path)
{
	// Deserialize from a file
	GameObject* object = new GameObject;
	{
		std::ifstream is(_path.string());
		cereal::JSONInputArchive archive(is);

		archive(*object);  // Deserialize polymorphic object
	}

	//デストラクタと一緒にスマートポインタに登録
	m_currentList->first.push_back(std::unique_ptr<GameObject, void(*)(GameObject*)>
		(object, [](GameObject* p) {delete p; }));

	m_currentList->second[object->GetName()] = m_currentList->first.size() - 1;
}

void ObjectManager::DeleteObject(ObjectListMap::iterator& _iter)
{
	size_t index = _iter->second;
	size_t lastIndex = m_currentList->first.size() - 1;

	auto& vector = m_currentList->first;
	auto& map = m_currentList->second;

	//リストの最後の要素と入れ替える
	if (index != lastIndex) {
		vector[index] = std::move(vector[lastIndex]);
		map[vector[index]->GetName()] = index; //入れ替えた要素の番号を変更
	}

	// Remove the last element and update map
	vector.pop_back();
	map.erase(_iter);
}

void ObjectManager::DeleteObject(std::string _name)
{
	auto& list = ObjectManager::m_currentList;
	auto iter = list->second.find(_name);
	if (iter != list->second.end())
	{
		auto& targetObj = list->first[iter->second];
	

#ifdef DEBUG_TRUE
		PointerRegistryManager::deletePointer(targetObj.get());
#endif
		ObjectManager::DeleteObject(iter);
	}
}

void ObjectManager::DeleteObjectDelay(std::string _name)
{
	m_delayEraseObjectName.push_back(_name);
}

void ObjectManager::ChangeNextObjectList()
{
	m_eraseObjectList.reset();
	m_nextObjectList.reset(new ObjectList());
	m_currentList = m_nextObjectList.get();
	pDeleteObject = &ObjectManager::DeleteObject;
}

void ObjectManager::LinkNextObjectList()
{
	m_eraseObjectList = std::move(m_objectList);

	m_objectList = std::move(m_nextObjectList);

	m_currentList = m_objectList.get();
}

void ObjectManager::ChangeObjectName(const std::string& _before, const std::string& _after)
{
	std::unique_ptr<GameObject, void(*)(GameObject*)> object(nullptr, [](GameObject* p) {delete p; });
	auto iter = m_currentList->second.find(_before);
	if (iter != m_currentList->second.end())
	{
		int count = 1;
		std::string uniqueName = _after;

		while (m_currentList->second.find(uniqueName) != m_currentList->second.end()) {
			uniqueName = _after + "_" + std::to_string(count++);
		}

#ifdef DEBUG_TRUE
		if (_after != uniqueName)
		{
			LOG("%s name existed, so we changed %s.", _after.c_str(), uniqueName.c_str());
		}
#endif
		size_t index = iter->second;
		//size_t lastIndex = m_currentList->first.size() - 1;
		m_currentList->second.erase(iter);
		m_currentList->second[uniqueName] = index;

		Box2DBody* rb = nullptr;
		if (m_currentList->first[index]->TryGetComponent<Box2DBody>(&rb))
		{
			auto it = Box2DBodyManager::m_bodyObjectName.find(rb->m_bodyId.index1);
			if (it != Box2DBodyManager::m_bodyObjectName.end())
			{
				it->second = uniqueName;
			}
		}

		m_currentList->first[index]->name = uniqueName;
	}
}

void ObjectManager::CleanAllObjectList()
{
	m_objectList.reset();
	m_nextObjectList.reset();
	m_eraseObjectList.reset();
}

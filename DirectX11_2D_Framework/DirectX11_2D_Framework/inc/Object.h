#pragma once

//Scale = 1.0f �̂Ƃ��̑傫��
//================================================================
constexpr float DEFAULT_OBJECT_SIZE = 10.0f;
constexpr float HALF_OBJECT_SIZE = DEFAULT_OBJECT_SIZE / 2.0f;
constexpr float QUARTER_OBJECT_SIZE = DEFAULT_OBJECT_SIZE / 4.0f;
//================================================================

class Renderer;
class SubWindow;

struct Transform final
{
	//�������m�ۋ֎~
	void* operator new(size_t) = delete;

	GameObject* gameobject = nullptr;
	Vector3 position = { 0.0f,0.0f,0.0f };
	Vector3 scale =	{ 5.0f,5.0f,1.0f };
	Angle3D angle = { 0.0f,0.0f,0.0f };

private:
	GENERATE_SERIALIZE(position, scale, angle)
};

//class FileScene;

class GameObject final
{
	friend class Window;
	friend class Scene;
	friend class FileScene;
	friend class ImGuiApp;
	friend class Component;
	friend class ObjectManager;
	friend class CameraManager;
	friend class RenderNode;
	friend class UVRenderNode;
	friend class Box2D::WorldManager;
	friend class Box2DBoxRenderNode;
	friend class Box2DCircleRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class Box2DMeshRenderNode;	
	friend class Box2DLineRenderNode;

	template<typename T>
	friend bool CreateComponent(GameObject* obj, SERIALIZE_INPUT& ar);

	using functionPointer = void (GameObject::*)();
	
	using ComponentListMap = std::unordered_map<std::string, size_t>;
	using ComponentList = std::pair <std::vector<std::unique_ptr<Component, void(*)(Component*)>>, ComponentListMap>;

private:
	//�R���X�g���N�^
	GameObject() { transform.gameobject = this; }
	//���O����
	GameObject(std::string _name);
	//�f�X�g���N�^(�R���|�[�l���g�폜)
	~GameObject();

	//�v���W�F�N�V�����s��ϊ��܂ł��ēn���i�`��ȊO�ł͊�{�g��Ȃ��j
	VSObjectConstantBuffer& GetConstantBuffer();
	//�R���|�[�l���g�̍X�V
	void UpdateComponent();
	//�Ȃɂ����Ȃ�
	void Void(){}
private:
	functionPointer pUpdate = &GameObject::UpdateComponent;
public:
	//�A�N�e�B�u��ύX����(���������������ߕp�ɂɎg�p���Ȃ�)
	void SetActive(bool _active);
	//���O�̕ύX
	void SetName(const std::string _name);
	//���O�̎擾
	const std::string GetName() const;
	//���łɃR���|�[�l���g�����Ă邩�m���߂�
	template<typename T>
	bool ExistComponent()
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			return true;
		}

		return false;
	}
	//�R���|�[�l���g�ǉ�
	template<typename T>
	SAFE_TYPE(T) AddComponent(void)
	{
		if (ExistComponent<T>()) return GetComponent<T>();

		Component* component = nullptr;
		component = new T();
		component->m_this = this;

		TRY_CATCH_LOG(component->Start());

		//���X�g�ɒǉ�(�f�X�g���N�^�o�^)
		m_componentList.first.emplace_back(
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
		m_componentList.second[typeid(T).name()] = m_componentList.first.size() - 1;

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast failed",name.c_str(), typeid(T).name());
		}

		return downcast;
	}
	//�R���|�[�l���g�ǉ�(��������)
	template<typename T,typename Arg>
	SAFE_TYPE(T) AddComponent(Arg _arg)
	{
		if (ExistComponent<T>()) return GetComponent<T>();

		Component* component = nullptr;
		component = new T(_arg);
		component->m_this = this;

		TRY_CATCH_LOG(component->Start());

		//���X�g�ɒǉ�(�f�X�g���N�^�o�^)
		m_componentList.first.emplace_back(
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
		m_componentList.second[typeid(T).name()] = m_componentList.first.size() - 1;

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast failed", name.c_str(), typeid(T).name());
		}

		return downcast;
	}

	//RenderComponent���S���ꉻ
	template<>
	SAFE_TYPE(Renderer) AddComponent<Renderer>();
	//�e�N�X�`���w��
	template<>
	SAFE_TYPE(Renderer) AddComponent<Renderer, const wchar_t*>(const wchar_t* _texPath);
	//�A�j���[�^�[�w��
	template<>
	SAFE_TYPE(Renderer) AddComponent<Renderer,Animator*>(Animator* _animator);
	//Animator���S���ꉻ
	template<>
	SAFE_TYPE(Animator) AddComponent<Animator>();
	//Box2DBodyComponent���S���ꉻ
	template<>
	SAFE_TYPE(Box2DBody) AddComponent<Box2DBody>();
	//bodyDef�w��
	template<>
	SAFE_TYPE(Box2DBody) AddComponent<Box2DBody,b2BodyDef*>(b2BodyDef* _bodyDef);
	template<>
	SAFE_TYPE(SubWindow) AddComponent<SubWindow>();
	//bodyDef�w��
	template<>
	SAFE_TYPE(SubWindow) AddComponent<SubWindow, const char*>(const char* _windowName);
	template<>
	SAFE_TYPE(SFText) AddComponent<SFText>();
	template<>
	SAFE_TYPE(SFText) AddComponent<SFText>(const char* _str);

	//�R���|�[�l���g�폜
	template<typename T>
	void RemoveComponent()
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			auto& component = m_componentList.first[iter->second];
#ifdef DEBUG_TRUE
			//�R���|�[�l���g��SafePointer��Nullptr�ɂ���
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
	void RemoveComponent<Renderer>();

	template<typename T>
	SAFE_TYPE(T) GetComponent()
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			T* downcast = dynamic_cast<T*>(m_componentList.first[iter->second].get());
			if (downcast == nullptr)
			{
				LOG_ERROR("%s : %s component down_cast failed", name.c_str(), typeid(T).name());
			}
			return downcast;
		}

		LOG_WARNING("%s : %s component not exist",name.c_str(),typeid(T).name());
		return nullptr;
	}
	//transformComponent���S���ꉻ
	template<>
	SAFE_TYPE(Transform) GetComponent(void);

	template<typename T>
	bool TryGetComponent(T** _output)
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			T* downcast = dynamic_cast<T*>(m_componentList.first[iter->second].get());
			if (downcast == nullptr)
			{
				LOG_ERROR("%s : %s component down_cast failed", name.c_str(), typeid(T).name());
				return false;
			}
			*_output = downcast;
			return true;
		}

		*_output = nullptr;
		return false;
	}

	template<>
	bool TryGetComponent(Transform** _output);
public:
	Transform transform;
private:
	std::string name = "GameObject";
	bool active = true;
	static VSObjectConstantBuffer m_cb;
	ComponentList m_componentList = {};

#ifdef DEBUG_TRUE
	enum SELECT_TYPE
	{
		SELECT_NONE,
		SELECTED,
		ON_MOUSE,
	};

	SELECT_TYPE isSelected = SELECT_NONE;
	size_t selectedNum = 0;
#endif
	
private:
	template<typename T>
	void LoadComponent(SERIALIZE_INPUT& ar)
	{
		if (ExistComponent<T>())  return;

		Component* component = nullptr;
		component = new T(this, ar);
		component->m_this = this;

		TRY_CATCH_LOG(component->Start());

		//���X�g�ɒǉ�(�f�X�g���N�^�o�^)
		m_componentList.first.emplace_back(
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
		m_componentList.second[typeid(T).name()] = m_componentList.first.size() - 1;

		return;
	}

	// Custom save function
	template <class Archive>
	void save(Archive& archive) const {
		std::vector<std::string> comNames;
		for (auto& com : m_componentList.second)
		{
			comNames.push_back(com.first);
		}
		archive(CEREAL_NVP(name), CEREAL_NVP(transform), CEREAL_NVP(active), CEREAL_NVP(comNames));
		for (auto& com : m_componentList.first)
		{
			com->Serialize(archive);
		}
	}

	// Custom load function
	template <class Archive>
	void load(Archive& archive) {
		std::vector<std::string> comNames;
		archive(CEREAL_NVP(name), CEREAL_NVP(transform), CEREAL_NVP(active), CEREAL_NVP(comNames));
		int count = 1;
		std::string uniqueName = name;
		auto& list = ObjectManager::m_currentList;

		while (list->second.find(uniqueName) != list->second.end()) {
			uniqueName = name + "_" + std::to_string(count++);
		}

#ifdef DEBUG_TRUE
		if (name != uniqueName)
		{
			LOG("%s name existed, so we changed %s.", name.c_str(), uniqueName.c_str());
		}
#endif
		name = uniqueName;
		for (auto& name : comNames)
		{
			AssemblyComponent::CreateComponent(name, this, archive);
		}
		for (auto& com : m_componentList.first)
		{
			com->Deserialize(archive);
		}
	}

	// Declare Cereal archive types as friends
	friend class cereal::access;
};

//==================================================
// �֐��|�C���^���g���ď������������X�g�ɂ���邩
// ���̂܂ܔ��������邩������킯��
//==================================================
//
class ObjectManager final
{
	friend class Window;
	friend class ImGuiApp;
	friend class Component;
	friend class Scene;
	friend class FileScene;
	friend class SceneManager;
	friend void GameObject::SetName(const std::string);
	template <class Archive>
	friend void GameObject::load(Archive& archive);

	using ObjectListMap = std::unordered_map < std::string, size_t>;

	using ObjectList = std::pair<std::vector<std::unique_ptr<GameObject, void(*)(GameObject*)>>, ObjectListMap>;

public:
	//�I�u�W�F�N�g�ꗗ���猩���� �A�N�Z�X���xn(1)�Ȃ̂ł͂₢
	static SAFE_TYPE(GameObject) Find(const std::string& _name);
private:
	//�����֎~
	ObjectManager() = delete;
	//�V�������X�g�ɂ���
	static void GenerateList();
	//�I�u�W�F�N�g��������
	static void UnInit();
	//�I�u�W�F�N�g�ɂ����R���|�[�l���g�X�V
	static void UpdateObjectComponent();
	//�I�u�W�F�N�g�̒ǉ��E���O�̏d���֎~
	static void AddObject(GameObject* _gameObject);
	//�I�u�W�F�N�g���t�@�C������ǂݍ���
	static void AddObject(std::filesystem::path& _path);
	//�I�u�W�F�N�g�̍폜
	static void DeleteObject(ObjectListMap::iterator& _iter);
	static void DeleteObject(std::string _name);
	static void DeleteObjectDelay(std::string _name);
	//�X���b�h�̌��݂̃��X�g�����̃��X�g�ɕύX
	static void ChangeNextObjectList();
	//���̃m�[�h���X�g�Ɍq��
	static void LinkNextObjectList();
	//�I�u�W�F�N�g�̖��O��ύX����
	static void ChangeObjectName(const std::string& _before,const std::string& _after);
	//�S�Ẵ��X�g�𖾎��I���Y��ɂ���
	static void CleanAllObjectList();
private:
	static thread_local void(*pDeleteObject)(std::string);
private:
	// �X���b�h���Ƃ̌��݂̃��X�g
	static thread_local ObjectList* m_currentList;
	//�I�u�W�F�N�g���i�[
	static std::unique_ptr<ObjectList> m_objectList;
	//���̃I�u�W�F�N�g���i�[
	static std::unique_ptr<ObjectList> m_nextObjectList;
	//�폜����I�u�W�F�N�g���i�[
	static std::unique_ptr<ObjectList> m_eraseObjectList;
	//�폜��x�����Ă���I�u�W�F�N�g���i�[
	static std::vector<std::string> m_delayEraseObjectName;
};


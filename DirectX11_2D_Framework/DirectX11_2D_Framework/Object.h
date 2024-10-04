#pragma once

//Scale = 1.0f �̂Ƃ��̑傫��
const float DEFULT_OBJECT_SIZE = 10.0f;

class Renderer;

struct Transform final
{
	//�������m�ۋ֎~
	void* operator new(size_t) = delete;

	GameObject* gameobject = nullptr;
	Vector3 position = { 0.0f,0.0f,0.5f };
	Vector3 scale =	{ 10.0f,10.0f ,1.0f };
	Vector3 angle = { 0.0f,0.0f,0.0f };
};

//class Collider2D : public Component;

class GameObject final
{
	friend class RenderNode;
	friend class UVRenderNode;
	friend class ObjectManager;
	friend class Scene;
	friend class Component;

private:
	//�R���X�g���N�^
	GameObject() { transform.gameobject = this; }
	//���O����
	GameObject(std::string _name);
	//�f�X�g���N�^(�R���|�[�l���g�폜)
	~GameObject();

	//�v���W�F�N�V�����s��ϊ��܂ł��ēn���i�`��ȊO�ł͊�{�g��Ȃ��j
	VSConstantBuffer& GetContantBuffer();
	//���łɃR���|�[�l���g�����Ă邩�m���߂�
	template<typename T>
	bool ExistComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			LOG_WARNING("%s : %s component is exist", name.c_str(), typeid(T).name());
			return true;
		}

		return false;
	}
	//�R���|�[�l���g�̍X�V
	void UpdateComponent();

public:
	//�A�N�e�B�u��ύX����
	void SetActive(bool _active);
	//���C���[�̐ݒ�i�p�ɂɎg�p���Ȃ��j
	void SetLayer(const LAYER _layer);
	//���O�̕ύX�i�p�ɂɎg�p���Ȃ��j
	void SetName(const std::string _name);
	//���O�̎擾
	const std::string GetName() const;
	//���C���[�擾
	const LAYER GetLayer() const;
	//�R���|�[�l���g�ǉ�
	template<typename T>
	T* AddComponent(void)
	{
		if (ExistComponent<T>()) return nullptr;

		Component* component = nullptr;
		component = new T();
		component->m_this = this;

		//���X�g�ɒǉ�(�f�X�g���N�^�o�^)
		m_componentList.insert(std::make_pair(typeid(Component).name(),
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast faild",name.c_str(), typeid(T).name());
		}

		return downcast;
	}
	//�R���|�[�l���g�ǉ�(��������)
	template<typename T,typename Arg>
	T* AddComponent(Arg arg)
	{
		if (ExistComponent<T>()) return nullptr;

		Component* component = nullptr;
		component = new T(arg);
		component->m_this = this;

		//���X�g�ɒǉ�(�f�X�g���N�^�o�^)
		m_componentList.insert(std::make_pair(typeid(Component).name(),
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast faild", name.c_str(), typeid(T).name());
		}

		return downcast;
	}

	//RenderComponent���S���ꉻ
	template<>
	Renderer* AddComponent<Renderer>();
	//�e�N�X�`���w��
	template<>
	Renderer* AddComponent<Renderer, const wchar_t*>(const wchar_t* _texPath);
	//Box2DBodyComponent���S���ꉻ
	template<>
	Box2DBody* AddComponent<Box2DBody>();
	//bodyDef�w��
	template<>
	Box2DBody* AddComponent<Box2DBody,b2BodyDef*>(b2BodyDef* _bodyDef);
	//�R���|�[�l���g�폜
	template<typename T>
	void RemoveComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			iter->second->Delete();
			m_componentList.erase(iter);
		}
	}
	template<typename T>
	T* GetComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			T* downcast = dynamic_cast<T*>(iter->second.get());
			if (downcast == nullptr)
			{
				LOG_WARNING("%s : %s component down_cast faild", name.c_str(), typeid(T).name());
			}
			return downcast;
		}

		return nullptr;
	}
	//tarnsformComponent���S���ꉻ
	template<>
	Transform* GetComponent(void);
public:
	Transform transform;
private:
	std::string name = "GameObject";
	static VSConstantBuffer m_cb;
	LAYER m_layer = LAYER::LAYER_01;
	std::unordered_map<const char*, std::unique_ptr<Component, void(*)(Component*)>> m_componentList;
};

//==================================================
// �֐��|�C���^���g���ď������������X�g�ɂ���邩
// ���̂܂ܔ��������邩������킯��
//==================================================
//
class ObjectManager final
{
	friend class Window;
	friend class Component;
	friend class Scene;
	friend class SceneManager;
	friend void GameObject::SetName(const std::string);

	using ObjectList = std::pair<std::unordered_set<std::string>, std::vector<std::unique_ptr<GameObject, void(*)(GameObject*)>>>;
	//using ObjectList = std::unordered_set<std::string, std::unique_ptr<GameObject, void(*)(GameObject*)>>;

public:
	//�I�u�W�F�N�g�ꗗ���猩����(�p�ɂɎg�p���Ȃ�)
	static GameObject* Find(std::string _name);
private:
	//�����֎~
	ObjectManager();
	//�V�������X�g�ɂ���
	static void GenerateList();
	//�I�u�W�F�N�g��������
	static void Uninit();
	//�I�u�W�F�N�g�ɂ����R���|�[�l���g�X�V
	static void UpdateObjectComponent();
	//�I�u�W�F�N�g�̒ǉ��E���O�̏d���֎~
	static GameObject* AddObject(GameObject* _gameObject);
	//�X���b�h�̌��݂̃��X�g�����̃��X�g�ɕύX
	static void ChangeNextObjectList();
	//���̃m�[�h���X�g�Ɍq��
	static void LinkNextObjectList();
	//�I�u�W�F�N�g�̖��O��ύX����
	static void ChangeObjectName(std::string _before,std::string _after);
private:
	// �X���b�h���Ƃ̌��݂̃��X�g
	static thread_local ObjectList* m_currentList;
	//�I�u�W�F�N�g���i�[
	static std::unique_ptr<ObjectList> m_objectList;
	//���̃I�u�W�F�N�g���i�[
	static std::unique_ptr<ObjectList> m_nextObjectList;
};


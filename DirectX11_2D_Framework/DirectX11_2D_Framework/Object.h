#pragma once

#define DEFULT_OBJECT_SIZE 100.0f

class Renderer;

class Object
{
public:
	//�ʒu�̍X�V
	void SetPosition(Vector2 _vec2);
	void SetPosition(XMFLOAT3 _position);
protected:
	XMFLOAT3 m_position = { 0.0f,0.0f,0.5f };
	XMFLOAT3 m_scale = { 1.0f,1.0f,1.0f };
	float m_angle = 0.0f;
};

class GameObject : public Object
{
	friend class RenderNode;

private:
	//�v���W�F�N�V�����s��ϊ��܂ł��ēn���i�`��ȊO�ł͊�{�g��Ȃ��j
	VSConstantBuffer& GetContantBuffer();
	//���łɃR���|�[�l���g�����Ă邩�m���߂�
	template<typename T>
	bool ExistComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			LOG("%s component is exist", typeid(T).name());
			return true;
		}

		return false;
	}
public:
	//�f�X�g���N�^(�R���|�[�l���g�폜)
	~GameObject();
	//���C���[�̐ݒ�i�N�ł��ς����̂ł܂���������j
	void SetLayer(LAYER _layer);
	//���C���[�擾
	const LAYER GetLayer() const;
	//�R���|�[�l���g�ǉ�
	template<typename T>
	T* AddComponent(void)
	{
		if (ExistComponent<T>()) return nullptr;

		Component* component = nullptr;
		component = new T();

		//���X�g�ɒǉ�
		m_componentList[typeid(T).name()] = std::unique_ptr<Component>(component);

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG("%s component down_cast faild", typeid(T).name());
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

		m_componentList[typeid(T).name()] = std::unique_ptr<Component>(component);

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG("%s component down_cast faild", typeid(T).name());
		}

		return downcast;
	}
	//RenderComponent�p
	template<>
	Renderer* AddComponent<Renderer>(void);
	//�e�N�X�`���w��
	template<>
	Renderer* AddComponent<Renderer, const wchar_t*>(const wchar_t* _texPath);
	//�R���|�[�l���g�폜
	template<typename T>
	void RemoveComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			iter->second->Delete();
		}

		m_componentList.erase(iter);
	}
private:
	static VSConstantBuffer m_cb;
	LAYER m_layer = LAYER::LAYER_01;
	std::unordered_map<const char*, std::unique_ptr<Component>> m_componentList;
};


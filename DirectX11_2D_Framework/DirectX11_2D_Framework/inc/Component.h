#pragma once


class Component
{
	friend class Window;
	friend class GameObject;
	friend class Box2D::WorldManager;
	friend class ImGuiApp;

protected:
	//�����֎~
	Component(){}
	//�폜�֎~
	virtual ~Component() = default;
	//�R���|�[�l���g�����Ă���I�u�W�F�N�g
	GameObject* m_this = nullptr;

	//�I�u�W�F�N�g����
	GameObject* Instantiate();
	//�I�u�W�F�N�g����(���O�w��)
	GameObject* Instantiate(std::string _name);
	//�I�u�W�F�N�g����(���O,�e�N�X�`���w��)
	GameObject* Instantiate(std::string _name, const wchar_t* _texPath);

	//Update�̒��ō폜�ł��Ȃ��悤�ɒx������
	//================================================================================
	//�I�u�W�F�N�g�̍폜(�|�C���^�w��)
	void DeleteObject(GameObject* _object);
	//�I�u�W�F�N�g�̍폜(���O�w��)
	inline void DeleteObject(std::string _name);
	//================================================================================
	// 
	// Get the type name of the derived class
	std::string getType() const {
		return demangle(typeid(*this).name());
	}
private:
	//�A�N�e�B�u��ύX
	virtual void SetActive(bool _active) {}
	//�R���|�[�l���g�폜����
	virtual void Delete() {}
	//�I�v�V�����F �f�}���O�����O�֐��ɂ��A�R���p�C���ɂ���Ă͂��������o�͂�������B
	static std::string demangle(const char* name) {
#ifdef __GNUG__
		int status = -1;
		std::unique_ptr<char, void(*)(void*)> res{
			abi::__cxa_demangle(name, nullptr, nullptr, &status),
			std::free
		};
		return (status == 0) ? res.get() : name;
#else
		return name;  // Fallback if demangling is not available
#endif
	}
private:
//============================================
// Awake�͂܂��@�\���ĂȂ�
//============================================
	//���߂ɌĂяo��
	virtual void Awake() {}
	//Awake���I����Ă���Ăяo��
	virtual void Start() {}
	//�A�N�e�B�u�ɂȂ����Ƃ��ɌĂяo��
	virtual void OnEnable() {}
	//���t���[���Ăяo��
	virtual inline void Update() {}
	//��A�N�e�B�u�ɂȂ����Ƃ��ɌĂяo��
	virtual void OnDisable() {}
	//�I�u�W�F�N�g�̃Z���T�[�ɐN�������Ƃ�
	virtual void OnColliderEnter(GameObject* _ohter) {}
	//�I�u�W�F�N�g�̃Z���T�[����o����
	virtual void OnColliderExit(GameObject* _other) {}
	//�I�u�W�F�N�g�̃Z���T�[�ɐN�������Ƃ�
	virtual void OnCollisionEnter(GameObject* _ohter) {}
	//�I�u�W�F�N�g�̃Z���T�[����o����
	virtual void OnCollisionExit(GameObject* _other) {}
	//�E�B���h�E��G������
	virtual void OnWindowEnter(HWND _target) {}
	//�E�B���h�E�𗣂�����
	virtual void OnWindowExit(HWND _target) {}
	//�E�B���h�E�𓮂������Ƃ�
	virtual void OnWindowMove(HWND _target, RECT* _rect) {}
	//�V���A���C�Y�̍ۂɌĂяo��
	virtual void Serialize(SERIALIZE_OUTPUT& ar)
	{
		//ar(CEREAL_NVP(version));
	}
	//�f�V���A���C�Y�̍ۂɌĂяo��
	virtual void Deserialize(SERIALIZE_INPUT& ar) 
	{
		//::cereal::make_optional_nvp(ar, "version", version);
	}
private:
	//============================================
	// imGui�ŕ`�悷��
	//============================================
	virtual void DrawImGui(ImGuiApp::HandleUI& _handle){
		ImGui::Text(" not override DrawImGui function!");
	}
private:
	int version = 0;
};


class AssemblyComponent final
{
	template<typename T>
	friend class ReflectionComponent;

	friend class Window;
	friend class ImGuiApp;

	class IReflection final
	{
	public:
#ifndef DEBUG_TRUE
		IReflection(std::function<bool(GameObject*, SERIALIZE_INPUT&)>&& _create)
			:createComponent(_create)
		{
		};
#endif
		~IReflection() = default;

		std::function<bool(GameObject*,SERIALIZE_INPUT&)> createComponent;

#ifdef DEBUG_TRUE
		IReflection(std::function<bool(GameObject*, SERIALIZE_INPUT&)>&& _create,std::function<void(GameObject*)>&& _add)
			:createComponent(_create),addComponent(_add)
		{};
		
		std::function<void(GameObject*)> addComponent;
#endif
	};
private:
	static inline std::map<std::string, IReflection> assemblies;

public:
	static void CreateComponent(const std::string& _comName, GameObject* _obj,SERIALIZE_INPUT& ar)
	{
		auto iter = assemblies.find(_comName);
		if (iter != assemblies.end())
		{
			iter->second.createComponent(_obj, ar);
		}
	}
};
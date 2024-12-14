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
	//�I�u�W�F�N�g�̍폜(�|�C���^�w��)
	void DeleteObject(GameObject* _object);
	//�I�u�W�F�N�g�̍폜(���O�w��)
	inline void DeleteObject(std::string _name);
	// Get the type name of the derived class
	std::string getType() const {
		return demangle(typeid(*this).name());
	}
private:
	//�A�N�e�B�u��ύX
	virtual void SetActive(bool _active) {}
	//�R���|�[�l���g�폜����
	virtual void Delete() {}
	// Optional: Demangling function for nicer output on some compilers
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
private:
	//============================================
	// imGui�ŕ`�悷��
	//============================================
	virtual void DrawImGui(ImGuiApp::HandleUI& _handle){
		ImGui::Text(" not override DrawImGui function!");
	}
};

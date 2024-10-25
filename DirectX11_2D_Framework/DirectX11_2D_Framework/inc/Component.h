#pragma once



//============================================
// Update���������ł��ĂȂ�
// Awake�Ƃ�����������
//============================================

class Component
{
	friend class GameObject;

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
private:
	//�A�N�e�B�u��ύX
	virtual void SetActive(bool _active) {}
	//���C���[��ύX
	virtual void SetLayer(const LAYER _layer) {}
	//�R���|�[�l���g�폜����
	virtual void Delete() {}
private:
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
};

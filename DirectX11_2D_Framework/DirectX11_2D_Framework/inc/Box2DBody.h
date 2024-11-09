#pragma once

class Box2DBody : public Component
{
	friend class GameObject;

private:
	Box2DBody(GameObject* _object);
	Box2DBody(GameObject* _object, b2BodyDef* _bodyDef);
	//Box2dWorld����ʒu�Ɗp�x���󂯎��
	inline void Update() override;
	//�ォ������
	void Delete() override;
	//�A�N�e�B�u�ݒ�
	void SetActive(bool _active) override;
	//imgui�̕`��
	void DrawImGui()override;
public:
	//bodyId�̎擾
	const b2BodyId GetBodyId() { return m_bodyId; }
	//�t�B���^�[�̕ύX
	void SetFilter(const FILTER _layer);
	//�t�B���^�[�̎擾
	const FILTER GetFilter() const
	{
		return m_filter;
	}

	//=================================
	// �q�b�g�{�b�N�X����
	//=================================
	//�����蔻��̍쐬(Box)
	void CreateBoxShape(bool _sensor = false);
	//Box�I�t�Z�b�g�w��
	void CreateBoxShape(float _offsetX, float _offsetY, float _angle = 0.0f, bool _sensor = false);
	//Box�T�C�Y�w��
	void CreateBoxShape(Vector2 _size, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f, bool _sensor = false);
	//�����蔻��̍쐬
	void CreateCircleShape();
	//Circle�I�t�Z�b�g�w��
	void CreateCircleShape(Vector2 _offset);
	//Circle�T�C�Y�w��
	void CreateCircleShape(float _diameter,Vector2 _offset = { 0.0f,0.0f });
	//�����蔻��̍쐬
	void CreateCapsuleShape();
	//Capsule�I�t�Z�b�g�w��
	void CreateCapsuleShape(float _height, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f);
	//Capsule�쐬
	void CreateCapsuleShape(float _diameter, float _height, float _angle = 0.0f, Vector2 _offset = { 0.0f,0.0f });
	//�����蔻��̍쐬
	void CreatePolygonShape(std::vector<b2Vec2> _pointList);
	//�����蔻��̍쐬 
	void CreateSegment(std::vector<b2Vec2> _pointList);
	//�����蔻��̍쐬 (�����_���l�ȏ�w��)
	void CreateChain(std::vector<b2Vec2>& _pointList);

	//=================================
	//body�̐���
	//=================================
	//position�̃e���|�[�g(������)
	void SetPosition(Vector2 _pos);
	//angle�̋����ύX(������)
	void SetAngle(float _deg);
	void SetAngle(double _rad);
	void SetAngle(Angle _angle);
	//�͂̌����̐ݒ�
	void SetVelocity(b2Vec2 _velocity);
	//�͂̌����̐ݒ�(x����)
	void SetVelocityX(float _velocityX);
	//�͂̌����̐ݒ�(y����)
	void SetVelocityY(float _velocityY);
	//�͂̌������擾
	const b2Vec2 GetVelocity() const;
	//�͂����킶�������
	void AddForce(b2Vec2 _force);
	//�͂��ς��Ɖ�����
	void AddForceImpule(b2Vec2 _force);
	//�d�͂̑傫���擾
	float GetGravityScale() const;
	//�d�͂�傫���ύX
	void SetGravityScale(float _scale);
	//��]���~�߂�ݒ�
	void SetFixedRotation(bool _flag);
	//�����̒�~��ύX����
	void SetAwake(bool _awake);

	//���ݏd�Ȃ��Ă���I�u�W�F�N�g�̑F��
	void GetOverlapObject(std::vector<GameObject*>& _objects);
	//���ݏd�Ȃ��Ă���I�u�W�F�N�g�̑F��(Transform�w��)
	void GetOverlapObject(std::vector<GameObject*>& _objects,b2Transform _tf);
	//���ݏd�Ȃ��Ă���I�u�W�F�N�g�̑F��
	void GetOverlapObject(std::unordered_map<GameObject*,b2ShapeId>& _objects);
	//���ݏd�Ȃ��Ă���I�u�W�F�N�g�̑F��(Transform�w��)
	void GetOverlapObject(std::unordered_map<GameObject*, b2ShapeId>& _objects, b2Transform _tf);
	//���ݏd�Ȃ��Ă���I�u�W�F�N�g�̑F��(Transform,filter�w��)
	void GetOverlapObject(std::unordered_map<GameObject*, b2ShapeId>& _objects, b2Transform _tf, FILTER _filter);
private:
	b2BodyId m_bodyId;
	//Shape���i�[����
	std::vector<b2ShapeId> m_shapeList;
	//�����蔻����킯��
	FILTER m_filter = FILTER::FILTER_01;
#ifdef DEBUG_TRUE
	std::vector<std::shared_ptr<RenderNode>> m_nodeList;
#endif
};

class Box2DBodyManager
{
	friend class Window;
	friend class Component;
	friend class Box2DBody;
	friend class SceneManager;
	friend class Box2D::WorldManager;
	friend class Box2DBoxRenderNode;
	friend class Box2DCircleRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class ImGuiApp;

public:
	//�w�肵���t�B���^�[���Փ˂��Ȃ��ݒ�ɂ���
	static void DisableCollisionFilter(FILTER _filter01, FILTER _filter02);
	//�w�肵���t�B���^�[���Փ˂���ݒ�ɂ���
	static void EnableCollisionFilter(FILTER _filter01, FILTER _filter02);
	//�t�B���^�[�P�̓t�B���^�[�Q�ɂ����Փ˂��Ȃ��ݒ�ɂ���
	static void OnlyCollisionFilter(FILTER _filter, FILTER _target);
private:
#ifdef DEBUG_TRUE
	static void Init();
#endif
	//�Ή������I�u�W�F�N�g�𓮂���
	static void ExcuteMoveFunction();
	//�w�肵��Layer��maskBit�擾
	static unsigned int GetMaskFilterBit(FILTER _filter);
private:
	//body�̈ʒu�ύX�֐����X�g
	static std::unordered_map<std::string, std::function<void()>> moveFunctions;
	//layer�̃t�B���^�[�̃r�b�g���i�[
	static std::unordered_map<FILTER, unsigned int> m_layerFilterBit;
	//bodyId�ɑΉ������I�u�W�F�N�g�̖��O���i�[
	static std::unordered_map<int32_t, std::string> m_bodyObjectName;

#ifdef DEBUG_TRUE
	// More segments = smoother circle
	static const int numSegments;
	//box�p�C���f�b�N�X
	static ComPtr<ID3D11Buffer> m_boxIndexBuffer;
	//Circle�p���_�f�[�^
	static ComPtr<ID3D11Buffer> m_circleVertexBuffer;
	//Circle�p�C���f�b�N�X
	static ComPtr<ID3D11Buffer> m_circleIndexBuffer;
#endif
};

#ifdef DEBUG_TRUE

//�����蔻��̕`��p�m�[�h�N���X(Debug)
class Box2DBoxRenderNode : public RenderNode
{
	friend class Box2DBody;

	Box2DBoxRenderNode(Vector2 _offset, Vector2 _size, float _angle, b2BodyId _bodyId) :
		m_offset(_offset), m_size(_size), m_angle(_angle), m_bodyId(_bodyId), RenderNode() {}
private:
	inline void Draw();
private:
	Vector2 m_offset;
	Vector2 m_size;
	float m_angle;
	b2BodyId m_bodyId;
};

class Box2DCircleRenderNode : public RenderNode
{
	friend class Box2DBody;

	Box2DCircleRenderNode(Vector2 _offset, float _size, b2BodyId _bodyId) :
		m_offset(_offset), m_size(_size), m_bodyId(_bodyId), RenderNode() {}
private:
	inline void Draw();
private:
	Vector2 m_offset;
	float m_size;
	b2BodyId m_bodyId;
};

class Box2DCapsuleRenderNode : public RenderNode
{
	friend class Box2DBody;

	Box2DCapsuleRenderNode(Vector2 _offset, float _diameter, float _sizeY, float _height, float _angle, b2BodyId _bodyId) :
		m_offset(_offset), m_diameter(_diameter), m_sizeY(_sizeY), m_height(_height), m_angle(_angle), m_bodyId(_bodyId) {}
private:
	inline void Draw();
private:
	Vector2 m_offset;
	float m_diameter;
	float m_sizeY;
	float m_height;
	float m_angle;
	b2BodyId m_bodyId;
};

class Box2DMeshRenderNode : public RenderNode
{
	friend class Box2DBody;

protected:
	Box2DMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId, bool _loop);
	Box2DMeshRenderNode(b2BodyId _bodyId):m_bodyId(_bodyId){}
private:
	inline void Draw();
protected:
	//�C���f�b�N�X�̐�
	int indexCount = 0;
	//�Ώۂ�body
	b2BodyId m_bodyId;
	//Circle�p���_�f�[�^
	ComPtr<ID3D11Buffer> m_chainVertexBuffer;
	//Circle�p�C���f�b�N�X
	ComPtr<ID3D11Buffer> m_chainIndexBuffer;
};

class Box2DConvexMeshRenderNode : public Box2DMeshRenderNode
{
	friend class Box2DBody;

	Box2DConvexMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId/*, bool _loop*/);
};

#endif


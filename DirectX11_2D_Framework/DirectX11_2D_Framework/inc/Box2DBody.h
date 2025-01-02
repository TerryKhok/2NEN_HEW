#pragma once

//#define RELEASE_SERIALIZE_VIEW_HITBOX

struct Box2DRenderData;

class Box2DBody : public Component
{
	friend class GameObject;
	friend class RenderNode;
	friend class TileRenderNode;
	friend class ObjectManager;
	friend class Box2D::WorldManager;

	struct BodySaveData
	{
		b2BodyType type;
		b2Vec2 lineVec;
		float angleVec;
		float gravity;
		float mass;
		bool bullet;
		bool fixRot;
		bool awake;

	private:
		template<class Archive>
		void serialize(Archive& ar)
		{
			ar(CEREAL_NVP(type), CEREAL_NVP(lineVec), CEREAL_NVP(angleVec), CEREAL_NVP(gravity),
				CEREAL_NVP(mass), CEREAL_NVP(bullet), CEREAL_NVP(fixRot),CEREAL_NVP(awake));
		}

		friend class cereal::access;
	};

	struct ShapeSaveData
	{
		bool sensor;
		float friction;
		float density;
		float restitution;

	private:
		template<class Archive>
		void serialize(Archive& ar)
		{
			ar(CEREAL_NVP(sensor), CEREAL_NVP(friction), CEREAL_NVP(density), CEREAL_NVP(restitution));
		}

		friend class cereal::access;
	};

	enum Box2DShapeType
	{
		BOX,
		CIRCLE,
		CAPSULE,
		SEGMENT,
		POLYGON,
		CHAIN,
		TYPE_MAX
	};
private:
	Box2DBody(GameObject* _object);
	Box2DBody(GameObject* _object, b2BodyDef* _bodyDef);
	//�f�V���A���C�Y�p
	Box2DBody(GameObject* _object, SERIALIZE_INPUT& ar);
	~Box2DBody() = default;
	//Box2dWorld����ʒu�Ɗp�x���󂯎��
	inline void Update() override;
	//�ォ������
	void Delete() override;
	//�A�N�e�B�u�ݒ�
	void SetActive(bool _active) override;
	//�V���A���C�Y
	void Serialize(SERIALIZE_OUTPUT& ar) override;
	//imGui�̕`��
	void DrawImGui(ImGuiApp::HandleUI& _handle)override;
public:
	//bodyId�̎擾
	const b2BodyId GetBodyId() { return m_bodyId; }
	// �t�B���^�[�̕ύX
	void SetFilter(const FILTER _layer);
	//�t�B���^�[�̎擾
	const FILTER GetFilter() const
	{
		return m_filter;
	}
	const int GetShapeCount() const
	{
		return (int)m_shapeList.size();
	}

	//=================================
	// �q�b�g�{�b�N�X����
	//=================================
	//�����蔻��̍쐬(Box)
	void CreateBoxShape(bool _sensor = false);
	//Box�I�t�Z�b�g�w��
	void CreateBoxShape(Vector2 _offset, float _angle = 0.0f, bool _sensor = false);
	//Box�T�C�Y�w��
	void CreateBoxShape(Vector2 _size, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f, bool _sensor = false);
	//�����蔻��̍쐬
	void CreateCircleShape(bool _sensor = false);
	//Circle�I�t�Z�b�g�w��
	void CreateCircleShape(Vector2 _offset, bool _sensor = false);
	//Circle�T�C�Y�w��
	void CreateCircleShape(float _diameter,Vector2 _offset = { 0.0f,0.0f }, bool _sensor = false);
	//�����蔻��̍쐬
	void CreateCapsuleShape(bool _sensor = false);
	//�I�t�Z�b�g�ƃA���O���w��
	void CreateCapsuleShape(Vector2 _offset, float _angle = 0.0f, bool _sensor = false);
	//Capsule�I�t�Z�b�g�w��
	void CreateCapsuleShape(float _height, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f, bool _sensor = false);
	//Capsule�쐬
	void CreateCapsuleShape(float _diameter, float _height, float _angle = 0.0f, Vector2 _offset = { 0.0f,0.0f }, bool _sensor = false);
	//�����蔻��̍쐬
	void CreatePolygonShape(std::vector<b2Vec2> _pointList, bool _sensor = false);
	//�����蔻��̍쐬 
	void CreateSegment(std::vector<b2Vec2> _pointList, bool _sensor = false);
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
	void AddForceImpulse(b2Vec2 _force);
	//body�̎�ނ�ύX����
	void SetType(b2BodyType _type);
	//body�̎�ނ��擾����
	b2BodyType GetType();
	//�d�͂̑傫���擾
	float GetGravityScale() const;
	//�d�͂�傫���ύX
	void SetGravityScale(float _scale);
	//�d�����擾
	float GetMass();
	//�d����ύX
	void SetMass(float _mass);
	//�����I�u�W�F�N�g�ǂ�����ݒ肷��
	void SetBullet(bool _isBullet);
	//�����I�u�W�F�N�g�����擾����
	bool IsBullet();
	//�e�݂̐ݒ�
	void SetRestitution(float _restitution);
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
#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	//�����蔻���`�悷�邽�߂̃f�[�^
	std::vector<std::unique_ptr<Box2DRenderData>> renderData;
#endif
#ifdef DEBUG_TRUE
	std::vector<std::shared_ptr<RenderNode>> m_nodeList;
#endif
};

//class Box2DBodyChain : public Component
//{
//	friend class GameObject;
//	friend class RenderNode;
//	friend class ObjectManager;
//	friend class Box2D::WorldManager;
//
//	struct BodySaveData
//	{
//		b2BodyType type;
//		b2Vec2 lineVec;
//		float angleVec;
//		float gravity;
//		float mass;
//		bool bullet;
//		bool fixRot;
//		bool awake;
//
//	private:
//		template<class Archive>
//		void serialize(Archive& ar)
//		{
//			ar(CEREAL_NVP(type), CEREAL_NVP(lineVec), CEREAL_NVP(angleVec), CEREAL_NVP(gravity),
//				CEREAL_NVP(mass), CEREAL_NVP(bullet), CEREAL_NVP(fixRot), CEREAL_NVP(awake));
//		}
//
//		friend class cereal::access;
//	};
//
//	struct ChainSaveData
//	{
//		bool sensor;
//		float friction;
//		float density;
//		float restitution;
//
//	private:
//		template<class Archive>
//		void serialize(Archive& ar)
//		{
//			ar(CEREAL_NVP(sensor), CEREAL_NVP(friction), CEREAL_NVP(density), CEREAL_NVP(restitution));
//		}
//
//		friend class cereal::access;
//	};
//
//private:
//	Box2DBodyChain(GameObject* _object);
//	Box2DBodyChain(GameObject* _object, b2BodyDef* _bodyDef);
//	//�f�V���A���C�Y�p
//	Box2DBodyChain(GameObject* _object, SERIALIZE_INPUT& ar);
//	~Box2DBodyChain() = default;
//	//Box2dWorld����ʒu�Ɗp�x���󂯎��
//	inline void Update() override;
//	//�ォ������
//	void Delete() override;
//	//�A�N�e�B�u�ݒ�
//	void SetActive(bool _active) override;
//	//�V���A���C�Y
//	void Serialize(SERIALIZE_OUTPUT& ar) override;
//	//imGui�̕`��
//	void DrawImGui(ImGuiApp::HandleUI& _handle)override;
//public:
//	//bodyId�̎擾
//	const b2BodyId GetBodyId() { return m_bodyId; }
//
//	//=================================
//	// �q�b�g�{�b�N�X����
//	//=================================
//	//�����蔻��̍쐬 (�����_���l�ȏ�w��)
//	void CreateChain(std::vector<b2Vec2>& _pointList,FILTER _filter);
//
//	//=================================
//	//body�̐���
//	//=================================
//	//position�̃e���|�[�g(������)
//	void SetPosition(Vector2 _pos);
//	//angle�̋����ύX(������)
//	void SetAngle(float _deg);
//	void SetAngle(double _rad);
//	void SetAngle(Angle _angle);
//	//�͂̌����̐ݒ�
//	void SetVelocity(b2Vec2 _velocity);
//	//�͂̌����̐ݒ�(x����)
//	void SetVelocityX(float _velocityX);
//	//�͂̌����̐ݒ�(y����)
//	void SetVelocityY(float _velocityY);
//	//�͂̌������擾
//	const b2Vec2 GetVelocity() const;
//	//�͂����킶�������
//	void AddForce(b2Vec2 _force);
//	//�͂��ς��Ɖ�����
//	void AddForceImpulse(b2Vec2 _force);
//	//body�̎�ނ�ύX����
//	void SetType(b2BodyType _type);
//	//body�̎�ނ��擾����
//	b2BodyType GetType();
//	//�d�͂̑傫���擾
//	float GetGravityScale() const;
//	//�d�͂�傫���ύX
//	void SetGravityScale(float _scale);
//	//�d�����擾
//	float GetMass();
//	//�d����ύX
//	void SetMass(float _mass);
//	//�����I�u�W�F�N�g�ǂ�����ݒ肷��
//	void SetBullet(bool _isBullet);
//	//�����I�u�W�F�N�g�����擾����
//	bool IsBullet();
//	//�e�݂̐ݒ�
//	void SetRestitution(float _restitution);
//	//��]���~�߂�ݒ�
//	void SetFixedRotation(bool _flag);
//	//�����̒�~��ύX����
//	void SetAwake(bool _awake);
//private:
//	b2BodyId m_bodyId;
//	//Shape���i�[����
//	std::vector<b2ChainId> m_chainList;
//	std::vector<std::vector<b2Vec2>> m_chainVertexList;
//#ifdef DEBUG_TRUE
//	std::vector<std::shared_ptr<RenderNode>> m_nodeList;
//#endif
//};

class Box2DBodyManager
{
	friend class Window;
	friend class ObjectManager;
	friend class Box2DBody;
	friend class Box2DBodyChain;
	friend class SceneManager;
	friend class Box2D::WorldManager;
	friend class Box2DBoxRenderNode;
	friend class Box2DCircleRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class ImGuiApp;
	friend class RenderNode;

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
	//�w�肵��Layer��MaskBit�擾
	static unsigned int GetMaskFilterBit(FILTER _filter);
private:
	//layer�̃t�B���^�[�̃r�b�g���i�[
	static std::unordered_map<FILTER, unsigned int> m_layerFilterBit;
	//bodyId�ɑΉ������I�u�W�F�N�g�̖��O���i�[
	static std::unordered_map<int32_t, const std::string&> m_bodyObjectName;

#ifdef DEBUG_TRUE
	// More segments = smoother circle
	static const int numSegments;
	//Circle�p���_�f�[�^
	static ComPtr<ID3D11Buffer> m_circleVertexBuffer;
	//Circle�p�C���f�b�N�X
	static ComPtr<ID3D11Buffer> m_circleIndexBuffer;
#endif
};


#ifdef RELEASE_SERIALIZE_VIEW_HITBOX

struct Box2DRenderData
{
	Box2DRenderData() = default;
	virtual ~Box2DRenderData() = default;

	virtual RenderNode* CreateRenderNode(b2BodyId bodyId) = 0;

	template<class Archive>
	void serialize(Archive& ar){}
};


struct Box2DBoxData : public Box2DRenderData
{
	Vector2 m_offset;
	Vector2 m_size;
	float m_angle;

	Box2DBoxData():
		m_offset({0.0f,0.0f}), m_size({HALF_OBJECT_SIZE,HALF_OBJECT_SIZE }), m_angle(0.0f) { }
	Box2DBoxData(Vector2 offset, Vector2 size, float angle) :
		m_offset(offset), m_size(size), m_angle(angle) {}

	RenderNode* CreateRenderNode(b2BodyId bodyId);

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(m_offset), CEREAL_NVP(m_size), CEREAL_NVP(m_angle));
	}
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(Box2DBoxData)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Box2DRenderData, Box2DBoxData)


struct Box2DCircleData : public Box2DRenderData
{
	Vector2 m_offset;
	float m_size;

	Box2DCircleData() :
		m_offset({0.0f,0.0f}), m_size(HALF_OBJECT_SIZE) { }
	Box2DCircleData(Vector2 offset,float size):
		m_offset(offset),m_size(size){}

	RenderNode* CreateRenderNode(b2BodyId bodyId);

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(m_offset), CEREAL_NVP(m_size));
	}
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(Box2DCircleData)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Box2DRenderData, Box2DCircleData)

struct Box2DCapsuleData : public Box2DRenderData
{
	Vector2 m_offset;
	float m_diameter;
	float m_sizeY;
	float m_height;
	float m_angle;

	Box2DCapsuleData() :
		m_offset({0.0f,0.0f}), m_diameter(HALF_OBJECT_SIZE), m_sizeY(HALF_OBJECT_SIZE), m_height(HALF_OBJECT_SIZE), m_angle(0.0f) { }
	Box2DCapsuleData(Vector2 offset, float diameter, float sizeY, float height, float angle) :
		m_offset(offset), m_diameter(diameter), m_sizeY(sizeY), m_height(height), m_angle(angle) {}

	RenderNode* CreateRenderNode(b2BodyId bodyId);

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(m_offset), CEREAL_NVP(m_diameter), CEREAL_NVP(m_sizeY), CEREAL_NVP(m_height), CEREAL_NVP(m_angle));
	}
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(Box2DCapsuleData)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Box2DRenderData, Box2DCapsuleData)

struct Box2DMeshData : public Box2DRenderData
{
	std::vector<b2Vec2> m_pointList;
	bool m_loop;

	Box2DMeshData():
		m_loop(false){}
	Box2DMeshData(std::vector<b2Vec2> pointList,bool loop):
		m_pointList(pointList),m_loop(loop){}

	RenderNode* CreateRenderNode(b2BodyId bodyId);

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(m_pointList), CEREAL_NVP(m_loop));
	}
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(Box2DMeshData)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Box2DRenderData, Box2DMeshData)

struct Box2DLineData : public Box2DRenderData
{
	float m_length;
	float m_radian;
	Vector2 m_center;

	Box2DLineData() :
		m_length(0.0f), m_radian(0.0f), m_center({ 0.0f,0.0f }) {}
	Box2DLineData(float length,float radian,Vector2 center) :
		m_length(length), m_radian(radian), m_center(center){}

	RenderNode* CreateRenderNode(b2BodyId bodyId);

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(m_length), CEREAL_NVP(m_radian),CEREAL_NVP(m_center));
	}
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(Box2DLineData)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Box2DRenderData, Box2DLineData)

struct Box2DConvexMeshData : public Box2DRenderData
{
	std::vector<b2Vec2> m_pointList;

	Box2DConvexMeshData() {}
	Box2DConvexMeshData(std::vector<b2Vec2> pointList):
		m_pointList(pointList){}

	RenderNode* CreateRenderNode(b2BodyId bodyId);

	template<class Archive>
	void serialize(Archive& ar)
	{
		ar(CEREAL_NVP(m_pointList));
	}
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(Box2DConvexMeshData)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Box2DRenderData, Box2DConvexMeshData)

#endif


#ifdef DEBUG_TRUE

//�����蔻��̕`��p�m�[�h�N���X(Debug)
class Box2DBoxRenderNode : public RenderNode
{
	friend class Box2DBody;
	friend struct Box2DBoxData;

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
	friend struct Box2DCircleData;

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
	friend struct Box2DCapsuleData;

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
	friend class Box2DBodyChain;
	friend struct Box2DMeshData;

protected:
	Box2DMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId, bool _loop);
	Box2DMeshRenderNode(b2BodyId _bodyId):m_bodyId(_bodyId){}
private:
	inline void Draw();
protected:
	//�C���f�b�N�X�̐�
	int indexCount = 0;
	//�Ώۂ�Body
	b2BodyId m_bodyId;
	//Circle�p���_�f�[�^
	ComPtr<ID3D11Buffer> m_chainVertexBuffer;
	//Circle�p�C���f�b�N�X
	ComPtr<ID3D11Buffer> m_chainIndexBuffer;
};

class Box2DConvexMeshRenderNode : public Box2DMeshRenderNode
{
	friend class Box2DBody;
	friend struct Box2DConvexMeshData;

	Box2DConvexMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId);
};

class Box2DLineRenderNode : public RenderNode
{
	friend class Box2DBody;
	friend struct Box2DLineData;

	Box2DLineRenderNode(float length,float radian,Vector2 center,b2BodyId bodyId):
		m_length(length),m_radian(radian),m_center(center),m_bodyId(bodyId){}
private:
	inline void Draw() override;
private:
	float m_length;
	float m_radian;
	Vector2 m_center;
	b2BodyId m_bodyId;
};

#endif


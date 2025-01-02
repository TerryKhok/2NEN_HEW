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
	//デシリアライズ用
	Box2DBody(GameObject* _object, SERIALIZE_INPUT& ar);
	~Box2DBody() = default;
	//Box2dWorldから位置と角度を受け取る
	inline void Update() override;
	//後かたずけ
	void Delete() override;
	//アクティブ設定
	void SetActive(bool _active) override;
	//シリアライズ
	void Serialize(SERIALIZE_OUTPUT& ar) override;
	//imGuiの描画
	void DrawImGui(ImGuiApp::HandleUI& _handle)override;
public:
	//bodyIdの取得
	const b2BodyId GetBodyId() { return m_bodyId; }
	// フィルターの変更
	void SetFilter(const FILTER _layer);
	//フィルターの取得
	const FILTER GetFilter() const
	{
		return m_filter;
	}
	const int GetShapeCount() const
	{
		return (int)m_shapeList.size();
	}

	//=================================
	// ヒットボックス生成
	//=================================
	//当たり判定の作成(Box)
	void CreateBoxShape(bool _sensor = false);
	//Boxオフセット指定
	void CreateBoxShape(Vector2 _offset, float _angle = 0.0f, bool _sensor = false);
	//Boxサイズ指定
	void CreateBoxShape(Vector2 _size, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f, bool _sensor = false);
	//当たり判定の作成
	void CreateCircleShape(bool _sensor = false);
	//Circleオフセット指定
	void CreateCircleShape(Vector2 _offset, bool _sensor = false);
	//Circleサイズ指定
	void CreateCircleShape(float _diameter,Vector2 _offset = { 0.0f,0.0f }, bool _sensor = false);
	//当たり判定の作成
	void CreateCapsuleShape(bool _sensor = false);
	//オフセットとアングル指定
	void CreateCapsuleShape(Vector2 _offset, float _angle = 0.0f, bool _sensor = false);
	//Capsuleオフセット指定
	void CreateCapsuleShape(float _height, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f, bool _sensor = false);
	//Capsule作成
	void CreateCapsuleShape(float _diameter, float _height, float _angle = 0.0f, Vector2 _offset = { 0.0f,0.0f }, bool _sensor = false);
	//当たり判定の作成
	void CreatePolygonShape(std::vector<b2Vec2> _pointList, bool _sensor = false);
	//当たり判定の作成 
	void CreateSegment(std::vector<b2Vec2> _pointList, bool _sensor = false);
	//当たり判定の作成 (※頂点を四つ以上指定)
	void CreateChain(std::vector<b2Vec2>& _pointList);

	//=================================
	//bodyの制御
	//=================================
	//positionのテレポート(高処理)
	void SetPosition(Vector2 _pos);
	//angleの強制変更(高処理)
	void SetAngle(float _deg);
	void SetAngle(double _rad);
	void SetAngle(Angle _angle);
	//力の向きの設定
	void SetVelocity(b2Vec2 _velocity);
	//力の向きの設定(xだけ)
	void SetVelocityX(float _velocityX);
	//力の向きの設定(yだけ)
	void SetVelocityY(float _velocityY);
	//力の向きを取得
	const b2Vec2 GetVelocity() const;
	//力をじわじわ加える
	void AddForce(b2Vec2 _force);
	//力をぱっと加える
	void AddForceImpulse(b2Vec2 _force);
	//bodyの種類を変更する
	void SetType(b2BodyType _type);
	//bodyの種類を取得する
	b2BodyType GetType();
	//重力の大きさ取得
	float GetGravityScale() const;
	//重力を大きさ変更
	void SetGravityScale(float _scale);
	//重さを取得
	float GetMass();
	//重さを変更
	void SetMass(float _mass);
	//速いオブジェクトどうかを設定する
	void SetBullet(bool _isBullet);
	//速いオブジェクトかを取得する
	bool IsBullet();
	//弾みの設定
	void SetRestitution(float _restitution);
	//回転を止める設定
	void SetFixedRotation(bool _flag);
	//処理の停止を変更する
	void SetAwake(bool _awake);

	//現在重なっているオブジェクトの詮索
	void GetOverlapObject(std::vector<GameObject*>& _objects);
	//現在重なっているオブジェクトの詮索(Transform指定)
	void GetOverlapObject(std::vector<GameObject*>& _objects,b2Transform _tf);
	//現在重なっているオブジェクトの詮索
	void GetOverlapObject(std::unordered_map<GameObject*,b2ShapeId>& _objects);
	//現在重なっているオブジェクトの詮索(Transform指定)
	void GetOverlapObject(std::unordered_map<GameObject*, b2ShapeId>& _objects, b2Transform _tf);
	//現在重なっているオブジェクトの詮索(Transform,filter指定)
	void GetOverlapObject(std::unordered_map<GameObject*, b2ShapeId>& _objects, b2Transform _tf, FILTER _filter);
private:
	b2BodyId m_bodyId;
	//Shapeを格納する
	std::vector<b2ShapeId> m_shapeList;
	//当たり判定をわける
	FILTER m_filter = FILTER::FILTER_01;
#ifdef RELEASE_SERIALIZE_VIEW_HITBOX
	//当たり判定を描画するためのデータ
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
//	//デシリアライズ用
//	Box2DBodyChain(GameObject* _object, SERIALIZE_INPUT& ar);
//	~Box2DBodyChain() = default;
//	//Box2dWorldから位置と角度を受け取る
//	inline void Update() override;
//	//後かたずけ
//	void Delete() override;
//	//アクティブ設定
//	void SetActive(bool _active) override;
//	//シリアライズ
//	void Serialize(SERIALIZE_OUTPUT& ar) override;
//	//imGuiの描画
//	void DrawImGui(ImGuiApp::HandleUI& _handle)override;
//public:
//	//bodyIdの取得
//	const b2BodyId GetBodyId() { return m_bodyId; }
//
//	//=================================
//	// ヒットボックス生成
//	//=================================
//	//当たり判定の作成 (※頂点を四つ以上指定)
//	void CreateChain(std::vector<b2Vec2>& _pointList,FILTER _filter);
//
//	//=================================
//	//bodyの制御
//	//=================================
//	//positionのテレポート(高処理)
//	void SetPosition(Vector2 _pos);
//	//angleの強制変更(高処理)
//	void SetAngle(float _deg);
//	void SetAngle(double _rad);
//	void SetAngle(Angle _angle);
//	//力の向きの設定
//	void SetVelocity(b2Vec2 _velocity);
//	//力の向きの設定(xだけ)
//	void SetVelocityX(float _velocityX);
//	//力の向きの設定(yだけ)
//	void SetVelocityY(float _velocityY);
//	//力の向きを取得
//	const b2Vec2 GetVelocity() const;
//	//力をじわじわ加える
//	void AddForce(b2Vec2 _force);
//	//力をぱっと加える
//	void AddForceImpulse(b2Vec2 _force);
//	//bodyの種類を変更する
//	void SetType(b2BodyType _type);
//	//bodyの種類を取得する
//	b2BodyType GetType();
//	//重力の大きさ取得
//	float GetGravityScale() const;
//	//重力を大きさ変更
//	void SetGravityScale(float _scale);
//	//重さを取得
//	float GetMass();
//	//重さを変更
//	void SetMass(float _mass);
//	//速いオブジェクトどうかを設定する
//	void SetBullet(bool _isBullet);
//	//速いオブジェクトかを取得する
//	bool IsBullet();
//	//弾みの設定
//	void SetRestitution(float _restitution);
//	//回転を止める設定
//	void SetFixedRotation(bool _flag);
//	//処理の停止を変更する
//	void SetAwake(bool _awake);
//private:
//	b2BodyId m_bodyId;
//	//Shapeを格納する
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
	//指定したフィルターが衝突しない設定にする
	static void DisableCollisionFilter(FILTER _filter01, FILTER _filter02);
	//指定したフィルターが衝突する設定にする
	static void EnableCollisionFilter(FILTER _filter01, FILTER _filter02);
	//フィルター１はフィルター２にしか衝突しない設定にする
	static void OnlyCollisionFilter(FILTER _filter, FILTER _target);
private:
#ifdef DEBUG_TRUE
	static void Init();
#endif
	//指定したLayerのMaskBit取得
	static unsigned int GetMaskFilterBit(FILTER _filter);
private:
	//layerのフィルターのビットを格納
	static std::unordered_map<FILTER, unsigned int> m_layerFilterBit;
	//bodyIdに対応したオブジェクトの名前を格納
	static std::unordered_map<int32_t, const std::string&> m_bodyObjectName;

#ifdef DEBUG_TRUE
	// More segments = smoother circle
	static const int numSegments;
	//Circle用頂点データ
	static ComPtr<ID3D11Buffer> m_circleVertexBuffer;
	//Circle用インデックス
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

//当たり判定の描画用ノードクラス(Debug)
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
	//インデックスの数
	int indexCount = 0;
	//対象のBody
	b2BodyId m_bodyId;
	//Circle用頂点データ
	ComPtr<ID3D11Buffer> m_chainVertexBuffer;
	//Circle用インデックス
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


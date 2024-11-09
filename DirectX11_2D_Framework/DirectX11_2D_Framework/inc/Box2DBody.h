#pragma once

class Box2DBody : public Component
{
	friend class GameObject;

private:
	Box2DBody(GameObject* _object);
	Box2DBody(GameObject* _object, b2BodyDef* _bodyDef);
	//Box2dWorldから位置と角度を受け取る
	inline void Update() override;
	//後かたずけ
	void Delete() override;
	//アクティブ設定
	void SetActive(bool _active) override;
	//imguiの描画
	void DrawImGui()override;
public:
	//bodyIdの取得
	const b2BodyId GetBodyId() { return m_bodyId; }
	//フィルターの変更
	void SetFilter(const FILTER _layer);
	//フィルターの取得
	const FILTER GetFilter() const
	{
		return m_filter;
	}

	//=================================
	// ヒットボックス生成
	//=================================
	//当たり判定の作成(Box)
	void CreateBoxShape(bool _sensor = false);
	//Boxオフセット指定
	void CreateBoxShape(float _offsetX, float _offsetY, float _angle = 0.0f, bool _sensor = false);
	//Boxサイズ指定
	void CreateBoxShape(Vector2 _size, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f, bool _sensor = false);
	//当たり判定の作成
	void CreateCircleShape();
	//Circleオフセット指定
	void CreateCircleShape(Vector2 _offset);
	//Circleサイズ指定
	void CreateCircleShape(float _diameter,Vector2 _offset = { 0.0f,0.0f });
	//当たり判定の作成
	void CreateCapsuleShape();
	//Capsuleオフセット指定
	void CreateCapsuleShape(float _height, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f);
	//Capsule作成
	void CreateCapsuleShape(float _diameter, float _height, float _angle = 0.0f, Vector2 _offset = { 0.0f,0.0f });
	//当たり判定の作成
	void CreatePolygonShape(std::vector<b2Vec2> _pointList);
	//当たり判定の作成 
	void CreateSegment(std::vector<b2Vec2> _pointList);
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
	void AddForceImpule(b2Vec2 _force);
	//重力の大きさ取得
	float GetGravityScale() const;
	//重力を大きさ変更
	void SetGravityScale(float _scale);
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
	//対応したオブジェクトを動かす
	static void ExcuteMoveFunction();
	//指定したLayerのmaskBit取得
	static unsigned int GetMaskFilterBit(FILTER _filter);
private:
	//bodyの位置変更関数リスト
	static std::unordered_map<std::string, std::function<void()>> moveFunctions;
	//layerのフィルターのビットを格納
	static std::unordered_map<FILTER, unsigned int> m_layerFilterBit;
	//bodyIdに対応したオブジェクトの名前を格納
	static std::unordered_map<int32_t, std::string> m_bodyObjectName;

#ifdef DEBUG_TRUE
	// More segments = smoother circle
	static const int numSegments;
	//box用インデックス
	static ComPtr<ID3D11Buffer> m_boxIndexBuffer;
	//Circle用頂点データ
	static ComPtr<ID3D11Buffer> m_circleVertexBuffer;
	//Circle用インデックス
	static ComPtr<ID3D11Buffer> m_circleIndexBuffer;
#endif
};

#ifdef DEBUG_TRUE

//当たり判定の描画用ノードクラス(Debug)
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
	//インデックスの数
	int indexCount = 0;
	//対象のbody
	b2BodyId m_bodyId;
	//Circle用頂点データ
	ComPtr<ID3D11Buffer> m_chainVertexBuffer;
	//Circle用インデックス
	ComPtr<ID3D11Buffer> m_chainIndexBuffer;
};

class Box2DConvexMeshRenderNode : public Box2DMeshRenderNode
{
	friend class Box2DBody;

	Box2DConvexMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId/*, bool _loop*/);
};

#endif


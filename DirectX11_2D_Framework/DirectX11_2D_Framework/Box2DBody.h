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
	//レイヤーの変更
	void SetLayer(const LAYER _layer);
public:
	//bodyIdの取得
	const b2BodyId GetBodyId() { return m_bodyId; }

	//=================================
	// ヒットボックス生成
	//=================================
	//当たり判定の作成(Box)
	void CreateBoxShape();
	//Boxオフセット指定
	void CreateBoxShape(float _offsetX, float _offsetY, float _angle = 0.0f);
	//Boxサイズ指定
	void CreateBoxShape(Vector2 _size, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f);
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
	//力の向きの設定
	void SetVelocity(b2Vec2 _velocity);
	//力の向きの設定(xだけ)
	void SetVelocityX(float _velocityX);
	//力の向きの設定(yだけ)
	void SetVelocityY(float _velocityY);
	//力をじわじわ加える
	void AddForce(b2Vec2 _force);
	//力をぱっと加える
	void AddForceImpule(b2Vec2 _force);
private:
	b2BodyId m_bodyId;
	//Shapeを格納する
	std::vector<b2ShapeId> m_shapeList;
#ifdef DEBUG_TRUE
	std::vector<std::shared_ptr<RenderNode>> m_nodeList;
#endif
};

class Box2DBodyManager
{
	friend class Window;
	friend class Box2DBody;
	friend class SceneManager;
	friend class Box2DBoxRenderNode;
	friend class Box2DCircleRenderNode;
	friend class Box2DCapsuleRenderNode;

public:
	static void DisableLayerCollision(LAYER _layer01, LAYER _layer02);
private:
#ifdef DEBUG_TRUE
	static void Init();
#endif
	//対応したオブジェクトを動かす
	static void ExcuteMoveFunction();
	//指定したLayerのmaskBit取得
	static unsigned int GetMaskLayerBit(LAYER _layer);
private:
	//bodyの位置変更関数リスト
	static std::vector<std::function<void()>> moveFunctions;
	//layerのフィルターのビットを格納
	static std::unordered_map<LAYER, unsigned int> m_layerFilterBit;

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

	
	Box2DMeshRenderNode(std::vector<b2Vec2>& _pointList, b2BodyId _bodyId, bool _loop);
protected:
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


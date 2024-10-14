#pragma once

class Box2DBody : public Component
{
	friend class GameObject;

private:
	Box2DBody(GameObject* _object);
	Box2DBody(GameObject* _object, b2BodyDef* _bodyDef);
	//Box2dWorldから位置と角度を受け取る
	inline void Update() override;
#ifdef DEBUG_TRUE
	void Delete() override;
#endif
public:
	//=================================
	// Circle Cupsule Polygon もつくる
	//=================================
	//当たり判定の作成(Box)
	void CreateBoxShape();
	//Boxオフセット指定
	void CreateBoxShape(float _offsetX, float _offsetY, float _angle = 0.0f);
	//Boxサイズ指定
	void CreateBoxShape(Vector2 _size, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f);
	//当たり判定の作成(Circle)
	void CreateCircleShape();
	//Circleオフセット指定
	void CreateCircleShape(Vector2 _offset);
	//Circleサイズ指定
	void CreateCircleShape(float _diameter,Vector2 _offset = { 0.0f,0.0f });
	//当たり判定の作成(Capsule)
	void CreateCapsuleShape();
	//Capsuleオフセット指定
	void CreateCapsuleShape(float _height, Vector2 _offset = { 0.0f,0.0f }, float _angle = 0.0f);
	//Capsule作成
	void CreateCapsuleShape(float _diameter, float _height, float _angle = 0.0f, Vector2 _offset = { 0.0f,0.0f });
private:
	b2BodyId m_bodyId;
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

private:
#ifdef DEBUG_TRUE
	static void Init();
#endif
	//対応したオブジェクトを動かす
	static void ExcuteMoveFunction();
private:
	static std::vector<std::function<void()>> moveFunctions;

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

#endif


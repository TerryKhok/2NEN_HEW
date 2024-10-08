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

private:
	//対応したオブジェクトを動かす
	static void ExcuteMoveFunction();
private:
	static std::vector<std::function<void()>> moveFunctions;
};

//当たり判定の描画用ノードクラス(Debug)
class Box2DBoxRenderNode : public RenderNode
{
	friend class Box2DBody;

	Box2DBoxRenderNode(Vector2 _size, b2BodyId _bodyId) :m_size(_size) ,m_bodyId(_bodyId),RenderNode(){}
private:
	inline void Draw();
private:
	Vector2 m_size;
	b2BodyId m_bodyId;
};


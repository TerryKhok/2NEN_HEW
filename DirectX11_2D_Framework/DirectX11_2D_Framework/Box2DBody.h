#pragma once

class Box2DBody : public Component
{
	friend class GameObject;

private:
	Box2DBody(GameObject* _object);
	Box2DBody(GameObject* _object, b2BodyDef* _bodyDef);
	//Box2dWorld����ʒu�Ɗp�x���󂯎��
	inline void Update() override;
#ifdef DEBUG_TRUE
	void Delete() override;
#endif
public:
	//=================================
	// Circle Cupsule Polygon ������
	//=================================
	//�����蔻��̍쐬(Box)
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
	//�Ή������I�u�W�F�N�g�𓮂���
	static void ExcuteMoveFunction();
private:
	static std::vector<std::function<void()>> moveFunctions;
};

//�����蔻��̕`��p�m�[�h�N���X(Debug)
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


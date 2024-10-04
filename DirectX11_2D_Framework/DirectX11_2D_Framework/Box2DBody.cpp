#include "Box2DBody.h"

std::vector<std::function<void()>> Box2DBodyManager::moveFunctions;

Box2DBody::Box2DBody(GameObject* _object)
{
	auto& position = _object->transform.position;
	//ボディ定義とワールドIDを使ってグラウンド・ボディを作成する
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { position.x, position.y };

	Box2D::WorldManager::GenerataeBody(m_bodyId, &bodyDef);
}

Box2DBody::Box2DBody(GameObject* _object, b2BodyDef* _bodyDef)
{
	auto& position = _object->transform.position;
	_bodyDef->position = { position.x, position.y };

	Box2D::WorldManager::GenerataeBody(m_bodyId, _bodyDef);
}

inline void Box2DBody::Update()
{
	//m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
	auto func = [&]() 
		{
			auto pos = b2Body_GetPosition(m_bodyId);
			m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
			auto rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
			m_this->transform.angle.z = rot;
		};

	Box2DBodyManager::moveFunctions.emplace_back(func);
}

void Box2DBody::Delete()
{
	for (auto& node : m_nodeList)
	{
		node->Delete();
	}
	b2DestroyBody(m_bodyId);
}

void Box2DBody::CreateBoxShape()
{
	static const float halfObjectSize = DEFULT_OBJECT_SIZE / 2.0f;

	auto& scale = m_this->transform.scale;
	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Polygon polygonBox = b2MakeBox(scale.x * halfObjectSize, scale.y * halfObjectSize);

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(m_bodyId, &shapeDef, &polygonBox);

	auto node = std::shared_ptr<RenderNode>(new Box2DBoxRenderNode({ scale.x,scale.y }, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
}


void Box2DBodyManager::ExcuteMoveFunction()
{
	for (const auto& func : moveFunctions) {
		func();
	}

	moveFunctions.clear();
}

void Box2DBoxRenderNode::Draw()
{
	VSConstantBuffer cb;

	auto transform = m_object->transform;

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_size.x, m_size.y, 1.0f);
	cb.world *= DirectX::XMMatrixRotationZ(transform.angle.z);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, 0.5f);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	cb.projection = DirectX::XMMatrixOrthographicLH(
		SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);
	cb.projection = DirectX::XMMatrixTranspose(cb.projection);

	b2BodyType bodyType = b2Body_GetType(m_bodyId);
	b2Vec2 vec = b2Body_GetLinearVelocity(m_bodyId);
	if (bodyType == b2_dynamicBody && (abs(vec.x) > 150 || abs(vec.y) > 150))
	{
		cb.color = Box2D::b2_colorRed;
	}
	if (bodyType == b2_dynamicBody && (abs(vec.x) > 0 || abs(vec.y) > 0))
	{
		cb.color = Box2D::b2_colorPink;
	}
	else if (bodyType == b2_staticBody)
	{
		cb.color = Box2D::b2_colorGreen;
	}
	else if (bodyType == b2_kinematicBody)
	{
		cb.color = Box2D::b2_colorBlue;
	}
	else
	{
		cb.color = Box2D::b2_colorGray;
	}

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}
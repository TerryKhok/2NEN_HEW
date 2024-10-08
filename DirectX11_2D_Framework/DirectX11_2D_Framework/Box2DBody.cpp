#include "Box2DBody.h"

std::vector<std::function<void()>> Box2DBodyManager::moveFunctions;

Box2DBody::Box2DBody(GameObject* _object)
{
	auto& position = _object->transform.position;
	//�{�f�B��`�ƃ��[���hID���g���ăO���E���h�E�{�f�B���쐬����
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { position.x, position.y };

	Box2D::WorldManager::GenerataeBody(m_bodyId, &bodyDef);
}

Box2DBody::Box2DBody(GameObject* _object, b2BodyDef* _bodyDef)
{
	auto& position = _object->transform.position;
	_bodyDef->position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };

	Box2D::WorldManager::GenerataeBody(m_bodyId, _bodyDef);
}

inline void Box2DBody::Update()
{
	//m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
	auto func = [&]() 
		{
			auto pos = b2Body_GetPosition(m_bodyId);
			pos *= DEFAULT_OBJECT_SIZE;
			m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
			auto rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
			m_this->transform.angle.z = rot;
		};

	Box2DBodyManager::moveFunctions.emplace_back(func);
}

#ifdef DEBUG_TRUE
void Box2DBody::Delete()
{
	for (auto& node : m_nodeList)
	{
		node->Delete();
	}
	b2DestroyBody(m_bodyId);
}
#endif

void Box2DBody::CreateBoxShape()
{
	auto scale = m_this->transform.scale;
	//�n�ʃ|���S�������B b2MakeBox()�w���p�[�֐����g���A�n�ʃ|���S���𔠌^�ɂ���B���̒��S�͐e�{�f�B�̌��_�ł���B
	b2Polygon polygonBox = b2MakeBox(scale.x / 2, scale.y / 2);

	//�V�F�C�v���쐬���Ēn�ʂ̃{�f�B���d�グ��
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(m_bodyId, &shapeDef, &polygonBox);

#ifdef DEBUG_TRUE
	auto node = std::shared_ptr<RenderNode>(new Box2DBoxRenderNode({ scale.x,scale.y }, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif
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

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();

	//���[���h�ϊ��s��̍쐬
	//�[���I�u�W�F�N�g�̈ʒu�E�傫���E�������w��
	cb.world = DirectX::XMMatrixScaling(m_size.x, m_size.y, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(transform.angle.z);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	cb.view = objectCb.view;
	cb.projection = objectCb.projection;

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

	//�e�N�X�`�����s�N�Z���V�F�[�_�[�ɓn��
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

	//�s����V�F�[�_�[�ɓn��
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);

	//���̃|�C���^�ɂȂ�
	NextFunc();
}
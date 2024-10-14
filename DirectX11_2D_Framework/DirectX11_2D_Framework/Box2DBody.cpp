#include "Box2DBody.h"

std::vector<std::function<void()>> Box2DBodyManager::moveFunctions;

#ifdef DEBUG_TRUE
const int Box2DBodyManager::numSegments = 36;
//box用インデックス
ComPtr<ID3D11Buffer> Box2DBodyManager::m_boxIndexBuffer;
//Circle用頂点データ
ComPtr<ID3D11Buffer> Box2DBodyManager::m_circleVertexBuffer;
//Circle用インデックス
ComPtr<ID3D11Buffer> Box2DBodyManager::m_circleIndexBuffer;

void SetDebugBodyColor(b2BodyId _bodyId, DirectX::XMFLOAT4& _color);
#endif

Box2DBody::Box2DBody(GameObject* _object)
{
	auto& position = _object->transform.position;
	//ボディ定義とワールドIDを使ってグラウンド・ボディを作成する
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };
	bodyDef.rotation = b2MakeRot(static_cast<float>(_object->transform.angle.z.Get()));

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	Box2D::WorldManager::GenerataeBody(m_bodyId, &bodyDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
}

Box2DBody::Box2DBody(GameObject* _object, b2BodyDef* _bodyDef)
{
	auto& position = _object->transform.position;
	_bodyDef->position = { position.x / DEFAULT_OBJECT_SIZE, position.y / DEFAULT_OBJECT_SIZE };

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif
	Box2D::WorldManager::GenerataeBody(m_bodyId, _bodyDef);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif
}

inline void Box2DBody::Update()
{
	auto func = [&]() 
		{
			auto pos = b2Body_GetPosition(m_bodyId);
			pos *= DEFAULT_OBJECT_SIZE;
			m_this->transform.position = Vector3(pos.x, pos.y, 0.5f);
			auto rot = b2Rot_GetAngle(b2Body_GetRotation(m_bodyId));
			m_this->transform.angle.z.Set(rot);
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
	CreateBoxShape({ scale.x, scale.y });
}

void Box2DBody::CreateBoxShape(float _offsetX, float _offsetY, float _angle)
{
	auto scale = m_this->transform.scale;
	CreateBoxShape({ scale.x, scale.y }, { _offsetX ,_offsetY }, _angle);
}

void Box2DBody::CreateBoxShape(Vector2 _size, Vector2 _offset, float _angle)
{
	float rad = Math::DegToRad(_angle);

	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Polygon polygonBox = 
		b2MakeOffsetBox(_size.x / 2, _size.y / 2, { _offset.x / DEFAULT_OBJECT_SIZE,_offset.y / DEFAULT_OBJECT_SIZE }, b2MakeRot(rad));

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2CreatePolygonShape(m_bodyId, &shapeDef, &polygonBox);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif


#ifdef DEBUG_TRUE

	Vector2 offset;
	offset.x = _offset.x * cos(rad) - offset.y * sin(rad);
	offset.y = (_offset.x * sin(rad) + offset.y * cos(rad)) / 2;
	auto node = std::shared_ptr<RenderNode>(new Box2DBoxRenderNode(_offset, { _size.x,_size.y }, rad, m_bodyId));
	//auto node = std::shared_ptr<RenderNode>(new Box2DCircleRenderNode(scale.x, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif
}

void Box2DBody::CreateCircleShape()
{
	auto scale = m_this->transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCircleShape(diameter);
}

void Box2DBody::CreateCircleShape(Vector2 _offset)
{
	auto scale = m_this->transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCircleShape(diameter, _offset);
}

void Box2DBody::CreateCircleShape(float _diameter, Vector2 _offset)
{
	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Circle circle = { {_offset.x / DEFAULT_OBJECT_SIZE,_offset.y / DEFAULT_OBJECT_SIZE},_diameter / 2 };

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2CreateCircleShape(m_bodyId, &shapeDef, &circle);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif


#ifdef DEBUG_TRUE
	auto node = std::shared_ptr<RenderNode>(new Box2DCircleRenderNode(_offset,_diameter, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif
}

void Box2DBody::CreateCapsuleShape()
{
	auto& transform = m_this->transform;
	auto scale = transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCapsuleShape(diameter, scale.y / 2);
}

void Box2DBody::CreateCapsuleShape(float _height, Vector2 _offset, float _angle)
{
	auto& transform = m_this->transform;
	auto scale = transform.scale;
	float diameter = (scale.x + scale.y) / 2;
	CreateCapsuleShape(diameter, _height, _angle, _offset);
}

void Box2DBody::CreateCapsuleShape(float _diameter, float _height, float _angle, Vector2 _offset)
{
	float rad = Math::DegToRad(_angle);
	float halfHeight = _height / 2;

	Vector2 offset = _offset;
	offset /= DEFAULT_OBJECT_SIZE;

	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Capsule capsule = {
				{ offset.x + sin(rad) * halfHeight,  offset.y + -cos(rad) * halfHeight },
				{ offset.x + -sin(rad) * halfHeight, offset.y + cos(rad) * halfHeight }, _diameter / 2.0f };

	//シェイプを作成して地面のボディを仕上げる
	b2ShapeDef shapeDef = b2DefaultShapeDef();

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pPauseWorldUpdate();
#endif 
	b2CreateCapsuleShape(m_bodyId, &shapeDef, &capsule);

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::pResumeWorldUpdate();
#endif


#ifdef DEBUG_TRUE
	halfHeight = _height / 2;

	auto node = std::shared_ptr<RenderNode>(
		new Box2DCapsuleRenderNode(_offset, _diameter, halfHeight * DEFAULT_OBJECT_SIZE, _height, rad, m_bodyId));
	node->m_object = m_this;
	m_nodeList.push_back((node));
	RenderManager::AddRenderList(node, LAYER::LAYER_BOX2D_DEBUG);
#endif
}

#ifdef DEBUG_TRUE
void Box2DBodyManager::Init()
{
	std::vector<WORD> indexList{
		2, 0 ,1, 3, 0, 1, 3, 2
	};

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = sizeof(WORD) * static_cast<UINT>(indexList.size());
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA irData;
	irData.pSysMem = indexList.data();
	irData.SysMemPitch = 0;
	irData.SysMemSlicePitch = 0;

	DirectX11::m_pDevice->CreateBuffer(&ibDesc, &irData, m_boxIndexBuffer.GetAddressOf());

	float radius = HALF_OBJECT_SIZE;
	std::vector<Vertex> vertices;

	for (int i = 0; i <= numSegments; i++) {
		float theta = (float)i / (float)numSegments * DirectX::XM_2PI;
		float x = radius * cosf(theta);
		float y = radius * sinf(theta);

		vertices.push_back({ x, y, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f ,1.0f,1.0f });
	}

	vertices.push_back({ 0.0f,0.0f,0.5f, 1.0f,1.0f,1.0f, 1.0f,0.0f,0.0f });

	std::vector<WORD> indices;

	for (int i = 0; i < numSegments; i++) {
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	indices.push_back(numSegments);
	indices.push_back(numSegments + 1);

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * static_cast<WORD>(vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();

	DirectX11::m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, m_circleVertexBuffer.GetAddressOf());

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(WORD) * static_cast<WORD>(indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices.data();

	DirectX11::m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, m_circleIndexBuffer.GetAddressOf());

	auto size = indices.size();
}
#endif


void Box2DBodyManager::ExcuteMoveFunction()
{
	for (const auto& func : moveFunctions) {
		func();
	}

	moveFunctions.clear();
}

#ifdef DEBUG_TRUE

void Box2DBoxRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_boxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();

	auto rad = static_cast<float>(transform.angle.z.Get());
	Vector2 offset;
	offset.x = m_offset.x * cos(rad) - m_offset.y * sin(rad);
	offset.y = m_offset.x * sin(rad) + m_offset.y * cos(rad);

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_size.x, m_size.y, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad + m_angle);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x + offset.x, transform.position.y + offset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	cb.view = objectCb.view;
	cb.projection = objectCb.projection;

	SetDebugBodyColor(m_bodyId, cb.color);

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(8, 0, 0);


	//次のポインタにつなぐ
	NextFunc();
}

inline void Box2DCircleRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, Box2DBodyManager::m_circleVertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_circleIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();


	auto rad = static_cast<float>(transform.angle.z.Get());
	Vector2 offset;
	offset.x = m_offset.x * cos(rad) - m_offset.y * sin(rad);
	offset.y = m_offset.x * sin(rad) + m_offset.y * cos(rad);

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_size, m_size, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad);
	cb.world *= DirectX::XMMatrixTranslation(transform.position.x + offset.x, transform.position.y + offset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	cb.view = objectCb.view;
	cb.projection = objectCb.projection;

	SetDebugBodyColor(m_bodyId, cb.color);

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(Box2DBodyManager::numSegments * 2 + 2, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}

inline void Box2DCapsuleRenderNode::Draw()
{
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, Box2DBodyManager::m_circleVertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_circleIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	static VSConstantBuffer cb;

	const auto& transform = m_object->transform;
	const auto& objectCb = m_object->GetContantBuffer();

	const float objectRad = static_cast<float>(transform.angle.z.Get());
	const float rad = objectRad + m_angle;

	Vector2 offset;
	offset.x = transform.position.x + m_offset.x * cos(objectRad) - m_offset.y * sin(objectRad);
	offset.y = transform.position.y + m_offset.x * sin(objectRad) + m_offset.y * cos(objectRad);

	Vector2 heightOffset;
	heightOffset.x = m_sizeY * sin(rad);
	heightOffset.y = m_sizeY * cos(rad);

	//上の半球描画
	//======================================================================================================================
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_diameter, m_diameter, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad);
	cb.world *= DirectX::XMMatrixTranslation(offset.x - heightOffset.x, offset.y + heightOffset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);
	cb.view = objectCb.view;
	cb.projection = objectCb.projection;

	SetDebugBodyColor(m_bodyId, cb.color);

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(Box2DBodyManager::numSegments, 0, 0);

	//下の半球描画
	//======================================================================================================================
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_diameter, m_diameter, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(rad + Math::PI);
	cb.world *= DirectX::XMMatrixTranslation(offset.x + heightOffset.x, offset.y - heightOffset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(Box2DBodyManager::numSegments, 0, 0);

	DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
	DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_boxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	//胴体のbox描画
	//======================================================================================================================
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	cb.world = DirectX::XMMatrixScaling(m_diameter, m_height, transform.scale.z);
	cb.world *= DirectX::XMMatrixRotationZ(objectRad + m_angle);
	cb.world *= DirectX::XMMatrixTranslation(offset.x, offset.y, transform.position.z);
	cb.world = DirectX::XMMatrixTranspose(cb.world);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(4, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}

void SetDebugBodyColor(b2BodyId _bodyId, DirectX::XMFLOAT4& _color)
{
	b2BodyType bodyType = b2Body_GetType(_bodyId);
	b2Vec2 vec = b2Body_GetLinearVelocity(_bodyId);
	if (bodyType == b2_dynamicBody && (abs(vec.x) > 150 || abs(vec.y) > 150))
	{
		_color = Box2D::b2_colorRed;
	}
	if (bodyType == b2_dynamicBody && (abs(vec.x) > 0 || abs(vec.y) > 0))
	{
		_color = Box2D::b2_colorPink;
	}
	else if (bodyType == b2_staticBody)
	{
		_color = Box2D::b2_colorGreen;
	}
	else if (bodyType == b2_kinematicBody)
	{
		_color = Box2D::b2_colorBlue;
	}
	else
	{
		_color = Box2D::b2_colorGray;
	}
}

#endif
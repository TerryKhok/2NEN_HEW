#include "Object.h"

VSConstantBuffer GameObject::m_cb = {};

void Object::SetPosition(Vector2 _vec2)
{
	m_position.x = _vec2.x;
	m_position.y = _vec2.y;
}

void Object::SetPosition(XMFLOAT3 _position)
{
	m_position = _position;
}

VSConstantBuffer& GameObject::GetContantBuffer()
{
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	m_cb.world = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	m_cb.world *= DirectX::XMMatrixRotationZ(m_angle);
	m_cb.world *= DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	m_cb.world = DirectX::XMMatrixTranspose(m_cb.world);
	m_cb.projection = DirectX::XMMatrixOrthographicLH(
		SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);
	m_cb.projection = DirectX::XMMatrixTranspose(m_cb.projection);

	return m_cb;
}

//void GameObject::AddComponentList(const char* _typename, Component* _component)
//{
//	m_componentList[_typename] = std::unique_ptr<Component>(_component);
//}


GameObject::~GameObject()
{
	for (auto& component : m_componentList)
	{
		component.second->Delete();
	}

	m_componentList.clear();
}

void GameObject::SetLayer(LAYER _layer)
{
	m_layer = _layer;
}

const LAYER GameObject::GetLayer() const
{
	return m_layer;
}

template<>
Renderer* GameObject::AddComponent<Renderer>(void)
{
	if (ExistComponent<Renderer>()) return nullptr;

	Component* component = nullptr;
	component = new Renderer(this);

	m_componentList[typeid(Renderer).name()] = std::unique_ptr<Component>(component);

	Renderer* render = dynamic_cast<Renderer*>(component);
	if (render == nullptr)
	{
		LOG("%s component down_cast faild", typeid(Renderer).name());
	}

	return render;
}

template<>
Renderer* GameObject::AddComponent<Renderer>(const wchar_t* _texPath)
{
	if (ExistComponent<Renderer>()) return nullptr;

	Component* component = nullptr;
	component = new Renderer(this, _texPath);

	m_componentList[typeid(Renderer).name()] = std::unique_ptr<Component>(component);

	Renderer* render = dynamic_cast<Renderer*>(component);
	if (render == nullptr)
	{
		LOG("%s component down_cast faild", typeid(Renderer).name());
	}

	return render;
}

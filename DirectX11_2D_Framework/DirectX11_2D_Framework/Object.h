#pragma once

#define DEFULT_OBJECT_SIZE 100.0f

class Renderer;

class Object
{
public:
	//位置の更新
	void SetPosition(Vector2 _vec2);
	void SetPosition(XMFLOAT3 _position);
protected:
	XMFLOAT3 m_position = { 0.0f,0.0f,0.5f };
	XMFLOAT3 m_scale = { 1.0f,1.0f,1.0f };
	float m_angle = 0.0f;
};

class GameObject : public Object
{
	friend class RenderNode;

private:
	//プロジェクション行列変換までして渡す（描画以外では基本使わない）
	VSConstantBuffer& GetContantBuffer();
	//すでにコンポーネントがついてるか確かめる
	template<typename T>
	bool ExistComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			LOG("%s component is exist", typeid(T).name());
			return true;
		}

		return false;
	}
public:
	//デストラクタ(コンポーネント削除)
	~GameObject();
	//レイヤーの設定（誰でも変えれるのでまた制限する）
	void SetLayer(LAYER _layer);
	//レイヤー取得
	const LAYER GetLayer() const;
	//コンポーネント追加
	template<typename T>
	T* AddComponent(void)
	{
		if (ExistComponent<T>()) return nullptr;

		Component* component = nullptr;
		component = new T();

		//リストに追加
		m_componentList[typeid(T).name()] = std::unique_ptr<Component>(component);

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG("%s component down_cast faild", typeid(T).name());
		}

		return downcast;
	}
	//コンポーネント追加(引数あり)
	template<typename T,typename Arg>
	T* AddComponent(Arg arg)
	{
		if (ExistComponent<T>()) return nullptr;

		Component* component = nullptr;
		component = new T(arg);

		m_componentList[typeid(T).name()] = std::unique_ptr<Component>(component);

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG("%s component down_cast faild", typeid(T).name());
		}

		return downcast;
	}
	//RenderComponent用
	template<>
	Renderer* AddComponent<Renderer>(void);
	//テクスチャ指定
	template<>
	Renderer* AddComponent<Renderer, const wchar_t*>(const wchar_t* _texPath);
	//コンポーネント削除
	template<typename T>
	void RemoveComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			iter->second->Delete();
		}

		m_componentList.erase(iter);
	}
private:
	static VSConstantBuffer m_cb;
	LAYER m_layer = LAYER::LAYER_01;
	std::unordered_map<const char*, std::unique_ptr<Component>> m_componentList;
};


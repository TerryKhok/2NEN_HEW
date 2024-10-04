#pragma once

//Scale = 1.0f のときの大きさ
const float DEFULT_OBJECT_SIZE = 10.0f;

class Renderer;

struct Transform final
{
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	GameObject* gameobject = nullptr;
	Vector3 position = { 0.0f,0.0f,0.5f };
	Vector3 scale =	{ 10.0f,10.0f ,1.0f };
	Vector3 angle = { 0.0f,0.0f,0.0f };
};

//class Collider2D : public Component;

class GameObject final
{
	friend class RenderNode;
	friend class UVRenderNode;
	friend class ObjectManager;
	friend class Scene;
	friend class Component;

private:
	//コンストラクタ
	GameObject() { transform.gameobject = this; }
	//名前決定
	GameObject(std::string _name);
	//デストラクタ(コンポーネント削除)
	~GameObject();

	//プロジェクション行列変換までして渡す（描画以外では基本使わない）
	VSConstantBuffer& GetContantBuffer();
	//すでにコンポーネントがついてるか確かめる
	template<typename T>
	bool ExistComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			LOG_WARNING("%s : %s component is exist", name.c_str(), typeid(T).name());
			return true;
		}

		return false;
	}
	//コンポーネントの更新
	void UpdateComponent();

public:
	//アクティブを変更する
	void SetActive(bool _active);
	//レイヤーの設定（頻繁に使用しない）
	void SetLayer(const LAYER _layer);
	//名前の変更（頻繁に使用しない）
	void SetName(const std::string _name);
	//名前の取得
	const std::string GetName() const;
	//レイヤー取得
	const LAYER GetLayer() const;
	//コンポーネント追加
	template<typename T>
	T* AddComponent(void)
	{
		if (ExistComponent<T>()) return nullptr;

		Component* component = nullptr;
		component = new T();
		component->m_this = this;

		//リストに追加(デストラクタ登録)
		m_componentList.insert(std::make_pair(typeid(Component).name(),
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast faild",name.c_str(), typeid(T).name());
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
		component->m_this = this;

		//リストに追加(デストラクタ登録)
		m_componentList.insert(std::make_pair(typeid(Component).name(),
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; })));

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast faild", name.c_str(), typeid(T).name());
		}

		return downcast;
	}

	//RenderComponent完全特殊化
	template<>
	Renderer* AddComponent<Renderer>();
	//テクスチャ指定
	template<>
	Renderer* AddComponent<Renderer, const wchar_t*>(const wchar_t* _texPath);
	//Box2DBodyComponent完全特殊化
	template<>
	Box2DBody* AddComponent<Box2DBody>();
	//bodyDef指定
	template<>
	Box2DBody* AddComponent<Box2DBody,b2BodyDef*>(b2BodyDef* _bodyDef);
	//コンポーネント削除
	template<typename T>
	void RemoveComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			iter->second->Delete();
			m_componentList.erase(iter);
		}
	}
	template<typename T>
	T* GetComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			T* downcast = dynamic_cast<T*>(iter->second.get());
			if (downcast == nullptr)
			{
				LOG_WARNING("%s : %s component down_cast faild", name.c_str(), typeid(T).name());
			}
			return downcast;
		}

		return nullptr;
	}
	//tarnsformComponent完全特殊化
	template<>
	Transform* GetComponent(void);
public:
	Transform transform;
private:
	std::string name = "GameObject";
	static VSConstantBuffer m_cb;
	LAYER m_layer = LAYER::LAYER_01;
	std::unordered_map<const char*, std::unique_ptr<Component, void(*)(Component*)>> m_componentList;
};

//==================================================
// 関数ポインタを使って初期化処理リストにいれるか
// そのまま発動させるかをきりわける
//==================================================
//
class ObjectManager final
{
	friend class Window;
	friend class Component;
	friend class Scene;
	friend class SceneManager;
	friend void GameObject::SetName(const std::string);

	using ObjectList = std::pair<std::unordered_set<std::string>, std::vector<std::unique_ptr<GameObject, void(*)(GameObject*)>>>;
	//using ObjectList = std::unordered_set<std::string, std::unique_ptr<GameObject, void(*)(GameObject*)>>;

public:
	//オブジェクト一覧から見つける(頻繁に使用しない)
	static GameObject* Find(std::string _name);
private:
	//生成禁止
	ObjectManager();
	//新しいリストにする
	static void GenerateList();
	//オブジェクトかたずけ
	static void Uninit();
	//オブジェクトについたコンポーネント更新
	static void UpdateObjectComponent();
	//オブジェクトの追加・名前の重複禁止
	static GameObject* AddObject(GameObject* _gameObject);
	//スレッドの現在のリストを次のリストに変更
	static void ChangeNextObjectList();
	//次のノードリストに繋ぐ
	static void LinkNextObjectList();
	//オブジェクトの名前を変更する
	static void ChangeObjectName(std::string _before,std::string _after);
private:
	// スレッドごとの現在のリスト
	static thread_local ObjectList* m_currentList;
	//オブジェクトを格納
	static std::unique_ptr<ObjectList> m_objectList;
	//次のオブジェクトを格納
	static std::unique_ptr<ObjectList> m_nextObjectList;
};


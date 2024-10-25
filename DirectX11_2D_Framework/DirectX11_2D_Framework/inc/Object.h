#pragma once

//Scale = 1.0f のときの大きさ
//===========================================================
const float DEFAULT_OBJECT_SIZE = 10.0f;
const float HALF_OBJECT_SIZE = DEFAULT_OBJECT_SIZE / 2.0f;
//===========================================================

class Renderer;

struct Transform final
{
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	GameObject* gameobject = nullptr;
	Vector3 position = { 0.0f,0.0f,0.0f };
	Vector3 scale =	{ 5.0f,5.0f,1.0f };
	Angle3D angle = { 0.0f,0.0f,0.0f };
};


class GameObject final
{
	friend class RenderNode;
	friend class UVRenderNode;
	friend class Box2DBoxRenderNode;
	friend class Box2DCircleRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class Box2DMeshRenderNode;
	friend class ObjectManager;
	friend class Scene;
	friend class Component;
	friend class CameraManager;

	using functionPointer = void (GameObject::*)();
private:
	//コンストラクタ
	GameObject() { transform.gameobject = this; }
	//名前決定
	GameObject(std::string _name);
	//デストラクタ(コンポーネント削除)
	~GameObject();

	//プロジェクション行列変換までして渡す（描画以外では基本使わない）
	VSObjectConstantBuffer& GetContantBuffer();
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
	//なにもしない
	void Void(){}
private:
	functionPointer pUpdate = &GameObject::UpdateComponent;
public:
	//アクティブを変更する(※処理が多いため頻繁に使用しない)
	void SetActive(bool _active);
	//レイヤーの設定（※処理が多いため頻繁に使用しない）
	void SetLayer(const LAYER _layer);
	//名前の変更
	void SetName(const std::string _name);
	//名前の取得
	const std::string GetName() const;
	//レイヤー取得
	const LAYER GetLayer() const;
	//コンポーネント追加
	template<typename T>
	T* AddComponent(void)
	{
		if (ExistComponent<T>()) return GetComponent<T>();

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
	T* AddComponent(Arg _arg)
	{
		if (ExistComponent<T>()) return GetComponent<T>();

		Component* component = nullptr;
		component = new T(_arg);
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
	//アニメーター指定
	template<>
	Renderer* AddComponent<Renderer,Animator*>(Animator* _animator);
	//Animator完全特殊化
	template<>
	Animator* AddComponent<Animator>();
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
#ifdef DEBUG_TRUE
			//コンポーネントのSafePointerをnullptrにする
			PointerRegistryManager::deletePointer(iter->second.get());
#endif 
			iter->second->Delete();
			m_componentList.erase(iter);
		}
	}
	template<>
	void RemoveComponent<Renderer>();

	template<typename T>
	T* GetComponent()
	{
		auto iter = m_componentList.find(typeid(T).name());
		if (iter != m_componentList.end())
		{
			T* downcast = dynamic_cast<T*>(iter->second.get());
			if (downcast == nullptr)
			{
				LOG_ERROR("%s : %s component down_cast faild", name.c_str(), typeid(T).name());
			}
			return downcast;
		}

		LOG_WARNING("%s : %s component not exist",name.c_str(),typeid(T).name());
		return nullptr;
	}
	//tarnsformComponent完全特殊化
	template<>
	Transform* GetComponent(void);
public:
	Transform transform;
private:
	std::string name = "GameObject";
	static VSObjectConstantBuffer m_cb;
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

	using ObjectList = std::unordered_map<std::string, std::unique_ptr<GameObject, void(*)(GameObject*)>>;

public:
	//オブジェクト一覧から見つける アクセス速度n(1)
	static GameObject* Find(const std::string& _name);
private:
	//生成禁止
	ObjectManager() = delete;
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
	static void ChangeObjectName(const std::string& _before,const std::string& _after);
	//全てのリストを明示的に綺麗にする
	static void CleanAllObjectList();
private:
	// スレッドごとの現在のリスト
	static thread_local ObjectList* m_currentList;
	//オブジェクトを格納
	static std::unique_ptr<ObjectList> m_objectList;
	//次のオブジェクトを格納
	static std::unique_ptr<ObjectList> m_nextObjectList;
	//削除するオブジェクトを格納
	static std::unique_ptr<ObjectList> m_eraseObjectList;
};


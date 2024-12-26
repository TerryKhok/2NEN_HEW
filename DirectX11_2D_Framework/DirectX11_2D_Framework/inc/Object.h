#pragma once

//Scale = 1.0f のときの大きさ
//================================================================
constexpr float DEFAULT_OBJECT_SIZE = 10.0f;
constexpr float HALF_OBJECT_SIZE = DEFAULT_OBJECT_SIZE / 2.0f;
constexpr float QUARTER_OBJECT_SIZE = DEFAULT_OBJECT_SIZE / 4.0f;
//================================================================

class Renderer;
class SubWindow;

struct Transform final
{
	//メモリ確保禁止
	void* operator new(size_t) = delete;

	GameObject* gameobject = nullptr;
	Vector3 position = { 0.0f,0.0f,0.0f };
	Vector3 scale =	{ 5.0f,5.0f,1.0f };
	Angle3D angle = { 0.0f,0.0f,0.0f };

private:
	GENERATE_SERIALIZE(position, scale, angle)
};

//class FileScene;

class GameObject final
{
	friend class Window;
	friend class Scene;
	friend class FileScene;
	friend class ImGuiApp;
	friend class Component;
	friend class ObjectManager;
	friend class CameraManager;
	friend class RenderNode;
	friend class UVRenderNode;
	friend class Box2D::WorldManager;
	friend class Box2DBoxRenderNode;
	friend class Box2DCircleRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class Box2DMeshRenderNode;	
	friend class Box2DLineRenderNode;

	template<typename T>
	friend bool CreateComponent(GameObject* obj, SERIALIZE_INPUT& ar);

	using functionPointer = void (GameObject::*)();
	
	using ComponentListMap = std::unordered_map<std::string, size_t>;
	using ComponentList = std::pair <std::vector<std::unique_ptr<Component, void(*)(Component*)>>, ComponentListMap>;

private:
	//コンストラクタ
	GameObject() { transform.gameobject = this; }
	//名前決定
	GameObject(std::string _name);
	//デストラクタ(コンポーネント削除)
	~GameObject();

	//プロジェクション行列変換までして渡す（描画以外では基本使わない）
	VSObjectConstantBuffer& GetConstantBuffer();
	//コンポーネントの更新
	void UpdateComponent();
	//なにもしない
	void Void(){}
private:
	functionPointer pUpdate = &GameObject::UpdateComponent;
public:
	//アクティブを変更する(※処理が多いため頻繁に使用しない)
	void SetActive(bool _active);
	//名前の変更
	void SetName(const std::string _name);
	//名前の取得
	const std::string GetName() const;
	//すでにコンポーネントがついてるか確かめる
	template<typename T>
	bool ExistComponent()
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			return true;
		}

		return false;
	}
	//コンポーネント追加
	template<typename T>
	SAFE_TYPE(T) AddComponent(void)
	{
		if (ExistComponent<T>()) return GetComponent<T>();

		Component* component = nullptr;
		component = new T();
		component->m_this = this;

		TRY_CATCH_LOG(component->Start());

		//リストに追加(デストラクタ登録)
		m_componentList.first.emplace_back(
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
		m_componentList.second[typeid(T).name()] = m_componentList.first.size() - 1;

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast failed",name.c_str(), typeid(T).name());
		}

		return downcast;
	}
	//コンポーネント追加(引数あり)
	template<typename T,typename Arg>
	SAFE_TYPE(T) AddComponent(Arg _arg)
	{
		if (ExistComponent<T>()) return GetComponent<T>();

		Component* component = nullptr;
		component = new T(_arg);
		component->m_this = this;

		TRY_CATCH_LOG(component->Start());

		//リストに追加(デストラクタ登録)
		m_componentList.first.emplace_back(
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
		m_componentList.second[typeid(T).name()] = m_componentList.first.size() - 1;

		T* downcast = dynamic_cast<T*>(component);
		if (downcast == nullptr)
		{
			LOG_WARNING("%s : %s component down_cast failed", name.c_str(), typeid(T).name());
		}

		return downcast;
	}

	//RenderComponent完全特殊化
	template<>
	SAFE_TYPE(Renderer) AddComponent<Renderer>();
	//テクスチャ指定
	template<>
	SAFE_TYPE(Renderer) AddComponent<Renderer, const wchar_t*>(const wchar_t* _texPath);
	//アニメーター指定
	template<>
	SAFE_TYPE(Renderer) AddComponent<Renderer,Animator*>(Animator* _animator);
	//Animator完全特殊化
	template<>
	SAFE_TYPE(Animator) AddComponent<Animator>();
	//Box2DBodyComponent完全特殊化
	template<>
	SAFE_TYPE(Box2DBody) AddComponent<Box2DBody>();
	//bodyDef指定
	template<>
	SAFE_TYPE(Box2DBody) AddComponent<Box2DBody,b2BodyDef*>(b2BodyDef* _bodyDef);
	template<>
	SAFE_TYPE(SubWindow) AddComponent<SubWindow>();
	//bodyDef指定
	template<>
	SAFE_TYPE(SubWindow) AddComponent<SubWindow, const char*>(const char* _windowName);
	template<>
	SAFE_TYPE(SFText) AddComponent<SFText>();
	template<>
	SAFE_TYPE(SFText) AddComponent<SFText>(const char* _str);

	//コンポーネント削除
	template<typename T>
	void RemoveComponent()
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			auto& component = m_componentList.first[iter->second];
#ifdef DEBUG_TRUE
			//コンポーネントのSafePointerをNullptrにする
			PointerRegistryManager::deletePointer(component.get());
#endif 
			auto& vector = m_componentList.first;
			auto& map = m_componentList.second;

			component->Delete();
			size_t index = iter->second;
			size_t lastIndex = vector.size() - 1;

			// Move the last entity to the position of the entity to be removed
			if (index != lastIndex) {
				vector[index] = std::move(vector[lastIndex]);
				map[vector[index]->getType().c_str()] = index; // Update map for the moved entity
			}

			// Remove the last element and update map
			vector.pop_back();
			map.erase(iter);
		}
	}
	template<>
	void RemoveComponent<Renderer>();

	template<typename T>
	SAFE_TYPE(T) GetComponent()
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			T* downcast = dynamic_cast<T*>(m_componentList.first[iter->second].get());
			if (downcast == nullptr)
			{
				LOG_ERROR("%s : %s component down_cast failed", name.c_str(), typeid(T).name());
			}
			return downcast;
		}

		LOG_WARNING("%s : %s component not exist",name.c_str(),typeid(T).name());
		return nullptr;
	}
	//transformComponent完全特殊化
	template<>
	SAFE_TYPE(Transform) GetComponent(void);

	template<typename T>
	bool TryGetComponent(T** _output)
	{
		auto iter = m_componentList.second.find(typeid(T).name());
		if (iter != m_componentList.second.end())
		{
			T* downcast = dynamic_cast<T*>(m_componentList.first[iter->second].get());
			if (downcast == nullptr)
			{
				LOG_ERROR("%s : %s component down_cast failed", name.c_str(), typeid(T).name());
				return false;
			}
			*_output = downcast;
			return true;
		}

		*_output = nullptr;
		return false;
	}

	template<>
	bool TryGetComponent(Transform** _output);
public:
	Transform transform;
private:
	std::string name = "GameObject";
	bool active = true;
	static VSObjectConstantBuffer m_cb;
	ComponentList m_componentList = {};

#ifdef DEBUG_TRUE
	enum SELECT_TYPE
	{
		SELECT_NONE,
		SELECTED,
		ON_MOUSE,
	};

	SELECT_TYPE isSelected = SELECT_NONE;
	size_t selectedNum = 0;
#endif
	
private:
	template<typename T>
	void LoadComponent(SERIALIZE_INPUT& ar)
	{
		if (ExistComponent<T>())  return;

		Component* component = nullptr;
		component = new T(this, ar);
		component->m_this = this;

		TRY_CATCH_LOG(component->Start());

		//リストに追加(デストラクタ登録)
		m_componentList.first.emplace_back(
			std::unique_ptr<Component, void(*)(Component*)>(component, [](Component* p) {delete p; }));
		m_componentList.second[typeid(T).name()] = m_componentList.first.size() - 1;

		return;
	}

	// Custom save function
	template <class Archive>
	void save(Archive& archive) const {
		std::vector<std::string> comNames;
		for (auto& com : m_componentList.second)
		{
			comNames.push_back(com.first);
		}
		archive(CEREAL_NVP(name), CEREAL_NVP(transform), CEREAL_NVP(active), CEREAL_NVP(comNames));
		for (auto& com : m_componentList.first)
		{
			com->Serialize(archive);
		}
	}

	// Custom load function
	template <class Archive>
	void load(Archive& archive) {
		std::vector<std::string> comNames;
		archive(CEREAL_NVP(name), CEREAL_NVP(transform), CEREAL_NVP(active), CEREAL_NVP(comNames));
		int count = 1;
		std::string uniqueName = name;
		auto& list = ObjectManager::m_currentList;

		while (list->second.find(uniqueName) != list->second.end()) {
			uniqueName = name + "_" + std::to_string(count++);
		}

#ifdef DEBUG_TRUE
		if (name != uniqueName)
		{
			LOG("%s name existed, so we changed %s.", name.c_str(), uniqueName.c_str());
		}
#endif
		name = uniqueName;
		for (auto& name : comNames)
		{
			AssemblyComponent::CreateComponent(name, this, archive);
		}
		for (auto& com : m_componentList.first)
		{
			com->Deserialize(archive);
		}
	}

	// Declare Cereal archive types as friends
	friend class cereal::access;
};

//==================================================
// 関数ポインタを使って初期化処理リストにいれるか
// そのまま発動させるかをきりわける
//==================================================
//
class ObjectManager final
{
	friend class Window;
	friend class ImGuiApp;
	friend class Component;
	friend class Scene;
	friend class FileScene;
	friend class SceneManager;
	friend void GameObject::SetName(const std::string);
	template <class Archive>
	friend void GameObject::load(Archive& archive);

	using ObjectListMap = std::unordered_map < std::string, size_t>;

	using ObjectList = std::pair<std::vector<std::unique_ptr<GameObject, void(*)(GameObject*)>>, ObjectListMap>;

public:
	//オブジェクト一覧から見つける アクセス速度n(1)なのではやい
	static SAFE_TYPE(GameObject) Find(const std::string& _name);
private:
	//生成禁止
	ObjectManager() = delete;
	//新しいリストにする
	static void GenerateList();
	//オブジェクトかたずけ
	static void UnInit();
	//オブジェクトについたコンポーネント更新
	static void UpdateObjectComponent();
	//オブジェクトの追加・名前の重複禁止
	static void AddObject(GameObject* _gameObject);
	//オブジェクトをファイルから読み込む
	static void AddObject(std::filesystem::path& _path);
	//オブジェクトの削除
	static void DeleteObject(ObjectListMap::iterator& _iter);
	static void DeleteObject(std::string _name);
	static void DeleteObjectDelay(std::string _name);
	//スレッドの現在のリストを次のリストに変更
	static void ChangeNextObjectList();
	//次のノードリストに繋ぐ
	static void LinkNextObjectList();
	//オブジェクトの名前を変更する
	static void ChangeObjectName(const std::string& _before,const std::string& _after);
	//全てのリストを明示的に綺麗にする
	static void CleanAllObjectList();
private:
	static thread_local void(*pDeleteObject)(std::string);
private:
	// スレッドごとの現在のリスト
	static thread_local ObjectList* m_currentList;
	//オブジェクトを格納
	static std::unique_ptr<ObjectList> m_objectList;
	//次のオブジェクトを格納
	static std::unique_ptr<ObjectList> m_nextObjectList;
	//削除するオブジェクトを格納
	static std::unique_ptr<ObjectList> m_eraseObjectList;
	//削除を遅延しているオブジェクトを格納
	static std::vector<std::string> m_delayEraseObjectName;
};


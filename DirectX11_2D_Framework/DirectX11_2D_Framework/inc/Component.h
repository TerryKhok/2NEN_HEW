#pragma once


class Component
{
	friend class Window;
	friend class GameObject;
	friend class Box2D::WorldManager;
	friend class ImGuiApp;

protected:
	//生成禁止
	Component(){}
	//削除禁止
	virtual ~Component() = default;
	//コンポーネントをつけているオブジェクト
	GameObject* m_this = nullptr;

	//オブジェクト生成
	GameObject* Instantiate();
	//オブジェクト生成(名前指定)
	GameObject* Instantiate(std::string _name);
	//オブジェクト生成(名前,テクスチャ指定)
	GameObject* Instantiate(std::string _name, const wchar_t* _texPath);

	//Updateの中で削除できないように遅延する
	//================================================================================
	//オブジェクトの削除(ポインタ指定)
	void DeleteObject(GameObject* _object);
	//オブジェクトの削除(名前指定)
	inline void DeleteObject(std::string _name);
	//================================================================================
	// 
	// Get the type name of the derived class
	std::string getType() const {
		return demangle(typeid(*this).name());
	}
private:
	//アクティブを変更
	virtual void SetActive(bool _active) {}
	//コンポーネント削除処理
	virtual void Delete() {}
	//オプション： デマングリング関数により、コンパイラによってはより美しい出力が得られる。
	static std::string demangle(const char* name) {
#ifdef __GNUG__
		int status = -1;
		std::unique_ptr<char, void(*)(void*)> res{
			abi::__cxa_demangle(name, nullptr, nullptr, &status),
			std::free
		};
		return (status == 0) ? res.get() : name;
#else
		return name;  // Fallback if demangling is not available
#endif
	}
private:
//============================================
// Awakeはまだ機能してない
//============================================
	//初めに呼び出す
	virtual void Awake() {}
	//Awakeが終わってから呼び出す
	virtual void Start() {}
	//アクティブになったときに呼び出す
	virtual void OnEnable() {}
	//毎フレーム呼び出す
	virtual inline void Update() {}
	//非アクティブになったときに呼び出す
	virtual void OnDisable() {}
	//オブジェクトのセンサーに侵入したとき
	virtual void OnColliderEnter(GameObject* _ohter) {}
	//オブジェクトのセンサーから出た時
	virtual void OnColliderExit(GameObject* _other) {}
	//オブジェクトのセンサーに侵入したとき
	virtual void OnCollisionEnter(GameObject* _ohter) {}
	//オブジェクトのセンサーから出た時
	virtual void OnCollisionExit(GameObject* _other) {}
	//ウィンドウを触った時
	virtual void OnWindowEnter(HWND _target) {}
	//ウィンドウを離した時
	virtual void OnWindowExit(HWND _target) {}
	//ウィンドウを動かしたとき
	virtual void OnWindowMove(HWND _target, RECT* _rect) {}
	//シリアライズの際に呼び出す
	virtual void Serialize(SERIALIZE_OUTPUT& ar)
	{
		//ar(CEREAL_NVP(version));
	}
	//デシリアライズの際に呼び出す
	virtual void Deserialize(SERIALIZE_INPUT& ar) 
	{
		//::cereal::make_optional_nvp(ar, "version", version);
	}
private:
	//============================================
	// imGuiで描画する
	//============================================
	virtual void DrawImGui(ImGuiApp::HandleUI& _handle){
		ImGui::Text(" not override DrawImGui function!");
	}
private:
	int version = 0;
};


class AssemblyComponent final
{
	template<typename T>
	friend class ReflectionComponent;

	friend class Window;
	friend class ImGuiApp;

	class IReflection final
	{
	public:
#ifndef DEBUG_TRUE
		IReflection(std::function<bool(GameObject*, SERIALIZE_INPUT&)>&& _create)
			:createComponent(_create)
		{
		};
#endif
		~IReflection() = default;

		std::function<bool(GameObject*,SERIALIZE_INPUT&)> createComponent;

#ifdef DEBUG_TRUE
		IReflection(std::function<bool(GameObject*, SERIALIZE_INPUT&)>&& _create,std::function<void(GameObject*)>&& _add)
			:createComponent(_create),addComponent(_add)
		{};
		
		std::function<void(GameObject*)> addComponent;
#endif
	};
private:
	static inline std::map<std::string, IReflection> assemblies;

public:
	static void CreateComponent(const std::string& _comName, GameObject* _obj,SERIALIZE_INPUT& ar)
	{
		auto iter = assemblies.find(_comName);
		if (iter != assemblies.end())
		{
			iter->second.createComponent(_obj, ar);
		}
	}
};
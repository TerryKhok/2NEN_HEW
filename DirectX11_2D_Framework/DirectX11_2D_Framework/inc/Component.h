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
	//オブジェクトの削除(ポインタ指定)
	void DeleteObject(GameObject* _object);
	//オブジェクトの削除(名前指定)
	inline void DeleteObject(std::string _name);
	// Get the type name of the derived class
	std::string getType() const {
		return demangle(typeid(*this).name());
	}
private:
	//アクティブを変更
	virtual void SetActive(bool _active) {}
	//コンポーネント削除処理
	virtual void Delete() {}
	// Optional: Demangling function for nicer output on some compilers
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
private:
	//============================================
	// imGuiで描画する
	//============================================
	virtual void DrawImGui(ImGuiApp::HandleUI& _handle){
		ImGui::Text(" not override DrawImGui function!");
	}
};

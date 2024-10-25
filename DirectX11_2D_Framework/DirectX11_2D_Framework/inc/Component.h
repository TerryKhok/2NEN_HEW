#pragma once



//============================================
// Updateしか実装できてない
// Awakeとかも実装する
//============================================

class Component
{
	friend class GameObject;

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
private:
	//アクティブを変更
	virtual void SetActive(bool _active) {}
	//レイヤーを変更
	virtual void SetLayer(const LAYER _layer) {}
	//コンポーネント削除処理
	virtual void Delete() {}
private:
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
};

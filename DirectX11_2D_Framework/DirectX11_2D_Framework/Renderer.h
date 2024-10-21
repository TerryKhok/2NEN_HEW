#pragma once


//テクスチャの指定がなかった時の代用テクスチャ
//============================================================================
#define DEFUALT_TEXTURE_FILEPATH L"asset/pic/noTextureImg.png"
//============================================================================

class Renderer : public Component
{
	friend class GameObject;
	friend class Animator;
private:
	//生成禁止
	Renderer(GameObject* _pObject);
	Renderer(GameObject* _pObject,const wchar_t* _texpath);
	Renderer(GameObject* _pObject,Animator* _animator);
	//削除禁止
	~Renderer() = default;
	//アクティブ変更
	void SetActive(bool _active);
	//対応したノードの削除
	void Delete();
	//レイヤーの変更
	void SetLayer(const LAYER _layer);
	//Debug用のポインター取得
	Renderer* GetDevide(){
		return this;
	}
	//UVRenderNodeへ切り替える
	void SetUVRenderNode(Animator* _animator);
public:
	void SetTexture(const wchar_t* _texPath);
	void SetColor(XMFLOAT4 _color);
	void SetTexcode(int _splitX, int _splitY, int _frameX, int _frameY);
private:
	//対応したノード
	std::shared_ptr<RenderNode> m_node;
};

//双方向リストノード
class RenderNode
{
	friend class Renderer;
	friend class RenderManager;
	friend class Box2DBody;

	//描画関連
protected:
	RenderNode();
	RenderNode(const wchar_t* _texpath);
	//アクティブを切り替える
	virtual void Active(bool _active);
	//描画関数の実行
	virtual void Excute() { (this->*pDrawFunc)(); }
	//描画して次につなぐ
	virtual inline void Draw();
	//なにもせずに次につなぐ
	void VoidNext() { NextFunc(); }
	//テクスチャーの設定
	void SetTexture(const wchar_t* _texpath);
private:
	//描画関数ポインター
	void(RenderNode::* pDrawFunc)(void) = nullptr;
protected:
	GameObject* m_object = nullptr;
	XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f };
	ComPtr<ID3D11ShaderResourceView> m_pTextureView = nullptr;

	//リスト関連
protected:
	//関数ポインターの実行
	void NextFunc() { (this->*pFunc)(); }
	//リストの次を設定する
	void NextContinue() { pFunc = &RenderNode::Continue; }
	//リストの終わりを設定する
	void NextEnd() { pFunc = &RenderNode::End; }
	//次のノードの描画をする
	void Continue() { next->Excute(); }
	//ノードの切れ端
	void End() {}
	//リストからこのインスタンスを削除する
	void Delete();	
	//リストの削除
	inline void DeleteList();
private:
	//リストを繋ぐ関数ポインター
	void(RenderNode::* pFunc)() = nullptr;
	//リストポインタ
	std::shared_ptr<RenderNode> back = nullptr;
	std::shared_ptr<RenderNode> next = nullptr;
};

class UVRenderNode : public RenderNode
{
	friend class Renderer;
	friend class AnimationClip;
	friend class AnimationClipLoop;

private:
	UVRenderNode() {}
	UVRenderNode(const wchar_t* _texPath) :RenderNode(_texPath) {}
private:
	//描画して次につなぐ
	inline void Draw();
private:
	float m_scaleX = 0.5f;
	float m_scaleY = 0.5f;
	int m_frameX = 0;
	int m_frameY = 0;
};

class RenderManager final
{
	friend class Window;
	friend class RenderNode;
	friend class UVRenderNode;
	friend class Renderer;
	friend class RenderManager;
	friend class SceneManager;
	friend class Box2DBody;
	friend class Box2DBoxRenderNode;
	friend class Box2DCapsuleRenderNode;

	using RenderList = std::pair < std::shared_ptr<RenderNode>, std::shared_ptr<RenderNode>>;

	RenderManager() = delete;
private:
	//初期化処理
	static HRESULT Init();
	//新しいリストに入れ替える
	static void GenerateList();
	//リストノードの一斉描画
	static void Draw();
	//リストにノードを追加する
	static void AddRenderList(std::shared_ptr<RenderNode> _node, LAYER _layer);
	//スレッドの現在のリストを次のリストに変更
	static void ChangeNextRenderList();
	//次のノードリストに繋ぐ
	static void LinkNextRenderList();
private:
	// スレッドごとの現在のリスト
	static thread_local RenderList* currentList;
	//リストをロックする
	static std::mutex listMutex;
	//ノードリスト(レイヤーの数だけ増える)
	static RenderList m_rendererList[LAYER::LATER_MAX];
	//次のレンダーリスト（後に上のリストにコピーする）
	static RenderList m_nextRendererList[LAYER::LATER_MAX];
	//共通の頂点バッファー
	static ComPtr<ID3D11Buffer> m_vertexBuffer;
	//共通のインデックスバッファー
	static ComPtr<ID3D11Buffer> m_indexBuffer;
};

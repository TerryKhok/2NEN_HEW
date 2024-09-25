#pragma once


//テクスチャの指定がなかった時の代用テクスチャ
//============================================================================
#define DEFUALT_TEXTURE_FILEPATH L"asset/pic/noTextureImg.png"
//============================================================================


class Renderer : public Component
{
	friend class GameObject;
private:
	Renderer(GameObject* _pObject);
	Renderer(GameObject* _pObject,const wchar_t* texpath);
public:
	//対応したノードの削除
	void Delete();
private:
	//対応したノード
	std::shared_ptr<RenderNode> m_node;
};

//双方向リストノード
class RenderNode
{
	friend class Renderer;
	friend class RenderManager;

	//描画関連
private:
	RenderNode();
	RenderNode(const wchar_t* texpath);
	inline void Draw();
private:
	GameObject* m_object = nullptr;
	XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f };
	ComPtr<ID3D11ShaderResourceView> m_pTextureView = nullptr;

	//リスト関連
public:
	//関数ポインターの実行
	void Excute() { (this->*pFunc)(); }
	//リストを繋ぐ関数
	void(RenderNode::* pFunc)() = nullptr;
	//リストの次を設定する
	void NextContinue() { pFunc = &RenderNode::Continue; }
	//リストの終わりを設定する
	void NextEnd() { pFunc = &RenderNode::End; }
	//次のノードの描画をする
	void Continue() { next->Draw(); }
	//ノードの切れ端
	void End() {}
	//リストからこのインスタンスを削除する
	void Delete();
private:
	//リストポインタ
	std::shared_ptr<RenderNode> back = nullptr;
	std::shared_ptr<RenderNode> next = nullptr;
};

class RenderManager
{
	friend class Window;
	friend class RenderNode;
	friend class Renderer;

	RenderManager() = delete;
private:
	//初期化処理
	static HRESULT Init();
	//リストノードの一斉描画
	static void Draw();
	//リストにノードを追加する
	static void AddRenderList(std::shared_ptr<RenderNode> _node, LAYER _layer);
private:
	//ノードリスト(レイヤーの数だけ増える)
	static std::pair < std::shared_ptr<RenderNode>, std::shared_ptr<RenderNode>> m_rendererList[LAYER::LATER_MAX];
	//共通の頂点バッファー
	static ComPtr<ID3D11Buffer> m_vertexBuffer;
	//共通のインデックスバッファー
	static ComPtr<ID3D11Buffer> m_indexBuffer;
	//指定なしのテクスチャビュー
	static ComPtr<ID3D11ShaderResourceView> m_pTextureView;
};

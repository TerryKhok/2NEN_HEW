#pragma once


class Renderer : public Component
{
	friend class GameObject;
	friend class Animator;
	
private:
	//生成禁止
	Renderer(GameObject* _pObject);
	Renderer(GameObject* _pObject,const wchar_t* _texpath);
	Renderer(GameObject* _pObject,Animator* _animator);
	//デシリアライズ用
	Renderer(GameObject* _pObject, SERIALIZE_INPUT& ar);
	~Renderer() = default;
	//アクティブ変更
	void SetActive(bool _active);
	//対応したノードの削除
	void Delete();
	//UVRenderNodeへ切り替える
	void SetUVRenderNode(Animator* _animator);
	//シリアライズ
	void Serialize(SERIALIZE_OUTPUT& ar) override;
	//imGuiの描画
	void DrawImGui(ImGuiApp::HandleUI& _handle) override;
public:
	//レイヤーの変更
	void SetLayer(const LAYER _layer);
	//テクスチャの変更
	void SetTexture(const wchar_t* _texPath);
	void SetTexture(const std::string& _filePath);
	//色の変更
	void SetColor(XMFLOAT4 _color);
	//uv座標の変更(ちょっと重い)
	void SetUV(int _splitX, int _splitY, int _frameX, int _frameY);
private:
	//対応したノード
	std::shared_ptr<RenderNode> m_node;
	//描画する順番
	LAYER m_layer = LAYER::LAYER_01;
//private:
//	template<class Archive>
//	void save(const Archive& archive) const
//	{
//		archive(CEREAL_NVP(m_node), CEREAL_NVP(m_layer));
//	}
//
//	template<class Archive>
//	void load(const Archive& archive) const
//	{
//		archive(CEREAL_NVP(m_node), CEREAL_NVP(m_layer));
//		RenderManager::AddRenderList(m_node, m_layer);
//	}
};


//双方向リストノード
class RenderNode
{
	friend class Renderer;
	friend class RenderManager;
	friend class Box2DBody;
	friend class Animator;

	//描画関連
protected:
	RenderNode();
	RenderNode(const wchar_t* _texpath);
	//アクティブを切り替える
	void Active(bool _active);
	//描画関数の実行
	virtual void Execute() { (this->*pDrawFunc)(); }
	//描画して次につなぐ
	virtual inline void Draw();
	//なにもせずに次につなぐ
	void VoidNext() { NextFunc(); }
	//テクスチャーの設定
	void SetTexture(const wchar_t* _texpath);
	//テクスチャーの設定
	void SetTexture(const std::string& _filePath);
private:
	//描画関数ポインター
	void(RenderNode::* pDrawFunc)(void) = &RenderNode::Draw;
protected:
	GameObject* m_object = nullptr;
	DirectX::XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f };
	//テクスチャパス
	bool active = false;
	std::wstring texPath = L"null";
	ComPtr<ID3D11ShaderResourceView> m_pTextureView = nullptr;

	//リスト関連
protected:
	//関数ポインターの実行
	void NextFunc() { (this->*pConnectFunc)(); }
	//リストの次を設定する
	void NextContinue() { pConnectFunc = &RenderNode::Continue; }
	//リストの終わりを設定する
	void NextEnd() { pConnectFunc = &RenderNode::End; }
	//次のノードの描画をする
	void Continue() { next->Execute(); }
	//ノードの切れ端
	void End() {}
	//リストからこのインスタンスを削除する
	void Delete(LAYER _nodeLayer);
	//リストの削除
	inline void DeleteList();
private:
	//リストを繋ぐ関数ポインター
	void(RenderNode::* pConnectFunc)() = &RenderNode::End;
	//リストポインタ
	std::shared_ptr<RenderNode> back = nullptr;
	std::shared_ptr<RenderNode> next = nullptr;
private:
	//template<class Archive>
	//void serialize(Archive& ar) 
	//{
	//	ar(CEREAL_NVP(m_color), CEREAL_NVP(active)/*,CEREAL_NVP(texPath)*/);
	//}

	template <class Archive>
	void save(Archive& archive) const 
	{
		std::string path = wstring_to_string(texPath);
		archive(CEREAL_NVP(active), CEREAL_NVP(m_color), CEREAL_NVP(path));
	}

	template<class Archive>
	void load(Archive& archive)
	{
		std::string path;
		archive(CEREAL_NVP(active), CEREAL_NVP(m_color), CEREAL_NVP(path));
		Active(active);
		if (path != "null")
		{
			texPath = string_to_wstring(path);
			SetTexture(texPath.c_str());
		}
	}

	friend class cereal::access;
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
private:
	/*template<class Archive>
	void serialize(Archive& ar) const
	{
		ar(CEREAL_NVP(m_scaleX), CEREAL_NVP(m_scaleY), CEREAL_NVP(m_frameX), CEREAL_NVP(m_frameY));
	}*/

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(CEREAL_NVP(m_scaleX), CEREAL_NVP(m_scaleY), CEREAL_NVP(m_frameX), CEREAL_NVP(m_frameY));
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(CEREAL_NVP(m_scaleX), CEREAL_NVP(m_scaleY), CEREAL_NVP(m_frameX), CEREAL_NVP(m_frameY));
	}

	friend class cereal::access;
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(RenderNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(RenderNode, UVRenderNode)

class RenderManager final
{
	friend class Window;
	friend class RenderNode;
	friend class UVRenderNode;
	friend class Renderer;
	friend class RenderManager;
	friend class SceneManager;
	friend class Box2D::WorldManager;
	friend class Box2DBody;
	friend class Box2DBoxRenderNode;
	friend class Box2DCapsuleRenderNode;
	friend class ImGuiApp;

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
	//メインカメラの行列変換をセットする
	static void SetMainCameraMatrix();
public:
	//ウィンドウ全体の位置
	static Vector2 renderOffset;
	//ウィンドウ全体の拡大率
	static Vector2 renderZoom;
private:
	// スレッドごとの現在のリスト
	static thread_local RenderList* currentList;
	//リストをロックする
	static std::mutex listMutex;
	//ノードリスト(レイヤーの数だけ増える)
	static RenderList m_rendererList[LAYER::LAYER_MAX];
	//次のレンダーリスト（後に上のリストにコピーする）
	static RenderList m_nextRendererList[LAYER::LAYER_MAX];
	//共通の頂点バッファー
	static ComPtr<ID3D11Buffer> m_vertexBuffer;
	//共通のインデックスバッファー
	static ComPtr<ID3D11Buffer> m_indexBuffer;
#ifdef DEBUG_TRUE
	struct DrawRayNode
	{
		Vector2 center;
		float length;
		float radian;
		XMFLOAT4 color;
	};
	//当たり判定の描画
	static bool drawHitBox;
	//rayの描画
	static bool drawRay;
	//rayを描画するための線の頂点
	static ComPtr<ID3D11Buffer> m_lineVertexBuffer;
	//共通のRayを描画するための線のインデックス
	static ComPtr<ID3D11Buffer> m_lineIndexBuffer;
	//rayを描画するための要素
	static std::vector<DrawRayNode> m_drawRayNode;
#endif
};

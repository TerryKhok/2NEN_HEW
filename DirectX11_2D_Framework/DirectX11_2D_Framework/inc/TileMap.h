#pragma once

class TileRenderNode;

class TileMap : public Component
{
	friend class GameObject;

	TileMap(GameObject* _object);
	TileMap(GameObject* _object, SERIALIZE_INPUT& ar);
	//アクティブ変更
	void SetActive(bool _active) override;
	void Delete() override;
	void Serialize(SERIALIZE_OUTPUT& ar) override;
	void DrawImGui(ImGuiApp::HandleUI& _handle) override;
public:
	//レイヤーの変更
	void SetLayer(const LAYER _layer);
	//レイヤーの取得
	const LAYER GetLayer() const { return m_layer; }
	//タイルの追加
	void AddTileTip(const std::wstring& _path, Vector2 _pos);
	//タイルの削除
	void EraseTileTip(Vector2 _pos);
private:
	std::shared_ptr<RenderNode> m_renderNode;
	TileRenderNode* m_node;
	LAYER m_layer;
};

struct TileTip
{
	int texIndex;
	Vector2 pos;

	// operator== を定義
	bool operator==(const Vector2& other) const {
		return pos.x == other.x && pos.y == other.y;
	}

	template<class Archive>
	void save(Archive& ar) const
	{
		ar(CEREAL_NVP(texIndex), CEREAL_NVP(pos));
	}

	template<class Archive>
	void load(Archive& ar)
	{
		ar(CEREAL_NVP(texIndex), CEREAL_NVP(pos));
	}
};

struct TileTextureData
{
	std::wstring path;
	ComPtr<ID3D11ShaderResourceView> texture;
	int count = 0;

	// operator== を定義
	bool operator==(const std::wstring& other) const {
		return path == other;
	}

	template<class Archive>
	void save(Archive& ar) const
	{
		std::string pathStr = wstring_to_string(path);
		ar(CEREAL_NVP(pathStr), CEREAL_NVP(count));
	}

	template<class Archive>
	void load(Archive& ar)
	{
		std::string pathStr;
		ar(CEREAL_NVP(pathStr), CEREAL_NVP(count));
		path = string_to_wstring(pathStr);
		TextureAssets::pLoadTexture(texture, path.c_str());
	}
};


class TileRenderNode : public RenderNode
{
	friend class TileMap;

	void Draw() override;
private:
	std::vector<TileTip> tileTips;
	std::vector<TileTextureData> textureList;

	Vector2 tileSize = { 5.0f, 5.0f };
private:
	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::base_class<RenderNode>(this));  // Save base class data
		archive(CEREAL_NVP(tileTips), CEREAL_NVP(textureList), CEREAL_NVP(tileSize));
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(cereal::base_class<RenderNode>(this));  // Load base class data
		archive(CEREAL_NVP(tileTips), CEREAL_NVP(textureList), CEREAL_NVP(tileSize));
	}

	friend class cereal::access;
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(TileRenderNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(RenderNode, TileRenderNode)


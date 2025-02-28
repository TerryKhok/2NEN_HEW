#pragma once


struct ParticleData
{
	bool texEnable = false;
	std::wstring texPath;
	Vector2 vec;
	Vector2 offset;
	Vector2 scale;
	Vector2 scaling;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float stayTime;
	bool world = false;
};

class TransformRenderNode : public RenderNode
{
	friend class Particle;
	friend class Canvas;

protected:
	TransformRenderNode() : RenderNode(){}
	TransformRenderNode(const wchar_t* _texpath) : RenderNode(_texpath) {}

	virtual void Draw() override;
protected:
	Vector2 scale = { DEFAULT_OBJECT_SIZE,DEFAULT_OBJECT_SIZE };
	float angle = 0.0f;

private:
	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::base_class<RenderNode>(this));
		archive(CEREAL_NVP(scale), CEREAL_NVP(angle));
	}

	template<class Archive>
	void load(Archive& archive)
	{
		archive(cereal::base_class<RenderNode>(this));
		archive(CEREAL_NVP(scale), CEREAL_NVP(angle));
	}

	friend class cereal::access;
};

// Register the types with Cereal
CEREAL_REGISTER_TYPE(TransformRenderNode)
CEREAL_REGISTER_POLYMORPHIC_RELATION(RenderNode, TransformRenderNode)

class WorldRenderNode : TransformRenderNode
{
	friend class Particle;

	WorldRenderNode(Vector2 _startPos) : TransformRenderNode(),startPos(_startPos) {}
	WorldRenderNode(Vector2 _startPos,const wchar_t* _texpath) : TransformRenderNode(_texpath), startPos(_startPos) {}

	virtual void Draw() override;
private:
	Vector2 startPos;
};

class Particle
{
	friend class ParticleSystem;

	Particle(ParticleData _data, GameObject* _object, LAYER _layer);

	bool Update();
	void Delete(LAYER _layer);
	void Active(bool _active);
private:
	std::shared_ptr<TransformRenderNode> m_node;
	Vector2 m_vec;
	Vector2 m_scaling;
	DirectX::XMFLOAT4 startColor = {1.0f,1.0f,1.0f,1.0f};
	DirectX::XMFLOAT4 endColor = { 0.0f,0.0f,0.0f,0.0f };
	float stayTime = 1.0f;
	float time = 0.0f;
};


class ParticleSystem : public Component
{
private:
	void Start() override;
	void CreateParticle();
	void Update() override;
	void Delete() override;
	void SetActive(bool _active) override;
	void DrawImGui(ImGuiApp::HandleUI& _handle) override;
public:
	void Play();
	void Pause();
	void Resume();
	void SetTexture(const wchar_t* _texPath)
	{
		texPath = _texPath;
	}
	void SetCircleShotVec(int _segNum, float _speed);
	void ClearShotVec() {shotVec.clear();}
	void SetMinOffset(Vector2 _minOffset);
	void SetMaxOffset(Vector2 _maxOffset);
	void SetMinScale(Vector2 _minScale);
	void SetMaxScale(Vector2 _maxScale);
private:
	void IntervalCreate();
	void UpdateParticle();
	void(ParticleSystem::*pUpdateFunc)() = &ParticleSystem::IntervalCreate;
private:
	LAYER m_layer = LAYER_01;
	std::wstring texPath = L"null";
	std::vector<Vector2> shotVec;
	Vector2 minVec = { 0.0f,0.0f };
	Vector2 maxVec = { 0.0f,0.0f };
	Vector2 minOffset = { 0.0f,0.0f };
	Vector2 maxOffset = { 0.0f,0.0f };
	Vector2 minScale = {1.0f,1.0f};
	Vector2 maxScale = {5.0f,5.0f};
	Vector2 scaling;
	XMFLOAT4 colorRange[2] = {
		{1.0f,1.0f,1.0f,1.0f},
		{1.0f,1.0f,1.0f,1.0f}
	};
	float stayTimeRange[2] = { 0.0f,1.0f };
	std::vector<std::unique_ptr<Particle>> m_particles;
	bool m_awake = true;
	float time = 0.0f;
	float interval = 5.0f;
	bool m_isWorld = false;

	void Serialize(SERIALIZE_OUTPUT& ar) override;

	void Deserialize(SERIALIZE_INPUT& ar) override;
};

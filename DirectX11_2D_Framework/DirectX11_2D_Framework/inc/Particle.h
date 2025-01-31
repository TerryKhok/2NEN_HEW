#pragma once

struct ParticleData
{
	std::wstring texPath;
	Vector2 vec;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float stayTime;
};

class Particle
{
	friend class ParticleSystem;

	Particle(ParticleData _data, GameObject* _object, LAYER _layer);
public:
	bool Update();
private:
	std::shared_ptr<RenderNode> m_node;
	Vector2 m_vec;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float stayTime = 1.0f;
	float time = 0.0f;
};


class ParticleSystem : public Component
{
private:
	void Start() override;
	void Update() override;
private:
	ParticleData m_data;
	std::vector<std::unique_ptr<Particle>> m_particles;
	LAYER m_layer;
	bool m_awake = true;
};


Particle::Particle(ParticleData _data, GameObject* _object, LAYER _layer)
{
	m_vec = _data.vec;
	startColor = _data.startColor;
	endColor = _data.endColor;
	stayTime = _data.stayTime;

	RenderNode* node = new RenderNode();
	node->m_object = _object;
	m_node = std::shared_ptr<RenderNode>(node);
	RenderManager::AddRenderList(m_node, _layer);
}

bool Particle::Update()
{
	static float frameTime = 1.0f / UPDATE_FPS;

	float t = time / stayTime;

	// XMFLOAT4 ‚ð XMVECTOR ‚É•ÏŠ·
	XMVECTOR startVec = XMLoadFloat4(&startColor);
	XMVECTOR endVec = XMLoadFloat4(&endColor);

	// ƒŠƒjƒA•âŠÔ
	XMVECTOR resultVec = XMVectorLerp(startVec, endVec, t);

	// Œ‹‰Ê‚ð XMFLOAT4 ‚ÉŠi”[
	XMFLOAT4 result;
	XMStoreFloat4(&result, resultVec);

	m_node->m_color = result;

	m_node->m_offset += m_vec;

	time += frameTime;
	if (time >= frameTime)
	{
		return true;
	}

	return false;
}

void ParticleSystem::Start()
{
	if (m_awake)
	{
		m_particles.emplace_back(new Particle(m_data, m_this, m_layer));
	}
}

void ParticleSystem::Update()
{
	for (auto iter = m_particles.begin(); iter != m_particles.end();)
	{
		bool leave = iter->get()->Update();
		if (leave)
		{
			iter = m_particles.erase(iter);
			continue;
		}

		iter++;
	}
}

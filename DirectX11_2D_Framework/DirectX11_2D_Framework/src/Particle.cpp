
void TransformRenderNode::Draw()
{
	static VSObjectConstantBuffer m_cb;

	auto& transform = m_object->transform;
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	m_cb.world = DirectX::XMMatrixScaling(scale.x, scale.y, transform.scale.z);
	m_cb.world *= DirectX::XMMatrixRotationZ(static_cast<float>(transform.angle.z.Get()) + angle);
	m_cb.world *= DirectX::XMMatrixTranslation(transform.position.x + m_offset.x, transform.position.y + m_offset.y, transform.position.z);
	m_cb.world = DirectX::XMMatrixTranspose(m_cb.world);

	m_cb.color = m_color;

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, m_pTextureView.GetAddressOf());

	m_cb.uvScale = XMFLOAT2(1, 1);
	m_cb.uvOffset = XMFLOAT2(0, 0);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &m_cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);

	//次のポインタにつなぐ
	NextFunc();
}

void WorldRenderNode::Draw()
{
	static VSObjectConstantBuffer m_cb;

	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	m_cb.world = DirectX::XMMatrixScaling(scale.x, scale.y, 0.5f);
	m_cb.world *= DirectX::XMMatrixRotationZ(angle);
	m_cb.world *= DirectX::XMMatrixTranslation(startPos.x + m_offset.x, startPos.y + m_offset.y, 0.5f);
	m_cb.world = DirectX::XMMatrixTranspose(m_cb.world);

	m_cb.color = m_color;

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, m_pTextureView.GetAddressOf());

	m_cb.uvScale = XMFLOAT2(1, 1);
	m_cb.uvOffset = XMFLOAT2(0, 0);

	//行列をシェーダーに渡す
	DirectX11::m_pDeviceContext->UpdateSubresource(
		DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &m_cb, 0, 0);

	DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);


	//次のポインタにつなぐ
	NextFunc();
}

Particle::Particle(ParticleData _data, GameObject* _object, LAYER _layer)
{
	m_vec = _data.vec;
	m_scaling = _data.scaling;
	startColor = _data.startColor;
	endColor = _data.endColor;
	stayTime = _data.stayTime;

	TransformRenderNode* node = nullptr;
	if (_data.world)
	{
		Vector2 startPos = _object->transform.position;
		node =  _data.texEnable ? new WorldRenderNode(startPos,_data.texPath.c_str()) : new WorldRenderNode(startPos);
	}
	else
	{
		node = _data.texEnable ? new TransformRenderNode(_data.texPath.c_str()) : new TransformRenderNode();
		
	}
	
	node->m_offset = _data.offset;
	node->scale = _data.scale;
	node->m_object = _object;
	node->m_color = startColor;
	m_node.reset(node);
	RenderManager::AddRenderList(m_node, _layer);
}

bool Particle::Update()
{
	static float frameTime = 1.0f / UPDATE_FPS;

	float t = time / stayTime;

	// XMFLOAT4 を XMVECTOR に変換
	XMVECTOR startVec = XMLoadFloat4(&startColor);
	XMVECTOR endVec = XMLoadFloat4(&endColor);

	// リニア補間
	XMVECTOR resultVec = XMVectorLerp(startVec, endVec, t);

	// 結果を XMFLOAT4 に格納
	XMFLOAT4 result;
	XMStoreFloat4(&result, resultVec);

	m_node->m_color = result;

	m_node->m_offset += m_vec;

	m_node->scale += m_scaling;

	time += frameTime;
	if (time >= stayTime)
	{
		return true;
	}

	return false;
}

void Particle::Delete(LAYER _layer)
{
	m_node->Delete(_layer);
	m_node.reset();
}

void Particle::Active(bool _active)
{
	m_node->Active(_active);
}

void ParticleSystem::Start()
{
	if (m_awake)
	{
		CreateParticle();
	}
}

void ParticleSystem::CreateParticle()
{
	std::uniform_real_distribution<float> vecDistX(minVec.x, maxVec.x);
	std::uniform_real_distribution<float> vecDistY(minVec.y, maxVec.y);
	std::uniform_real_distribution<float> posDistX(minOffset.x, maxOffset.x);
	std::uniform_real_distribution<float> posDistY(minOffset.y, maxOffset.y);
	std::uniform_real_distribution<float> scaleDistX(minScale.x, maxScale.x);
	std::uniform_real_distribution<float> scaleDistY(minScale.y, maxScale.y);
	std::uniform_real_distribution<float> stayTimeDist(stayTimeRange[0], stayTimeRange[1]);

	auto& rGenerator = GetRandomGenerator();

	bool texEnable = texPath != L"null";
	for (auto& vec : shotVec)
	{
		Vector2 targetVec = vec;
		targetVec.x += vecDistX(rGenerator);
		targetVec.y += vecDistY(rGenerator);
		Vector2 targetPos;
		targetPos.x = posDistX(rGenerator);
		targetPos.y = posDistY(rGenerator);
		Vector2 targetScale;
		targetScale.x = scaleDistX(rGenerator);
		targetScale.y = scaleDistY(rGenerator);
		ParticleData data;
		data.texEnable = texEnable;
		data.texPath = texPath;
		data.vec = targetVec;
		data.offset = targetPos;
		data.scale = targetScale;
		data.scaling = scaling;
		data.startColor = colorRange[0];
		data.endColor = colorRange[1];
		data.stayTime = stayTimeDist(rGenerator);
		data.world = m_isWorld;
		m_particles.emplace_back(new Particle(data, m_this, m_layer));
	}
}

void ParticleSystem::Update()
{
	(this->*pUpdateFunc)();
}

void ParticleSystem::Delete()
{
	for (auto& particle : m_particles)
	{
		particle->Delete(m_layer);
	}

	m_particles.clear();
}

void ParticleSystem::SetActive(bool _active)
{
	for (auto& particle : m_particles)
	{
		particle->Active(_active);
	}
}

void ParticleSystem::DrawImGui(ImGuiApp::HandleUI& _handle)
{
#ifdef DEBUG_TRUE
	if (ImGui::Button("<>##ParticleSystemLayer"))
	{
		ImGui::OpenPopup("SelectLayerParticleSystem");
	}
	ImGui::SameLine();
	ImGui::Text("  layer : %s", magic_enum::enum_name(m_layer).data());

	if (ImGui::BeginPopup("SelectLayerParticleSystem"))
	{
		for (int i = 0; i < LAYER_MAX; i++)
		{
			LAYER layer = (LAYER)i;
			if (layer == LAYER_BOX2D_DEBUG) continue;

			bool same = layer == m_layer;
			if (ImGui::Selectable(magic_enum::enum_name(layer).data(), same))
			{
				if (!same) m_layer = layer;
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::Button("Link##ParticleSystemTex"))
	{
		std::string str = m_this->GetName() + "Texture";
		_handle.SetUploadFile(str, [&](GameObject* obj, std::filesystem::path path)
			{
				if (obj == nullptr || obj != m_this) return;

				auto particleSystem = obj->GetComponent<ParticleSystem>();
				if (particleSystem != nullptr)
				{
					particleSystem->SetTexture(path.wstring().c_str());
				}
			}, { ".png" ,".jpg",".dds" });
	}
	ImGui::SameLine();
	ImGui::Text("path  : %s", wstring_to_string(texPath).c_str());

	ImGui::SeparatorText("ShotVec");

	if (ImGui::Button("Play##Particle"))
	{
		CreateParticle();
	}

	static bool circle = false;
	static int segmentNum = 4;
	static float speed = 1.0f;
	if (ImGui::Button("Add Vec"))
	{
		if (circle)
		{
			SetCircleShotVec(segmentNum, speed);
		}
		else
			shotVec.emplace_back(0.0f, 0.0f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear##Particle"))
	{
		shotVec.clear();
	}

	ImGui::Checkbox("CircleVec##particle", &circle);
	if (circle)
	{
		ImGui::DragInt("Circle##Particle", &segmentNum, 1.0f, 0);
		ImGui::DragFloat("speed##Particle", &speed, 0.1f, 0.0f);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("ShotVec##ParticleSystem", ImVec2(300, 75), ImGuiChildFlags_Borders);
	{
		int i = 0;
		for (auto iter = shotVec.begin(); iter != shotVec.end();)
		{
			i++;
			ImGui::PushID(i);
			if (ImGui::Button("erase"))
			{
				iter = shotVec.erase(iter);
				ImGui::PopID();
				continue;
			}
			ImGui::SameLine();
			ImGui::DragFloat2("vec", iter->data(), 0.01f);
			ImGui::PopID();
			iter++;
		}
		ImGui::EndChild();
	}
	ImGui::PopStyleVar();

	ImGui::SeparatorText("Parameter");
	ImGui::DragFloatRange2("vecX##Particle", &minVec.x, &maxVec.x, 0.01f, -100.0f, 100.0f, "Min: %.2f", "Max: %.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloatRange2("vecY##Particle", &minVec.y, &maxVec.y, 0.01f, -100.0f, 100.0f, "Min: %.2f", "Max: %.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloatRange2("offsetX##Particle", &minOffset.x, &maxOffset.x, 0.1f, -1000.0f, 1000.0f, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloatRange2("offsetY##Particle", &minOffset.y, &maxOffset.y, 0.1f, -1000.0f, 1000.0f, "Min: %.1f", "Max: %.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloatRange2("scaleX##Particle", &minScale.x, &maxScale.x, 0.1f, 0.0f, 100.0f, "Min: %.2f", "Max: %.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloatRange2("scaleY##Particle", &minScale.y, &maxScale.y, 0.1f, 0.0f, 100.0f, "Min: %.2f", "Max: %.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat2("scaling##Particle", scaling.data(), 0.1f);
	ImGui::ColorEdit4("startColor##Particle", &colorRange[0].x);
	ImGui::ColorEdit4("endColor##Particle", &colorRange[1].x);
	ImGui::DragFloatRange2("stayTime##Particle", &stayTimeRange[0], &stayTimeRange[1], 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("interval##Particle", &interval, 0.1f, 0.0f);
	ImGui::Checkbox("awake##Particle", &m_awake);
	bool intervalUpdate = pUpdateFunc == &ParticleSystem::IntervalCreate;
	if (ImGui::Checkbox("autoCrate##Particle", &intervalUpdate))
	{
		pUpdateFunc = intervalUpdate ? &ParticleSystem::IntervalCreate : &ParticleSystem::UpdateParticle;
	}
	ImGui::Checkbox("world##Particle", &m_isWorld);
#endif
}

void ParticleSystem::SetCircleShotVec(int _segNum, float _speed)
{
	float segRad = Math::PI2 / _segNum;
	for (int i = 0; i < _segNum; i++)
	{
		float rad = segRad * i;
		shotVec.emplace_back(cos(rad) * _speed, sin(rad) * _speed);
	}
}

void ParticleSystem::SetMinOffset(Vector2 _minOffset)
{
	minOffset =
	{
		(std::min)(_minOffset.x, maxOffset.x),
		(std::min)(_minOffset.y, maxOffset.y)
	};
}

void ParticleSystem::SetMaxOffset(Vector2 _maxOffset)
{
	maxOffset =
	{
		(std::max)(_maxOffset.x, minOffset.x),
		(std::max)(_maxOffset.y, minOffset.y)
	};
}

void ParticleSystem::SetMinScale(Vector2 _minScale)
{
	minScale =
	{
		(std::min)(_minScale.x, maxScale.x),
		(std::min)(_minScale.y, maxScale.y)
	};
}

void ParticleSystem::SetMaxScale(Vector2 _maxScale)
{
	maxScale =
	{
		(std::max)(_maxScale.x, minScale.x),
		(std::max)(_maxScale.y, minScale.y)
	};
}

void ParticleSystem::IntervalCreate()
{
	for (auto iter = m_particles.begin(); iter != m_particles.end();)
	{
		bool leave = iter->get()->Update();
		if (leave)
		{
			iter->get()->Delete(m_layer);
			iter = m_particles.erase(iter);
			continue;
		}

		iter++;
	}

	time += 1.0f / UPDATE_FPS;
	if (time >= interval)
	{
		time = 0.0f;
		CreateParticle();
	}
}

void ParticleSystem::UpdateParticle()
{
	for (auto iter = m_particles.begin(); iter != m_particles.end();)
	{
		bool leave = iter->get()->Update();
		if (leave)
		{
			iter->get()->Delete(m_layer);
			iter = m_particles.erase(iter);
			continue;
		}

		iter++;
	}
}

void ParticleSystem::Serialize(SERIALIZE_OUTPUT& ar)
{
	bool autoCreate = pUpdateFunc == &ParticleSystem::IntervalCreate;

	std::string path = wstring_to_string(texPath);
	ar(::cereal::make_nvp("layer##particle", m_layer), ::cereal::make_nvp("path##particle", path), CEREAL_NVP(shotVec), CEREAL_NVP(minVec), CEREAL_NVP(maxVec),
		CEREAL_NVP(minOffset), CEREAL_NVP(maxOffset), CEREAL_NVP(minScale), CEREAL_NVP(maxScale),
		CEREAL_NVP(scaling), CEREAL_NVP(colorRange[0]), CEREAL_NVP(colorRange[1]), CEREAL_NVP(stayTimeRange[0]),
		CEREAL_NVP(stayTimeRange[1]), CEREAL_NVP(m_awake), CEREAL_NVP(autoCreate), CEREAL_NVP(interval), CEREAL_NVP(m_isWorld));
}

void ParticleSystem::Deserialize(SERIALIZE_INPUT& ar)
{
	bool autoCreate = false;

	std::string path;
	ar(::cereal::make_nvp("layer##particle", m_layer), ::cereal::make_nvp("path##particle", path), CEREAL_NVP(shotVec), CEREAL_NVP(minVec), CEREAL_NVP(maxVec),
		CEREAL_NVP(minOffset), CEREAL_NVP(maxOffset), CEREAL_NVP(minScale), CEREAL_NVP(maxScale),
		CEREAL_NVP(scaling), CEREAL_NVP(colorRange[0]), CEREAL_NVP(colorRange[1]), CEREAL_NVP(stayTimeRange[0]),
		CEREAL_NVP(stayTimeRange[1]), CEREAL_NVP(m_awake), CEREAL_NVP(autoCreate), CEREAL_NVP(interval), CEREAL_NVP(m_isWorld));

	pUpdateFunc = autoCreate ? &ParticleSystem::IntervalCreate : &ParticleSystem::UpdateParticle;

	texPath = string_to_wstring(path);
}

void ParticleSystem::Play()
{
	CreateParticle();
}

void ParticleSystem::Pause()
{
	pUpdateFunc = &ParticleSystem::UpdateParticle;
}

void ParticleSystem::Resume()
{
	pUpdateFunc = &ParticleSystem::IntervalCreate;
}



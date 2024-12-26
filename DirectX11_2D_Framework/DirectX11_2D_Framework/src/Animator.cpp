

std::shared_ptr<AnimationClip> AnimatorManager::m_commonClip;
long long AnimatorManager::deltaCount;

void AnimationClip::SetUVRenderNode(UVRenderNode* _renderNode)
{
	auto& frame = frames[frameIndex];
	_renderNode->m_pTextureView = textureList[frame.texIndex];
#ifdef DEBUG_TRUE
	_renderNode->texPath = frame.texPath;
#endif
	_renderNode->m_frameX = frame.frameX;
	_renderNode->m_frameY = frame.frameY;
	_renderNode->m_scaleX = frame.scaleX;
	_renderNode->m_scaleY = frame.scaleY;
}

void AnimationClip::Awake(UVRenderNode* _renderNode)
{
	frameIndex = 0;
	nowCount = 0;
	pUpdate = &AnimationClip::UpdateCount;

	SetUVRenderNode(_renderNode);
}

void AnimationClip::UpdateCount(long long _count, UVRenderNode* _renderNode)
{
	nowCount += _count;

	const auto& waitCount = frames[frameIndex].waitCount;
	if (nowCount > waitCount)
	{
		nowCount -= waitCount;
		frameIndex++;

		if (frameIndex >= static_cast<int>(frames.size()))
		{
			pUpdate = &AnimationClip::UpdateVoid;
			return;
		}

		SetUVRenderNode(_renderNode);
	}
}

void AnimationClipLoop::Awake(UVRenderNode* _renderNode)
{
	frameIndex = 0;
	nowCount = 0;
	SetUVRenderNode(_renderNode);
}

void AnimationClipLoop::Update(long long _count, UVRenderNode* _renderNode)
{
	nowCount += _count;

	const auto& waitCount = frames[frameIndex].waitCount;
	if (nowCount > waitCount)
	{
		nowCount -= waitCount;
		frameIndex = (frameIndex + 1) % static_cast<int>(frames.size());

		SetUVRenderNode(_renderNode);
	}
}

Animator::Animator(GameObject* _gameObject)
{
	//プレイするまでなにもしないに設定
	m_currentClip.reset(new AnimationClip());
	Pause();

	auto renderer = _gameObject->GetComponent<Renderer>();
	if (renderer == nullptr)
	{
		_gameObject->AddComponent<Renderer>(this);
		LOG("so we added an Renderer. Never mind the warning above.\n");
		return;
	}

	renderer->SetUVRenderNode(this);
}

void Animator::Update()
{
	(m_currentClip.get()->*pUpdate)(AnimatorManager::deltaCount, m_uvNode);
}

void Animator::SetActive(bool _active)
{
	m_uvNode->Active(_active);
}

void Animator::AddClip(std::string _name, std::string _path, bool _loop)
{
	if (m_clip.find(_name) != m_clip.end()) return;

	//読み込みファイルを開く
	std::ifstream fin;
	fin.open(_path, std::ios::in | std::ios::binary);

	bool open = fin.is_open();
	//読み込み失敗
	if (open)
	{
		AnimationClip* clip = _loop ? new AnimationClipLoop() : new AnimationClip();

		//プレイヤーの数を読み込む
		int size = 0;
		fin.read((char*)&size, sizeof(size));

		for (int i = 0; i < size; i++)
		{
			//パスの大きさを読み込み
			int nameSize = 0;
			fin.read((char*)&nameSize, sizeof(nameSize));

			AnimationFrameData frame;

			//パスを読み込み
			std::string path;
			path.resize(nameSize);
			fin.read((char*)path.c_str(), nameSize);
			std::filesystem::path texPath(path);
			frame.texPath = texPath.wstring();
			//splitX
			int splitX;
			fin.read((char*)&splitX, sizeof(splitX));
			frame.scaleX = 1.0f / splitX;
			//splitY
			int splitY;
			fin.read((char*)&splitY, sizeof(splitY));
			frame.scaleY = 1.0f / splitY;
			//uvX
			fin.read((char*)&frame.frameX, sizeof(frame.frameX));
			//uvY
			fin.read((char*)&frame.frameY, sizeof(frame.frameY));
			//waitCout
			fin.read((char*)&frame.waitCount, sizeof(frame.waitCount));

			clip->AddFrame(frame);
		}

		//読み込みファイルを閉じる
		fin.close();

		m_clip.insert(std::make_pair(_name, clip));
	}
	else
	{
		LOG_ERROR("アニメーションクリップ読み込み失敗");
	}
}

void Animator::Play(const std::string& _clipName)
{
	//再開開始
	Resume();

	auto iter = m_clip.find(_clipName);
	if(iter == m_clip.end())
	{
		LOG_WARNING("not find AnimationClip : %s", _clipName.c_str());
		return;
	}

#ifdef DEBUG_TRUE
	m_currentClipName = _clipName;
#endif

	m_currentClip =iter->second;
	m_currentClip->Awake(m_uvNode);
}

void Animator::Pause()
{
	pUpdate = &AnimationClip::UpdateVoid;
}

void Animator::Resume()
{
	pUpdate = &AnimationClip::Update;
}


void Animator::DrawImGui(ImGuiApp::HandleUI& _handle)
{
#ifdef DEBUG_TRUE
	ImGui::SeparatorText("editor");
	
	if (ImGui::Button("Link##AnimationClip"))
	{
		_handle.SetUploadFile("animation clip file",
			[&](GameObject* _obj, std::filesystem::path _path)
			{
				currentClipPath = _path;
			}
		, { ANIM_CLIP_EXTENSION_DOT });
	}
	ImGui::SameLine();
	ImGui::Text("clip path : %s", currentClipPath.filename().string().c_str());
	
	static char buf[256] = {};
	if (ImGui::Button("-##clear"))
	{
		memset(buf, '\0', strlen(buf));
	}
	ImGui::SetItemTooltip("clear");
	ImGui::SameLine();
	ImGui::InputText("clipName", buf, sizeof(buf) / sizeof(char));
	static bool loop = true;
	ImGui::Checkbox("loop", &loop);
	if (ImGui::Button("+Add Clip"))
	{
		AddClip(buf, currentClipPath.string(), loop);
	}

	ImGui::SeparatorText("clipList");

	if (ImGui::Button("Pause"))
	{
		Pause();
	}
	ImGui::SameLine();
	if (ImGui::Button("Resume"))
	{
		Resume();
	}
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	float windowSize = (std::min)(5, (int)m_clip.size()) * 15.0f + 20.0f;
	ImGui::BeginChild("ChildR", ImVec2(0, windowSize), ImGuiChildFlags_Borders);
	{
		for (auto clip : m_clip)
		{
			bool selected = clip.first == m_currentClipName;
			if (ImGui::Selectable(clip.first.c_str(), &selected))
			{
				Play(clip.first);
			}
		}
		ImGui::EndChild();
	}
	ImGui::PopStyleVar();

#endif
}


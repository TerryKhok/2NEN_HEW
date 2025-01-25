

TileMap::TileMap(GameObject* _object)
{
	m_layer = LAYER_BG;
	m_node = new TileRenderNode();
	m_node->m_object = _object;
	m_renderNode.reset(m_node);
	RenderManager::AddRenderList(m_renderNode, m_layer);
}

TileMap::TileMap(GameObject* _object, SERIALIZE_INPUT& ar)
{
	ar(CEREAL_NVP(m_renderNode), CEREAL_NVP(m_layer));
	m_renderNode->m_object = _object;
	m_node = dynamic_cast<TileRenderNode*>(m_renderNode.get());
	RenderManager::AddRenderList(m_renderNode, m_layer);
}

void TileMap::SetActive(bool _active)
{
	m_renderNode->Active(_active);
}

void TileMap::Delete()
{
	m_node->Delete(m_layer);
}

void TileMap::Serialize(SERIALIZE_OUTPUT& ar)
{
	ar(CEREAL_NVP(m_renderNode), CEREAL_NVP(m_layer));
}

void TileMap::DrawImGui(ImGuiApp::HandleUI& _handle)
{
#ifdef DEBUG_TRUE
	if (ImGui::Button("<>##LayerTileMap"))
	{
		ImGui::OpenPopup("SelectLayer");
	}
	ImGui::SameLine();
	ImGui::Text("  layer : %s", magic_enum::enum_name(m_layer).data());

	ImGui::ColorEdit4("color##tileMap", &m_renderNode->m_color.x);

	if (ImGui::BeginPopup("SelectLayer"))
	{
		for (int i = 0; i < LAYER_MAX; i++)
		{
			LAYER layer = (LAYER)i;
			if (layer == LAYER_BOX2D_DEBUG) continue;

			bool same = layer == m_layer;
			if (ImGui::Selectable(magic_enum::enum_name(layer).data(), same))
			{
				if (!same)
				{
					m_node->Delete(m_layer);
					RenderManager::AddRenderList(m_renderNode, layer);
					m_layer = layer;
				}
			}
		}

		ImGui::EndPopup();
	}

	bool editTileMap = _handle.DrawLockButton("editTileMap");
	/*if (ImGui::Checkbox("EditLock##tileMap", &editTileMap))
	{
		_handle.LockHandle(editTileMap, "editTileMap");
	}*/
	if (editTileMap)
	{
		Vector2 tileSize = m_node->tileSize * DEFAULT_OBJECT_SIZE;
		ImGui::DragFloat2("CellSize", m_node->tileSize.data(), 0.1f, 1.0f, 192.0f, "%.2f");
		
		static std::map<std::wstring,ComPtr<ID3D11ShaderResourceView>> workTextures;
		static std::wstring_view selectTexPath;
		if (ImGui::Button("Link##Texture"))
		{
			_handle.SetUploadFile("TileTexture", [&](GameObject* obj, std::filesystem::path path)
				{
					if (obj == nullptr || obj != m_this) return;

					ComPtr<ID3D11ShaderResourceView> tex;
					TextureAssets::pLoadTexture(tex, path.wstring().c_str());
					workTextures.emplace(path.wstring(),tex);
				}, { ".png" ,".jpg",".dds" }
			);
		}
		static std::wstring erasePath = L"Erase";
		ImGui::SameLine();
		if (ImGui::Button("Erase##texPath"))
		{
			auto iter = workTextures.find(std::wstring(selectTexPath));
			if (iter != workTextures.end())
			{
				workTextures.erase(iter);
				selectTexPath = erasePath;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Load##texPath"))
		{
			for (auto& tex : m_node->textureList)
			{
				auto iter = workTextures.find(tex.path);
				if (iter == workTextures.end())
				{
					workTextures.emplace(tex.path, tex.texture);
				}
			}
		}

		ImGui::SeparatorText("Tile");

		bool eraseTile = selectTexPath == erasePath.c_str();

		if (ImGui::BeginChild("Tile",ImVec2(0,300)))
		{
			if (eraseTile)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.067f, 0.067f, 0.067f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.7f, 1.0f));
			}
			if (ImGui::ImageButton("erase", ImTextureID(DirectX11::m_pTextureView.Get()),ImVec2(75,75),ImVec2(0,0),ImVec2(1,1),
				ImVec4(0,0,0,0),ImVec4(1,1,1,1.0f)))
			{
				selectTexPath = erasePath;
			}
			ImGui::SetItemTooltip("Eraser");
			ImGui::PopStyleColor(2);

			int count = 1;
			for (auto& tex : workTextures)
			{
				ImGui::PushID(count);
				if (count % 5 != 0) ImGui::SameLine();
				bool selected = selectTexPath == tex.first.c_str();
				if (selected)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f,1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.067f, 0.067f, 0.067f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.7f, 1.0f));
				}
				if (ImGui::ImageButton("textures", ImTextureID(tex.second.Get()), ImVec2(75, 75), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.067f, 0.067f, 0.067f, 1.0f)))
				{
					selectTexPath = tex.first;
				}
				ImGui::PopStyleColor(2);
				ImGui::PopID();
				count++;
			}
			ImGui::EndChild();
		}

		Vector2 worldPos = Input::Get().MousePoint();
		worldPos.x = worldPos.x * DISPALY_ASPECT_WIDTH / RenderManager::renderZoom.x + RenderManager::renderOffset.x;
		worldPos.y = worldPos.y * DISPALY_ASPECT_HEIGHT / RenderManager::renderZoom.y + RenderManager::renderOffset.y;

		static int currentIndex[2] = { 0,0 };
		if (Input::Get().MouseLeftTrigger())
		{
			int indexX = (int)std::round((worldPos.x - m_this->transform.position.x) / tileSize.x);
			int indexY = (int)std::round((worldPos.y - m_this->transform.position.y) / tileSize.y);

			if (eraseTile)
			{
				Vector2 pos = { indexX * DEFAULT_OBJECT_SIZE,indexY * DEFAULT_OBJECT_SIZE };
				EraseTileTip(pos);
			}
			else if (!selectTexPath.empty())
			{
				Vector2 pos = { indexX * DEFAULT_OBJECT_SIZE,indexY * DEFAULT_OBJECT_SIZE };
				std::wstring path(selectTexPath);
				AddTileTip(path, pos);
			}

			currentIndex[0] = indexX;
			currentIndex[1] = indexY;
		}
		if (Input::Get().MouseLeftPress())
		{
			int indexX = (int)std::round((worldPos.x - m_this->transform.position.x) / tileSize.x);
			int indexY = (int)std::round((worldPos.y - m_this->transform.position.y) / tileSize.y);
			if (eraseTile)
			{
				Vector2 pos = { indexX * DEFAULT_OBJECT_SIZE,indexY * DEFAULT_OBJECT_SIZE };
				EraseTileTip(pos);
			}
			else if (!selectTexPath.empty())
			{
				if (currentIndex[0] != indexX || currentIndex[1] != indexY)
				{
					Vector2 pos = { indexX * DEFAULT_OBJECT_SIZE,indexY * DEFAULT_OBJECT_SIZE };
					std::wstring path(selectTexPath);
					AddTileTip(path, pos);

					currentIndex[0] = indexX;
					currentIndex[1] = indexY;
				}
			}
		}

		if (Input::Get().MouseMiddleTrigger())
		{
			int indexX = (int)std::round((worldPos.x - m_this->transform.position.x) / tileSize.x);
			int indexY = (int)std::round((worldPos.y - m_this->transform.position.y) / tileSize.y);

			Vector2 pos = { indexX * DEFAULT_OBJECT_SIZE,indexY * DEFAULT_OBJECT_SIZE };

			auto& tipList = m_node->tileTips;
			auto iter = std::find(tipList.begin(), tipList.end(), pos);
			if (iter != tipList.end())
			{
				auto& data = m_node->textureList[iter->texIndex];
				auto iter = workTextures.find(data.path);
				if (iter == workTextures.end())
				{
					workTextures.emplace(data.path, data.texture);
					selectTexPath = data.path;
				}
				else
				{
					selectTexPath = iter->first;
				}
			}
		}


		float displayX = SCREEN_WIDTH / RenderManager::renderZoom.x;
		float displayY = SCREEN_HEIGHT / RenderManager::renderZoom.y;
		int segmentX = static_cast<int>(displayX / tileSize.x) / 2 + 1;
		int segmentY = static_cast<int>(displayY / tileSize.y) / 2 + 1;

		RenderManager::DrawRayNode rayNode;
		rayNode.center = RenderManager::renderOffset;
		float offsetX = RenderManager::renderOffset.x - m_this->transform.position.x;
		offsetX = offsetX - (int)(offsetX / tileSize.x) * tileSize.x;
		float startX = rayNode.center.x - offsetX - tileSize.x / 2;
		rayNode.length = displayY;
		rayNode.radian = Math::hPI;
		rayNode.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.1f);
		for (int x = -segmentX; x <= segmentX; x++)
		{
			rayNode.center.x = startX + tileSize.x * x;
			RenderManager::m_drawRayNode.push_back(rayNode);
		}
		rayNode.center = RenderManager::renderOffset;
		float offsetY = RenderManager::renderOffset.y - m_this->transform.position.y;
		offsetY = offsetY - (int)(offsetY / tileSize.y) * tileSize.y;
		float startY = rayNode.center.y - offsetY - tileSize.y / 2;
		rayNode.length = displayX;
		rayNode.radian = 0.0f;
		for (int y = -segmentY; y <= segmentY; y++)
		{
			rayNode.center.y = startY + tileSize.y * y;
			RenderManager::m_drawRayNode.push_back(rayNode);
		}
		
	}
#endif
}

void TileMap::SetLayer(const LAYER _layer)
{
	if (m_layer == _layer) return;

	m_node->Delete(m_layer);
	RenderManager::AddRenderList(m_renderNode, _layer);
	m_layer = _layer;
}

void TileMap::AddTileTip(const std::wstring& _path, Vector2 _pos)
{
	auto& list = m_node->textureList;
	auto iter = std::find(list.begin(), list.end(), _path);
	TileTip tip;
	tip.pos = _pos;
	if (iter == list.end())
	{
		ComPtr<ID3D11ShaderResourceView> texture;
		TextureAssets::pLoadTexture(texture, _path.c_str());
		m_node->textureList.emplace_back(
			_path, texture, 1
		);
		tip.texIndex = (int)list.size() - 1;
	}
	else
	{
		iter->count++;
		tip.texIndex = (int)std::distance(list.begin(), iter);
	}

	auto& tipList = m_node->tileTips;
	auto it = std::find(tipList.begin(), tipList.end(), _pos);
	if (it != tipList.end())
	{
		it->texIndex = tip.texIndex;
		return;
	}

	m_node->tileTips.push_back(std::move(tip));
}

void TileMap::EraseTileTip(Vector2 _pos)
{
	auto& tipList = m_node->tileTips;
	auto iter = std::find(tipList.begin(), tipList.end(), _pos);
	if (iter == tipList.end())return;

	m_node->textureList[iter->texIndex].count--;
	if (m_node->textureList[iter->texIndex].count <= 0)
	{
		m_node->textureList.erase(m_node->textureList.begin() + iter->texIndex);
	}

	tipList.erase(iter);
}



void TileRenderNode::Draw()
{
	static VSObjectConstantBuffer cb;
	cb.color = m_color;
	cb.uvScale = XMFLOAT2(1, 1);
	cb.uvOffset = XMFLOAT2(0, 0);

	auto& transform = m_object->transform;

	DirectX::XMMATRIX worldMat;
	//ワールド変換行列の作成
	//ー＞オブジェクトの位置・大きさ・向きを指定
	worldMat = DirectX::XMMatrixScaling(tileSize.x, tileSize.y, transform.scale.z);
	worldMat *= DirectX::XMMatrixRotationZ(static_cast<float>(transform.angle.z.Get()));

	for (auto& tip : tileTips)
	{
		cb.world = worldMat;

		cb.world *= DirectX::XMMatrixTranslation(transform.position.x + tip.pos.x * tileSize.x,
			transform.position.y + tip.pos.y * tileSize.y, transform.position.z);
		cb.world = DirectX::XMMatrixTranspose(cb.world);

		//テクスチャをピクセルシェーダーに渡す
		DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, textureList[tip.texIndex].texture.GetAddressOf());

		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);
	}

#ifdef DEBUG_TRUE
	bool flg = true;
	if (m_object->isSelected != GameObject::SELECT_NONE)
	{
		Box2DBody* rb = nullptr;
		flg = m_object->TryGetComponent(&rb);
		if (flg)
		{
			flg = !rb->m_shapeList.empty();
		}
	}
	if (!flg)
	{
		UINT strides = sizeof(Vertex);
		UINT offsets = 0;

		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_lineBoxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		//テクスチャをピクセルシェーダーに渡す
		DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

		//ワールド変換行列の作成
		//ー＞オブジェクトの位置・大きさ・向きを指定
		cb.world = DirectX::XMMatrixScaling(m_object->transform.scale.x + 1.0f, m_object->transform.scale.y + 1.0f, 1.0f);
		cb.world *= DirectX::XMMatrixRotationZ(static_cast<float>(m_object->transform.angle.z.Get()));
		cb.world *= DirectX::XMMatrixTranslation(m_object->transform.position.x, m_object->transform.position.y, 0.0f);
		cb.world = DirectX::XMMatrixTranspose(cb.world);
		if (m_object->isSelected == GameObject::SELECTED)
			cb.color = Box2D::b2_colorSelected;
		else if (m_object->isSelected == GameObject::ON_MOUSE)
			cb.color = Box2D::b2_colorOnMouse;

		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(8, 0, 0);

		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
#endif

	//次のポインタにつなぐ
	NextFunc();
}

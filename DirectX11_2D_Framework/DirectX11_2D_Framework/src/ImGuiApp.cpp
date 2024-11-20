#include"../../DirectX11_2D_Framework/img/icon_sheet_32x32.c"

#ifdef DEBUG_TRUE

int ImGuiApp::worldFpsCounter;
int ImGuiApp::updateFpsCounter = 0;


//window関連
HWND ImGuiApp::m_hWnd[TYPE_MAX];
ImGuiContext* ImGuiApp::context[TYPE_MAX];
std::unordered_map<HWND, ImGuiContext*> ImGuiApp::m_hWndContexts;
ComPtr<IDXGISwapChain> ImGuiApp::m_pSwapChain[TYPE_MAX];
ComPtr<ID3D11RenderTargetView> ImGuiApp::m_pRenderTargetView[TYPE_MAX];
ComPtr<ID3D11ShaderResourceView> ImGuiApp::m_pIconTexture;
ImTextureID ImGuiApp::m_imIconTexture;
void(*ImGuiApp::pDrawImGui[ImGuiApp::TYPE_MAX])() = {};

void ImGuiSetKeyMap(ImGuiContext* _imguiContext);

std::vector<const char*> filterNames;

constexpr long long numFilter = magic_enum::enum_count<FILTER>() - 1;

ImVec4 windowBgCol;

HRESULT ImGuiApp::Init(HINSTANCE hInstance)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();

	// ウィンドウクラス情報をまとめる
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "GUI_WINDOW";
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	Vector2 windowPos[TYPE_MAX];
	windowPos[OPTIONS] = { Window::MONITER_HALF_WIDTH / -1.35f,0 };
	windowPos[INSPECTER] = { Window::MONITER_HALF_WIDTH / 1.35f,0 };

	// Use std::fill to set all elements to the same function
	std::fill(std::begin(pDrawImGui), std::end(pDrawImGui), []() {});
	pDrawImGui[OPTIONS] = DrawOptionGui;
	pDrawImGui[INSPECTER] = DrawInspectorGui;

	for (int type = 0; type < TYPE_MAX; type++)
	{
		HWND hWnd = CreateWindowEx(
			0,										// 拡張ウィンドウスタイル
			"GUI_WINDOW",								// ウィンドウクラスの名前
			IMGUI_WINDOW_NAME,							// ウィンドウの名前
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,//WS_OVERLAPPEDWINDOW,	// ウィンドウスタイル
			CW_USEDEFAULT,							// ウィンドウの左上Ｘ座標
			CW_USEDEFAULT,							// ウィンドウの左上Ｙ座標 
			IMGUI_WINDOW_WIDTH,							// ウィンドウの幅
			IMGUI_WINDOW_HEIGHT,							// ウィンドウの高さ
			NULL,									// 親ウィンドウのハンドル
			NULL,									// メニューハンドルまたは子ウィンドウID
			hInstance,								// インスタンスハンドル
			NULL								// ウィンドウ作成データ
		);

		// After creating the window, modify the system menu to remove the close button
		HMENU hMenu = GetSystemMenu(hWnd, FALSE);
		if (hMenu != NULL) {
			// Disable the close button by removing the SC_CLOSE item from the system menu
			RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}

		//ウィンドウのサイズを修正
		RECT rc1, rc2;
		GetWindowRect(hWnd, &rc1);
		GetClientRect(hWnd, &rc2);
		int sx = IMGUI_WINDOW_WIDTH;
		int sy = IMGUI_WINDOW_HEIGHT;
		sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
		sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, sx, sy, (SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_NOMOVE));

		ShowWindow(hWnd, SW_SHOWDEFAULT);
		// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
		UpdateWindow(hWnd);

		SetWindowPosition(hWnd, windowPos[type]);

		m_hWnd[type] = hWnd;

		// Describe the swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		::ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = IMGUI_WINDOW_WIDTH;
		swapChainDesc.BufferDesc.Height = IMGUI_WINDOW_HEIGHT;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;


		HRESULT hr = DirectX11::m_pDxgiFactory->CreateSwapChain(DirectX11::m_pDevice.Get(), &swapChainDesc, m_pSwapChain[type].GetAddressOf());
		if (FAILED(hr)) {
			return hr;
		}

		// レンダーターゲットビュー作成
		ID3D11Texture2D* renderTarget;
		hr = m_pSwapChain[type]->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
		if (FAILED(hr)) return hr;
		hr = DirectX11::m_pDevice->CreateRenderTargetView(renderTarget, NULL, m_pRenderTargetView[type].GetAddressOf());
		renderTarget->Release();
		if (FAILED(hr)) return hr;

		context[type] = ImGui::CreateContext();
		ImGui::SetCurrentContext(context[type]);

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		m_hWndContexts.insert(std::make_pair(hWnd, context[type]));

		ImGui_ImplWin32_Init(m_hWnd[type]); // hWnd is your main window handle
		ImGui_ImplDX11_Init(DirectX11::m_pDevice.Get(), DirectX11::m_pDeviceContext.Get());

		ImGuiSetKeyMap(context[type]);
	}
	
	ImGui::StyleColorsDark(); // Set ImGui style (optional)

	auto style = ImGui::GetStyle();
	windowBgCol = style.Colors[ImGuiCol_WindowBg];

	UINT filter = 1;
	for (long long i = 0; i < numFilter; ++i)
	{
		filterNames.push_back(magic_enum::enum_name((FILTER)filter).data());
		filter *= 2;
	}

	//texture追加の処理をする
	HRESULT  hr;
	hr = DirectX::CreateWICTextureFromMemory(
		DirectX11::m_pDevice.Get(), _acicon_sheet_32x32, sizeof(_acicon_sheet_32x32) / sizeof(_acicon_sheet_32x32[0]), NULL, m_pIconTexture.GetAddressOf());

	m_imIconTexture = (ImTextureID)m_pIconTexture.Get();

	return HRESULT();
}


// Open File Dialog to select PNG
std::string OpenFileDialog() {
	OPENFILENAME ofn;
	char szFile[260];
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.lpstrFilter = "PNG Files (*.png)\0*.png\0";//All Files (*.*)\0*.*\0;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrTitle = "Select a PNG file";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		std::string str = szFile;
		return str;
	}

	return " ";
}

namespace fs = std::filesystem;

// Set of allowed extensions
std::set<std::string> allowed_extensions = { ".txt",".png",".jpg",".json"}; // Add the extensions you want to display

// Set of unallowed folder
std::set<std::string> unallowed_folders = {"x64"};

// Struct to store move operations
struct MoveOperation {
	fs::path src;
	fs::path dst;
};

void DisplayFolder(const fs::path& path,std::vector<MoveOperation>& move_operations) {
	for (const auto& entry : fs::directory_iterator(path)) {
		if (entry.is_directory()) {
			if (unallowed_folders.count(entry.path().filename().string()) > 0) continue;

			// Display folder as a drop target
			bool node_open = ImGui::TreeNode(entry.path().filename().string().c_str());

			// Right-click context menu for directories
			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Add New File")) {
					fs::path new_file_path = entry.path() / "new_file.txt"; // Replace with desired file name
					std::ofstream(new_file_path.c_str()); // Create an empty file
					std::cout << std::endl << "Created file: " << new_file_path << std::endl;
				}
				if (ImGui::MenuItem("Add New Folder")) {
					fs::path new_folder_path = entry.path() / "New Folder"; // Replace with desired folder name
					fs::create_directory(new_folder_path);
					std::cout << std::endl << "Created folder: " << new_folder_path << std::endl;
				}
				ImGui::EndPopup();
			}

			// Make folder draggable
			if (ImGui::BeginDragDropSource()) {
				std::string folder_path_str = entry.path().string();
				ImGui::SetDragDropPayload("FOLDER_DRAG", folder_path_str.c_str(), folder_path_str.size() + 1);
				ImGui::Text("Moving folder: %s", entry.path().filename().string().c_str());
				ImGui::EndDragDropSource();
			}

			// Set folder as a drop target
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FOLDER_DRAG")) {
					std::string src_folder(static_cast<const char*>(payload->Data), payload->DataSize);
					fs::path destination = entry.path() / fs::path(src_folder).filename();

					// Store the move operation instead of executing it immediately
					move_operations.push_back({ src_folder, destination });
				}

				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_DRAG")) {
					std::string src_path(static_cast<const char*>(payload->Data), payload->DataSize);
					fs::path destination = entry.path() / fs::path(src_path).filename();

					if (src_path != destination.string())
					{
						// Move the file
						try {
							fs::rename(src_path, destination);
							std::cout << std::endl << "Moved " << src_path << " to " << destination << std::endl;
						}
						catch (const std::filesystem::filesystem_error& e) {
							std::cerr << std::endl << "Error moving file: " << e.what() << std::endl;
						}
					}
				}
				ImGui::EndDragDropTarget();
			}

			// Recursively display subfolders if the folder node is open
			if (node_open)
			{
				DisplayFolder(entry.path(), move_operations);
				ImGui::TreePop();
			}				
		}
		else 
		{
			if (allowed_extensions.count(entry.path().extension().string()) > 0)
			{
				// Unique ID for each file by pushing an ID based on its path
				ImGui::PushID(entry.path().string().c_str());
				//ImGui::BulletText("%s", entry.path().filename().string().c_str());
				if (ImGui::Selectable(entry.path().filename().string().c_str())) {
					// Optional: Handle selection event if needed
				}


				// Make file draggable
				if (ImGui::BeginDragDropSource()) {
					std::string path_str = entry.path().string();
					ImGui::SetDragDropPayload("FILE_DRAG", path_str.c_str(), path_str.size() + 1);
					ImGui::Text("%s", entry.path().filename().string().c_str());
					ImGui::EndDragDropSource();
				}

				ImGui::PopID(); // Restore to avoid ID conflicts
			}
		}
	}
}

void RenderDirectoryTree(const fs::path& root_path) {

	// Vector to store move operations
	std::vector<MoveOperation> move_operations;

	if(ImGui::CollapsingHeader(root_path.filename().string().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Right-click context menu for directories
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Add New File")) {
				fs::path new_file_path = root_path / "new_file.txt"; // Replace with desired file name
				std::ofstream(new_file_path.c_str()); // Create an empty file
				std::cout << std::endl << "Created file: " << new_file_path << std::endl;
			}
			if (ImGui::MenuItem("Add New Folder")) {
				fs::path new_folder_path = root_path / "New Folder"; // Replace with desired folder name
				fs::create_directory(new_folder_path);
				std::cout << std::endl << "Created folder: " << new_folder_path << std::endl;
			}
			ImGui::EndPopup();
		}

		DisplayFolder(root_path, move_operations);

		// Process all move operations after displaying the directory tree
		for (const auto& operation : move_operations) {
			try {
				fs::rename(operation.src, operation.dst); // Execute the move operation
				std::cout << std::endl << "Moved folder: " << operation.src << " to " << operation.dst << std::endl;
			}
			catch (const fs::filesystem_error& e) {
				std::cerr << std::endl << "Error moving folder: " << e.what() << std::endl;
			}
		}
	}
}

// 画面塗りつぶし色
static float clearColor[4] = { 0.0f, 0.25f, 0.25f, 1.0f };

void ImGuiApp::Draw()
{
	for (int type = 0; type < TYPE_MAX; type++)
	{
		ImGui::SetCurrentContext(context[type]);

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		// 描画先キャンバスを塗りつぶす
		DirectX11::m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView[type].Get(), clearColor);

		// 描画先のキャンバスと使用する深度バッファを指定する
		DirectX11::m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView[type].GetAddressOf(), DirectX11::m_pDepthStencilView.Get());

		pDrawImGui[type]();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		//ダブルバッファの切り替えを行い画面を更新する
		m_pSwapChain[type]->Present(0, 0);
	}
}

GameObject* selectedObject = nullptr;

void ImGuiApp::DrawOptionGui()
{
	if (selectedObject != nullptr && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		auto& list = ObjectManager::m_currentList;
		auto iter = list->find(selectedObject->name);
		if (iter != list->end())
		{
			PointerRegistryManager::deletePointer(iter->second.get());
			list->erase(iter);
			selectedObject = nullptr;
		}
	}

	if (ImGui::Begin("Status"))
	{
		ImGui::Text("world average %.3f ms/frame (%d FPS)", 1000.0f / worldFpsCounter, worldFpsCounter);
		ImGui::Text("update average %.3f ms/frame (%d FPS)", 1000.0f / updateFpsCounter, updateFpsCounter);
	}
	ImGui::End();

	static bool showFilterTable = false;

	static float iconTexScale = 1.0f / 20;

	if (ImGui::Begin("Menu")) {
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Tool"))
			{
				static bool pauseGame = true;
				//ImVec4 b2col = playGame ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(1, 1, 1, 1);
				int uvX = pauseGame ? 6 : 12;
				int uvY = pauseGame ? 4 : 1;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				if(ImGui::ImageButton("PauseGame",m_imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)),windowBgCol))
				{
					if (pauseGame){
						PostMessage(Window::GetMainHwnd(), WM_PAUSE_GAME, 0, 0);
					}
					else{
						PostMessage(Window::GetMainHwnd(), WM_RESUME_GAME, 0, 0);
					}

					pauseGame = !pauseGame;
				}
				uvX = 19;
				uvY = 1;
				ImGui::SameLine();
				if (ImGui::ImageButton("ReloadGame", m_imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					SceneManager::ReloadCurrentScene();
					
				}
				ImGui::PopStyleVar();
				ImGui::ColorEdit3("clear color", clearColor); // Edit 3 floats representing a color
				if (ImGui::Button("Button"))
				{
					OpenFileDialog();
				}

				if(ImGui::Button("Serialize"))
				{
					fs::path filePath = /*"asset/object/" + */selectedObject->GetName() + ".json";
					//filePath /= ;
					std::ofstream ofs(filePath);
					cereal::JSONOutputArchive archive(ofs);
					archive(CEREAL_NVP(*selectedObject));
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Box2D"))
			{
				ImGui::Checkbox("HitBox", &RenderManager::drawHitBox);
				ImGui::Checkbox("Ray", &RenderManager::drawRay);
				ImGui::Checkbox("Filter Table", &showFilterTable);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();
	}
	ImGui::End();

	// 3. Show another simple window.
	if (showFilterTable)
	{
		static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_HighlightHoveredColumn;
		static ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;
		static int frozen_cols = 1;
		static int frozen_rows = 2;

		ImGui::Begin("Filter Table", &showFilterTable);
		//const int numFilter = static_cast<int>(filterNames.size());
		bool dummy = false;
		if (ImGui::BeginTable("fitler table", numFilter + 1, table_flags))
		{
			ImGui::TableSetupColumn("FILTER", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder);
			for (int n = numFilter - 1; n >= 0; n--)
				ImGui::TableSetupColumn(filterNames[n], column_flags);
			ImGui::TableSetupScrollFreeze(frozen_cols, frozen_rows);

			ImGui::TableAngledHeadersRow(); // Draw angled headers for all columns with the ImGuiTableColumnFlags_AngledHeader flag.
			ImGui::TableHeadersRow();       // Draw remaining headers and allow access to context-menu and other functions.
			// Disable input for the following items
			//ImGui::BeginDisabled(true); // Disable interaction
			for (int row = 0; row < numFilter; row++)
			{
				ImGui::PushID(row);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s", filterNames[row]);

				FILTER rowFilter = (FILTER)pow(2, row);
				UINT maskbit = Box2DBodyManager::GetMaskFilterBit(rowFilter);
				for (int column = 1; column < numFilter - row + 1; column++)
					if (ImGui::TableSetColumnIndex(column))
					{
						ImGui::PushID(column);
						FILTER colFilter = static_cast<FILTER>(pow(2, numFilter - column));
						UINT bit = maskbit & colFilter;
						bool collision = colFilter == bit;
						if (ImGui::Checkbox("", &collision))
						{
							if (collision){
								Box2DBodyManager::EnableCollisionFilter(rowFilter, colFilter);
							}
							else {
								Box2DBodyManager::DisableCollisionFilter(rowFilter, colFilter);
							}
						}
						ImGui::PopID();
					}
				ImGui::PopID();
			}
			//ImGui::EndDisabled(); // Re-enable interaction

			ImGui::EndTable();
		}

		ImGui::End();
	}

	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::BeginTabBar("hierarchyTab"))
		{
			if (ImGui::BeginTabItem("ObjectList"))
			{
				for (auto& object : *ObjectManager::m_currentList) {
					bool selected = object.second.get() == selectedObject;
					if (!object.second->active)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.4f)); // Set text color to red
					}
					if (ImGui::Selectable(object.second->GetName().c_str(), selected)) {
						// Handle selection, e.g., highlighting the object or showing more details
						selectedObject = object.second.get();
					}
					if (!object.second->active)
					{
						ImGui::PopStyleColor();
					}
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("FileList"))
			{
				static fs::path currentPath = fs::current_path();

				RenderDirectoryTree(currentPath);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("SceneList"))
			{
				for (auto scene : SceneManager::m_sceneList)
				{
					std::string currentSceneName = typeid(*SceneManager::m_currentScene).name();
					bool current = currentSceneName == scene.first;
					if (ImGui::Selectable(scene.first.substr(5).c_str(), &current))
					{
						SceneManager::LoadScene(scene.first);
					}
				}

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();
	}
	ImGui::End();
}

void ImGuiApp::DrawInspectorGui()
{
	if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None))
	{
		if (selectedObject != nullptr)
		{
			ImGui::SeparatorText("General");

			ImGui::Text("Selected : %s", selectedObject->name.c_str());
			bool active = selectedObject->active;
			ImGui::SameLine();
			ImGui::Checkbox(" ", &active);
			if (active != selectedObject->active)
			{
				selectedObject->SetActive(active);
			}

			ImGui::SeparatorText("Component");

			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_None))
			{
				if (ImGui::InputFloat3("Position", selectedObject->transform.position.data(), "%.1f"))
				{
					if (selectedObject->ExistComponent<Box2DBody>())
					{
						auto box2d = selectedObject->GetComponent<Box2DBody>();
						box2d->SetPosition(selectedObject->transform.position);
					}
				}
				ImGui::InputFloat3("Scale", selectedObject->transform.scale.data(), "%.1f");
				auto& angle = selectedObject->transform.angle;
				float angles[3] = {
					static_cast<float>(angle.x),
					static_cast<float>(angle.y),
					static_cast<float>(angle.z)
				};
				if (ImGui::InputFloat3("Angle", angles, "%.1f"))
				{
					angle.x = angles[0];
					angle.y = angles[1];

					Angle radZ(angles[2]);
					if (angle.z != radZ)
					{
						angle.z = radZ;
						if (selectedObject->ExistComponent<Box2DBody>())
						{
							auto box2d = selectedObject->GetComponent<Box2DBody>();
							box2d->SetAngle(radZ);
						}
					}
				}

				//ImGui::TreePop();
			}

			for (auto& component : selectedObject->m_componentList)
			{
				std::string componentName = component.first;

				if (ImGui::CollapsingHeader(componentName.substr(6).c_str(), ImGuiTreeNodeFlags_None))
				{
					component.second->DrawImGui();
					//ImGui::TreePop();
				}
			}

			ImVec2 buttonPos = ImGui::GetCursorScreenPos();
			buttonPos.x += 50;
			buttonPos.y += 25;
			ImGui::SetCursorScreenPos(buttonPos);
			if (ImGui::Button("Add Component", ImVec2(300, 30)))
			{
			//==================================================
			// コンポーネント追加gui
			//==================================================
			}
		}
		else
		{
			ImGui::Text("not selected gameobject");
		}
		
	}
	ImGui::End();
}


void ImGuiApp::UnInit()
{
	for (int type = 0; type < TYPE_MAX; type++)
	{
		ImGui::SetCurrentContext(context[type]);

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}

LRESULT ImGuiApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto iter = m_hWndContexts.find(hWnd);
	if (iter != m_hWndContexts.end())
	{
		ImGui::SetCurrentContext(iter->second);
		// Get ImGui IO reference
		ImGuiIO& io = ImGui::GetIO();

		switch (uMsg) {
			// Mouse button down
		case WM_LBUTTONDOWN:
			io.MouseDown[0] = true;  // Left mouse button
			break;
		case WM_RBUTTONDOWN:
			io.MouseDown[1] = true;  // Right mouse button
			break;
		case WM_MBUTTONDOWN:
			io.MouseDown[2] = true;  // Middle mouse button
			break;

			// Mouse button up
		case WM_LBUTTONUP:
			io.MouseDown[0] = false;
			break;
		case WM_RBUTTONUP:
			io.MouseDown[1] = false;
			break;
		case WM_MBUTTONUP:
			io.MouseDown[2] = false;
			break;

			// Mouse movement
		case WM_MOUSEMOVE:
			io.MousePos.x = (float)GET_X_LPARAM(lParam);
			io.MousePos.y = (float)GET_Y_LPARAM(lParam);
			break;

			// Mouse wheel
		case WM_MOUSEWHEEL:
			io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
			break;

			// Keyboard events
		case WM_KEYDOWN:
			if (wParam < 256)
				io.KeysDown[wParam] = 1;
			break;
		case WM_KEYUP:
			if (wParam < 256)
				io.KeysDown[wParam] = 0;
			break;
		case WM_CHAR:
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			break;
		}
	}

	// Pass any unhandled messages to the default window procedure
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ImGuiSetKeyMap(ImGuiContext* _imguiContext)
{
	ImGui::SetCurrentContext(_imguiContext);
	ImGuiIO& io = ImGui::GetIO();
#ifdef _WIN32
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;             // Map the Tab key
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;      // Map the Left Arrow key
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;    // Map the Right Arrow key
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;          // Map the Up Arrow key
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;      // Map the Down Arrow key
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;        // Map the Page Up key
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;       // Map the Page Down key
	io.KeyMap[ImGuiKey_Home] = VK_HOME;           // Map the Home key
	io.KeyMap[ImGuiKey_End] = VK_END;             // Map the End key
	io.KeyMap[ImGuiKey_Insert] = VK_INSERT;       // Map the Insert key
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;       // Map the Delete key
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;      // Map the Backspace key
	io.KeyMap[ImGuiKey_Space] = VK_SPACE;         // Map the Space key
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;        // Map the Enter key
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;       // Map the Escape key
	io.KeyMap[ImGuiKey_A] = 'A';                  // Map the 'A' key (for Ctrl+A, etc.)
	io.KeyMap[ImGuiKey_C] = 'C';                  // Map the 'C' key (for Ctrl+C, etc.)
	io.KeyMap[ImGuiKey_V] = 'V';                  // Map the 'V' key (for Ctrl+V, etc.)
	io.KeyMap[ImGuiKey_X] = 'X';                  // Map the 'X' key (for Ctrl+X, etc.)
	io.KeyMap[ImGuiKey_Y] = 'Y';                  // Map the 'Y' key (for Ctrl+Y, etc.)
	io.KeyMap[ImGuiKey_Z] = 'Z';                  // Map the 'Z' key (for Ctrl+Z, etc.)
#elif defined(__APPLE__)
	io.KeyMap[ImGuiKey_Tab] = kVK_Tab;             // Map the Tab key
	io.KeyMap[ImGuiKey_LeftArrow] = kVK_LEFT;      // Map the Left Arrow key
	io.KeyMap[ImGuiKey_RightArrow] = kVK_Right;    // Map the Right Arrow key
	io.KeyMap[ImGuiKey_UpArrow] = kVK_Up;          // Map the Up Arrow key
	io.KeyMap[ImGuiKey_DownArrow] = kVK_Down;      // Map the Down Arrow key
	io.KeyMap[ImGuiKey_PageUp] = kVK_Prior;        // Map the Page Up key
	io.KeyMap[ImGuiKey_PageDown] = kVK_Next;       // Map the Page Down key
	io.KeyMap[ImGuiKey_Home] = kVK_Home;           // Map the Home key
	io.KeyMap[ImGuiKey_End] = kVK_End;             // Map the End key
	io.KeyMap[ImGuiKey_Insert] = kVK_Insert;       // Map the Insert key
	io.KeyMap[ImGuiKey_Delete] = kVK_Delete;       // Map the Delete key
	io.KeyMap[ImGuiKey_Backspace] = kVK_Back;      // Map the Backspace key
	io.KeyMap[ImGuiKey_Space] = kVK_Space;         // Map the Space key
	io.KeyMap[ImGuiKey_Enter] = kVK_Return;        // Map the Enter key
	io.KeyMap[ImGuiKey_Escape] = kVK_Escape;       // Map the Escape key
	io.KeyMap[ImGuiKey_A] = 'A';                  // Map the 'A' key (for Ctrl+A, etc.)
	io.KeyMap[ImGuiKey_C] = 'C';                  // Map the 'C' key (for Ctrl+C, etc.)
	io.KeyMap[ImGuiKey_V] = 'V';                  // Map the 'V' key (for Ctrl+V, etc.)
	io.KeyMap[ImGuiKey_X] = 'X';                  // Map the 'X' key (for Ctrl+X, etc.)
	io.KeyMap[ImGuiKey_Y] = 'Y';                  // Map the 'Y' key (for Ctrl+Y, etc.)
	io.KeyMap[ImGuiKey_Z] = 'Z';                  // Map the 'Z' key (for Ctrl+Z, etc.)
#endif
}


#endif
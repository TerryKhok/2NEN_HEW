#include"../../DirectX11_2D_Framework/img/icon_sheet_32x32.c"

#ifdef DEBUG_TRUE
//#include "../font/VL-Gothic-Regular.c"

int ImGuiApp::worldFpsCounter;
int ImGuiApp::updateFpsCounter = 0;

//window関連
HWND ImGuiApp::m_hWnd[TYPE_MAX];
ImGuiContext* ImGuiApp::context[TYPE_MAX];
std::unordered_map<HWND, ImGuiContext*> ImGuiApp::m_hWndContexts;
ComPtr<IDXGISwapChain> ImGuiApp::m_pSwapChain[TYPE_MAX];
ComPtr<ID3D11RenderTargetView> ImGuiApp::m_pRenderTargetView[TYPE_MAX];
void(*ImGuiApp::pDrawImGui[ImGuiApp::TYPE_MAX])() = {};
std::unordered_map<std::string, std::array<int, ImGuiApp::VALUE_TYPE_MAX>> ImGuiApp::updateValues;
bool ImGuiApp::showMainEditor = false;
ImGuiApp::HandleUI ImGuiApp::handleUi;

ComPtr<ID3D11ShaderResourceView> pIconTexture;
ImTextureID imIconTexture;
std::stack<std::function<void()>> changes;
std::stack<std::pair<std::unique_ptr<GameObject, void(*)(GameObject*)>,std::pair<std::string,size_t>>> willDeleteObjects;
std::stack<std::unique_ptr<Component, void(*)(Component*)>> willRemoveComponents;
std::vector<const char*> filterNames;
constexpr long long numFilter = magic_enum::enum_count<FILTER>() - 1;
ImVec4 windowBgCol;

std::map<std::filesystem::path, std::pair<ComPtr<ID3D11ShaderResourceView>, ImTextureID>> textureSource;
std::map<std::filesystem::path, std::set<int>> textureCutNode;

struct ImTextureData
{
	ImTextureID id;
	ImVec2 uv0;
	ImVec2 uv1;
};

ImTextureData currentImTexture;

struct ImAnimationFrame
{
	std::filesystem::path path;
	int splitX = 1;
	int splitY = 1;
	int uvX = 0;
	int uvY = 0;
	float waitTime = 1.0f;
};

std::vector<ImAnimationFrame> willPushFrames;
std::vector<ImAnimationFrame> imAniFrames;

constexpr int maxTexSplit[2] = { 100,100 };

GameObject* selectedObject = nullptr;

float iconTexScale = 1.0f / 20;

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
	windowPos[OPTIONS] = { Window::MONITER_HALF_WIDTH / -1.35f - 8.0f,0 };
	windowPos[INSPECTER] = { Window::MONITER_HALF_WIDTH / 1.35f + 8.0f,0 };

	// Use std::fill to set all elements to the same function
	std::fill(std::begin(pDrawImGui), std::end(pDrawImGui),[](){});
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
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		m_hWndContexts.insert(std::make_pair(hWnd, context[type]));

		ImGui_ImplWin32_Init(m_hWnd[type]); // hWnd is your main window handle
		ImGui_ImplDX11_Init(DirectX11::m_pDevice.Get(), DirectX11::m_pDeviceContext.Get());

		//if (type == INSPECTER)
		/*{
			io.Fonts->AddFontFromMemoryTTF((void*)_acVL_Gothic_Regular, sizeof(_acVL_Gothic_Regular), 15.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
			io.Fonts->Build();
			ImGui_ImplDX11_CreateDeviceObjects();
		}*/

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
		DirectX11::m_pDevice.Get(), _acicon_sheet_32x32, sizeof(_acicon_sheet_32x32) / sizeof(_acicon_sheet_32x32[0]), NULL, pIconTexture.GetAddressOf());

	imIconTexture = (ImTextureID)pIconTexture.Get();

	currentImTexture.id = imIconTexture;
	int uvX = 12;
	int uvY = 17;
	currentImTexture.uv0 = ImVec2(iconTexScale * uvX, iconTexScale * uvY);
	currentImTexture.uv1 = ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1));

	handleUi.Init();

	return HRESULT();
}

namespace fs = std::filesystem;

void SaveAnimationClipFile(const std::string& _path);
void SaveAnimationClipFileDialog();
void ReadAnimationClipFile(fs::path _path);


// Set of allowed extensions
std::set<std::string> allowed_extensions = { ".txt",".png",".jpg",".dds",".json",".wav",ANIM_CLIP_EXTENSION_DOT }; // Add the extensions you want to display

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

		//ImGui::ShowMetricsWindow(); // メトリクスウィンドウを表示

		ImGuiIO& io = ImGui::GetIO();

		// キーボードのキー入力を設定 (必要なキーを追加)
		io.AddKeyEvent(ImGuiKey_Space, GetAsyncKeyState(VK_SPACE) & 0x8000);
		io.AddKeyEvent(ImGuiKey_Enter, GetAsyncKeyState(VK_RETURN) & 0x8000);
		io.AddKeyEvent(ImGuiKey_Escape, GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		io.AddKeyEvent(ImGuiKey_Backspace, GetAsyncKeyState(VK_BACK) & 0x8000);
		io.AddKeyEvent(ImGuiKey_LeftArrow, GetAsyncKeyState(VK_LEFT) & 0x8000);
		io.AddKeyEvent(ImGuiKey_RightArrow, GetAsyncKeyState(VK_RIGHT) & 0x8000);
		io.AddKeyEvent(ImGuiKey_UpArrow, GetAsyncKeyState(VK_UP) & 0x8000);
		io.AddKeyEvent(ImGuiKey_DownArrow, GetAsyncKeyState(VK_DOWN) & 0x8000);

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

void ImGuiApp::DrawOptionGui()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));  // Start at the end of game viewport
	ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, 100)); // Cover the remaining area
	if (ImGui::Begin("Status",nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("StatusTabBar", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Performance"))
			{
				ImGui::Text("world average %.3f ms/frame (%d FPS)", 1000.0f / worldFpsCounter, worldFpsCounter);
				ImGui::Text("update average %.3f ms/frame (%d FPS)", 1000.0f / updateFpsCounter, updateFpsCounter);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Input"))
			{
				Vector2 mousePos = Input::Get().MousePoint();
				ImGui::Text("mousePos x : %.3f, y : %.3f", mousePos.x, mousePos.y);
				Vector2 worldPos = mousePos;
				worldPos.x = worldPos.x * DISPALY_ASPECT_WIDTH / RenderManager::renderZoom.x + RenderManager::renderOffset.x;
				worldPos.y = worldPos.y * DISPALY_ASPECT_HEIGHT / RenderManager::renderZoom.y + RenderManager::renderOffset.y;
				ImGui::Text("worldMousePos x : %.3f, y : %.3f", worldPos.x, worldPos.y);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();
	}
	ImGui::End();

	static bool showFilterTable = false;

	ImGui::SetNextWindowPos(ImVec2(0,100));  // Start at the end of game viewport
	ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, 150)); // Cover the remaining area
	if (ImGui::Begin("Menu",nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
		{
			if (ImGui::BeginTabItem("Game"))
			{
				ImGui::Text("ScenePath : %s", SceneManager::currentScenePath.string().c_str());

				static bool pauseGame = false;
				//ImVec4 b2col = playGame ? ImVec4(0.5f, 0.5f, 0.5f, 1.0f) : ImVec4(1, 1, 1, 1);
				int uvX = pauseGame ? 13 : 16;
				int uvY = 13;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				if(ImGui::ImageButton("PauseGame",imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)),windowBgCol))
				{
					if (pauseGame){
						PostMessage(Window::GetMainHWnd(), WM_PAUSE_GAME, 0, 0);
						//float color[4] = { 0.1f,0.1f,0.1f,1.0f };
						float color[4] = { 0.1f,0.1f,0.1f,1.0f };
						memcpy(DirectX11::clearColor, color, sizeof(color));
					}
					else{
						PostMessage(Window::GetMainHWnd(), WM_RESUME_GAME, 0, 0);
						//float color[4] = { 0.0f,0.25f,0.25f,1.0f };
						float color[4] = { 0.0f,0.5f,0.5f,1.0f };
						memcpy(DirectX11::clearColor, color, sizeof(color));

						for (auto pair : updateValues)
						{
							auto object = ObjectManager::Find(pair.first);
							if (object == nullptr) continue;

							try {
								if (pair.second[BOX2D_POSITION] > 0)
								{
									object->GetComponent<Box2DBody>()->SetPosition(object->transform.position);
								}
								if (pair.second[BOX2D_ROTATION] > 0)
								{
									object->GetComponent<Box2DBody>()->SetAngle(object->transform.angle.z);
								}
								if (pair.second[WINDOW_POSITION] > 0)
								{
									auto windowRect = object->GetComponent<SubWindow>();
									SetWindowPosition(windowRect->m_hWnd, object->transform.position);
								}
							}
							catch (const std::exception& e) {
									LOG_ERROR(e.what());
							}
							catch (...) {}
						}

						ImGuiApp::ClearStack();
					}

					pauseGame = !pauseGame;
				}
				ImGui::SetItemTooltip(pauseGame ? "pause" : "play");

				uvX = 19;
				uvY = 1;
				ImGui::SameLine();
				if (ImGui::ImageButton("ReloadGame", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					SceneManager::ReloadCurrentScene();
					ImGuiApp::ClearStack();
					handleUi.lock = false;
				}
				ImGui::SetItemTooltip("reload");
				
				uvX = showMainEditor ? 15 : 13;
				uvY = 18;
				ImGui::SameLine();
				if (ImGui::ImageButton("mainEditor",imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					showMainEditor = !showMainEditor;
					CRect rect;
					::GetClientRect(Window::GetMainHWnd(), &rect);
					// ビューポートを作成（→画面分割などに使う、描画領域の指定のこと）
					D3D11_VIEWPORT viewport;
					if (showMainEditor)
					{
						viewport.TopLeftX = rect.Width() / 4.0f;
						viewport.Width = (FLOAT)rect.Width() / 2.0f;
						viewport.Height = (FLOAT)rect.Height() / 2.0f;
					}
					else
					{
						viewport.TopLeftX = 0;
						viewport.Width = (FLOAT)rect.Width();
						viewport.Height = (FLOAT)rect.Height();
					}
					viewport.TopLeftY = 0;
					viewport.MinDepth = 0.0f;
					viewport.MaxDepth = 1.0f;
					DirectX11::m_pDeviceContext->RSSetViewports(1, &viewport);
				}
				ImGui::SetItemTooltip(showMainEditor ? "close editor" : "show editor");

				uvX = 8;
				uvY = 5;
				ImGui::SameLine();
				if (ImGui::ImageButton("saveScene", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
					if (SceneManager::currentScenePath == "null")
					{
						 SaveSceneFileDialog(SceneManager::currentScenePath);
					}

					if (SceneManager::currentScenePath != "null")
						SceneManager::SaveScene(SceneManager::currentScenePath);
				}
				ImGui::SetItemTooltip("saveScene");

				uvX = 10;
				uvY = 5;
				ImGui::SameLine();
				if (ImGui::ImageButton("overSaveScene", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
					SaveSceneFileDialog(SceneManager::currentScenePath);

					if (SceneManager::currentScenePath != "null")
						SceneManager::SaveScene(SceneManager::currentScenePath);
				}
				ImGui::SetItemTooltip("overSaveScene");

				uvX = 18;
				uvY = 7;
				ImGui::SameLine();
				if (ImGui::ImageButton("serializeObject", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					if (selectedObject != nullptr)
					{
						fs::path filePath;
						ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
						SaveSceneFileDialog(filePath);
						std::ofstream ofs(filePath);
						SERIALIZE_OUTPUT archive(ofs);
						int index = ObjectFileIndex;
						archive(CEREAL_NVP(index));
						archive(CEREAL_NVP(*selectedObject));
					}
				}
				ImGui::SetItemTooltip("pack object");

				uvX = 13;
				uvY = 7;
				ImGui::SameLine();
				if (ImGui::ImageButton("deserializeObject", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					handleUi.SetUploadFile("object file",
						[](GameObject* obj, std::filesystem::path path)
						{
							ObjectManager::AddObject(path);
						}, { ".json" });
				}
				ImGui::SetItemTooltip("unpack object");

				uvX = handleUi.lock ? 17 : 1;
				uvY = handleUi.lock ? 10 : 18;
				ImGui::SameLine();
				if (ImGui::ImageButton("LockHandle", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
				{
					static bool lock = false;
					lock = !lock;
					handleUi.LockHandle(lock, "OptionGui");
				}
				ImGui::SetItemTooltip("lockHandle");

				ImGui::PopStyleVar();

				ImGui::ColorEdit3("clear color", DirectX11::clearColor); // Edit 3 floats representing a color
		
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Display"))
			{
				ImGui::Checkbox("Collider", &RenderManager::drawHitBox);
				ImGui::Checkbox("Ray", &RenderManager::drawRay);
				ImGui::Checkbox("Box", &RenderManager::drawBox);
				ImGui::Checkbox("Filter Table", &showFilterTable);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Camera"))
			{
				ImGui::InputFloat2("offset", RenderManager::renderOffset.data());
				ImGui::InputFloat2("zoom", RenderManager::renderZoom.data());
				ImGui::InputFloat("angle",&CameraManager::cameraRotation);
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
		if (ImGui::BeginTable("filter table", numFilter + 1, table_flags))
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

	ImGui::SetNextWindowPos(ImVec2(0, 250));  // Start at the end of game viewport
	ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, 470)); // Cover the remaining area
	if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		if (ImGui::BeginTabBar("hierarchyTab"))
		{
			if (ImGui::BeginTabItem("ObjectList"))
			{
				ImGui::BeginChild("objectList");
				auto& objectList = ObjectManager::m_currentList;
				int size = (int)objectList->first.size();
				for (int i = 0; i < size;i++) {
					auto& object = objectList->first[i];
					if (!object->active)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.4f)); // Set text color to red
					}
					bool selected = object->isSelected == GameObject::SELECTED;
					if (ImGui::Selectable(object->GetName().c_str(),selected) && !handleUi.lock)
					{
						if (selected)
						{
							InvalidSelectedObject();
						}
						else
						{
							// Handle selection, e.g., highlighting the object or showing more details
							if (selectedObject != nullptr) selectedObject->isSelected = GameObject::SELECT_NONE;
							selectedObject = object.get();
							if (selectedObject != nullptr) selectedObject->isSelected = GameObject::SELECTED;
						}
					}
					// Source: Dragging starts here
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						ImGui::SetDragDropPayload("DND_SCENE_ITEM", &i, sizeof(int)); // Pass the index
						ImGui::Text("%s", object->name.c_str());
						ImGui::EndDragDropSource();
					}
					// Target: Dropping ends here
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SCENE_ITEM")) {
							size_t dragged_index = *(const size_t*)payload->Data;
							if (dragged_index != i) {
								// Move the dragged item to the target index
								std::swap(objectList->first[i], objectList->first[dragged_index]);
								objectList->second[objectList->first[i]->name] = i;
								objectList->second[objectList->first[dragged_index]->name] = dragged_index;
							}
						}
						ImGui::EndDragDropTarget();
					}

					if (!object->active)
					{
						ImGui::PopStyleColor();
					}
				}
				ImGui::EndChild();
				if (ImGui::BeginPopupContextItem())
				{
					static char buf[128];
					ImGui::SeparatorText("Add Object");
					ImGui::InputText("name",buf,sizeof(buf));

					if (ImGui::MenuItem("+ Empty"))
					{
						GameObject* object = nullptr;
						if (buf[0] != '\0') 
							object = new GameObject(buf);
						else 
							object = new GameObject("empty");
						ObjectManager::AddObject(object);
						memset(buf, '\0', strlen(buf));
					}
					if (ImGui::MenuItem("+ Render"))
					{
						GameObject* object = nullptr;
						if (buf[0] != '\0')
							object = new GameObject(buf);
						else
							object = new GameObject("rend");
						ObjectManager::AddObject(object);
						object->AddComponent<Renderer>();
						memset(buf, '\0', strlen(buf));
					}
					if (ImGui::MenuItem("+ Box2D"))
					{
						GameObject* object = nullptr;
						if (buf[0] != '\0')
							object = new GameObject(buf);
						else
							object = new GameObject("box2d");
						ObjectManager::AddObject(object);
						object->AddComponent<Box2DBody>();
						memset(buf, '\0', strlen(buf));
					}
					
					ImGui::EndPopup();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("FileList"))
			{
				static fs::path currentPath = fs::current_path();

				std::string upFileType = "\n<<< UPLOAD FILE HERE\n<<< " + handleUi.uploadStr;
				for (auto& extension : handleUi.extensions)
				{
					upFileType += " << " + extension;
				}
				//ImGui::Button("drop here");
				int uvX = 18;
				int uvY = 6;
				ImGui::BeginGroup();
				{
					ImGui::Image(imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY),
						ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)));
					ImGui::SameLine();
					ImGui::Text(upFileType.c_str());
					ImGui::EndGroup();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_DRAG"))
					{
						IM_UNUSED(payload);
						ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
						fs::path path(std::string(static_cast<const char*>(payload->Data), payload->DataSize));
						//bool same = path.extension().string().compare(handleUi.extension);
						std::string extension = path.extension().string();
						bool same = false;
						for (auto& ex : handleUi.extensions)
						{
							same = std::equal(ex.begin(), ex.end(), extension.c_str());
							if (same) break;
						}
						if (same)
							handleUi.linkFunc(selectedObject, fs::relative(path, currentPath));
						//src_folder = fs::relative(path, currentPath).string();
					}
					ImGui::EndDragDropTarget();
				}
				
				ImGui::EndTabItem();

				ImGui::BeginChild("FileWindow", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, 0);
				RenderDirectoryTree(currentPath);
				ImGui::EndChild();
			}

			if (ImGui::BeginTabItem("SceneList"))
			{
				ImGui::BeginChild("SceneListChild");
				int eraseIndex = -1;
				static int changeSceneIndex = -1;
				auto& sceneList = SceneManager::m_registerScenePath;
				int sceneSize = (int)sceneList.size();
				static bool dont_ask_me_next_time = false;
				for (int i = 0;i < sceneSize;i++)
				{
					auto& scene = sceneList[i];
					ImGui::BeginGroup();
					{
						if (sceneSize > 1)
						{
							ImGui::PushID(i);
							if (ImGui::Button("-"))
							{
								eraseIndex = i;
							}
							ImGui::SetItemTooltip("move");
							ImGui::PopID();
						}

						std::string currentSceneName = SceneManager::m_currentScene->getType();
						ImGui::SameLine();
						bool current = currentSceneName == scene.name;
						if (ImGui::Selectable(scene.name.c_str(), &current))
						{
							if(!dont_ask_me_next_time){
								if (IsSceneChange()){
									ImGui::OpenPopup("SaveScene?");
									changeSceneIndex = i;
								}
								else{
									SceneManager::LoadSceneNotThrow(scene.name);
									ImGuiApp::ClearStack();
									handleUi.lock = false;
								}
							}
							else{ 
								SceneManager::LoadSceneNotThrow(scene.name);
								ImGuiApp::ClearStack();
								handleUi.lock = false;
							}
						}
						// Source: Dragging starts here
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
							ImGui::SetDragDropPayload("DND_SCENE_ITEM", &i, sizeof(int)); // Pass the index
							ImGui::Text("%s", currentSceneName.c_str());
							ImGui::EndDragDropSource();
						}
					}
					ImGui::EndGroup();
					// Target: Dropping ends here
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SCENE_ITEM")) {
							size_t dragged_index = *(const size_t*)payload->Data;
							if (dragged_index != i) {
								// Move the dragged item to the target index
								auto dragged_item = sceneList[dragged_index];
								sceneList.erase(sceneList.begin() + dragged_index);
								sceneList.insert(sceneList.begin() + i, dragged_item);
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
				// Always center this window when appearing
				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				if (ImGui::BeginPopupModal("SaveScene?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Do you want to save changes to the scene?");
					ImGui::Separator();

					//static int unused_i = 0;
					//ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
					ImGui::PopStyleVar();

					if (ImGui::Button("Yes", ImVec2(120, 0))) {
						if (SceneManager::currentScenePath == "null")
						{
							ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
							SaveSceneFileDialog(SceneManager::currentScenePath);
						}
						if (SceneManager::currentScenePath != "null")
							SceneManager::SaveScene(SceneManager::currentScenePath);
						SceneManager::LoadSceneNotThrow(sceneList[changeSceneIndex].name);
						ImGuiApp::ClearStack();
						handleUi.lock = false;
						ImGui::CloseCurrentPopup(); 
					}
					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(120, 0))) { 

						SceneManager::LoadSceneNotThrow(sceneList[changeSceneIndex].name);
						ImGuiApp::ClearStack();
						handleUi.lock = false;
						ImGui::CloseCurrentPopup();
					}
					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}
				ImGui::EndChild();
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::Selectable("New Scene"))
					{
						ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
						fs::path savePath;
						SaveSceneFileDialog(savePath);
						std::ofstream ofs(savePath);
						SERIALIZE_OUTPUT archive(ofs);
						int index = SceneFileIndex;
						int objectNum = 0;
						archive(CEREAL_NVP(index), CEREAL_NVP(objectNum));
						fs::path relativePath = savePath.lexically_relative(fs::current_path());
						SceneManager::RegisterScene(relativePath);
					}

					if (ImGui::Selectable("Load Scene"))
					{
						handleUi.SetUploadFile("Scene File", [](GameObject* _obj, fs::path _path)
							{
								int index = 0;
								{
									std::ifstream ifs(_path);
									SERIALIZE_INPUT archive(ifs);
									archive(CEREAL_NVP(index));
								}
								if (index == SceneFileIndex)
								{
									SceneManager::RegisterScene(_path);
								}
								else
								{
									LOG_WARNING("upload file is not scene file");
								}
							}, { ".json" });
					}

					if (ImGui::BeginMenu("Add Class Scene"))
					{
						for (auto& assembly : AssemblyScene::assemblies)
						{
							if (ImGui::MenuItem(assembly.first.substr(6).c_str()))
							{
								AssemblyScene::RegisterScene(assembly.first);
							}
						}
						ImGui::EndMenu();
					}

					ImGui::EndPopup();
				}

			

				if (eraseIndex >= 0)
				{
					auto iter = sceneList.begin() + eraseIndex;
					auto it = SceneManager::m_sceneList.find(iter->name);
					if (it != SceneManager::m_sceneList.end())
					{
						SceneManager::m_sceneList.erase(it);
					}
					sceneList.erase(iter);
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
	ImGui::SetNextWindowPos(ImVec2(0, 0));  // Start at the end of game viewport
	ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT)); // Cover the remaining area
	if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		if (selectedObject != nullptr)
		{
			ImGui::SeparatorText("General");
			
			bool active = selectedObject->active;
			ImGui::Checkbox(" ", &active);
			ImGui::SetItemTooltip("active");
			if (active != selectedObject->active)
			{
				selectedObject->SetActive(active);
			}
			ImGui::SameLine();

			static char str[128] = {};
			if (Window::IsPause())
			{
				memset(str, '\0', strlen(str));
				memcpy(str, selectedObject->name.c_str(), selectedObject->name.size());
				if (ImGui::InputText("##Selected", str, sizeof(str), ImGuiInputTextFlags_EnterReturnsTrue) && str[0] != '\0')
				{
					selectedObject->SetName(str);
				}
			}
			else ImGui::Text("%s : Selected", selectedObject->name.c_str());

			ImGui::SeparatorText("Component");

			ImGui::Button("/");
			ImGui::SetItemTooltip("not remove transform");
			ImGui::SameLine();
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_None))
			{
				if (ImGui::InputFloat3("Position", selectedObject->transform.position.data(), "%.1f"))
				{
					try
					{
						if (selectedObject->ExistComponent<Box2DBody>())
						{
							auto box2d = selectedObject->GetComponent<Box2DBody>();
							box2d->SetPosition(selectedObject->transform.position);
						}
						if (selectedObject->ExistComponent<SubWindow>())
						{
							auto windowRect = selectedObject->GetComponent<SubWindow>();
							SetWindowPosition(windowRect->m_hWnd, selectedObject->transform.position);
						}
					}
					catch (const std::exception& e) {
						LOG_ERROR(e.what());
					}
					catch (...) {}
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

			std::vector<std::string> eraseComponents;
			auto& comList = selectedObject->m_componentList;
			for (int i = 0;i < comList.first.size();i++)
			{
				auto& component = comList.first[i];
				std::string componentName = component->getType();

				if (!handleUi.lock)
				{
					ImGui::PushID(i);
					if (ImGui::Button("-"))
					{
						eraseComponents.push_back(componentName);
					}
					ImGui::SetItemTooltip("remove");
					
					ImGui::PopID();

					ImGui::SameLine();
				}
				if (ImGui::CollapsingHeader(componentName.substr(6).c_str(), ImGuiTreeNodeFlags_None))
				{
					component->DrawImGui(handleUi);
				}
				else
				{
					// Source: Dragging starts here
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						ImGui::SetDragDropPayload("DND_COMPONENT_ITEM", &i, sizeof(int)); // Pass the index
						ImGui::Text("%s", componentName.c_str());
						ImGui::EndDragDropSource();
					}
				}

				// Target: Dropping ends here
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_COMPONENT_ITEM")) {
						size_t dragged_index = *(const size_t*)payload->Data;
						if (dragged_index != i) {
							// Move the dragged item to the target index
							std::swap(comList.first[i], comList.first[dragged_index]);
							comList.second[comList.first[i]->getType()] = i;
							comList.second[comList.first[dragged_index]->getType()] = dragged_index;
						}
					}
					ImGui::EndDragDropTarget();
				}

			}

			for (auto name : eraseComponents)
			{
				auto& list = selectedObject->m_componentList;
				auto iter = list.second.find(name.c_str());
				if (iter != list.second.end())
				{
					if (Window::IsPause())
					{
						if (name == typeid(Renderer).name())
						{
							auto it = list.second.find(typeid(Animator).name());
							if (it != list.second.end())
							{
								auto& com = list.first[it->second];

								com->SetActive(false);
								com->Delete();

								willRemoveComponents.push(std::move(com));

								auto& vector = list.first;
								auto& map = list.second;

								size_t index = it->second;
								size_t lastIndex = vector.size() - 1;

								// Move the last entity to the position of the entity to be removed
								if (index != lastIndex) {
									vector[index] = std::move(vector[lastIndex]);
									map[vector[index]->getType().c_str()] = index; // Update map for the moved entity
								}

								// Remove the last element and update map
								vector.pop_back();
								map.erase(it);

								changes.emplace(std::bind([](GameObject* obj) {
									willRemoveComponents.top()->SetActive(true);
									auto& vec = obj->m_componentList.first;
									vec.push_back(std::move(willRemoveComponents.top()));
									obj->m_componentList.second.insert(std::make_pair(vec.back()->getType(), vec.size() - 1));
									willRemoveComponents.pop();
									}, selectedObject));
							}
						}

						auto& component = list.first[iter->second];
						component->SetActive(false);

						willRemoveComponents.push(std::move(component));

						auto& vector = list.first;
						auto& map = list.second;

						size_t index = iter->second;
						size_t lastIndex = vector.size() - 1;

						// Move the last entity to the position of the entity to be removed
						if (index != lastIndex) {
							vector[index] = std::move(vector[lastIndex]);
							map[vector[index]->getType().c_str()] = index; // Update map for the moved entity
						}

						// Remove the last element and update map
						vector.pop_back();
						map.erase(iter);

						changes.emplace(std::bind([](GameObject* obj) {
							willRemoveComponents.top()->SetActive(true);
							auto& vec = obj->m_componentList.first;
							vec.push_back(std::move(willRemoveComponents.top()));
							obj->m_componentList.second.insert(std::make_pair(vec.back()->getType(),vec.size() - 1));
							willRemoveComponents.pop();
							}, selectedObject));
					}
					else
					{
						auto& component = list.first[iter->second];
#ifdef DEBUG_TRUE
						//コンポーネントのSafePointerをNullptrにする
						PointerRegistryManager::deletePointer(component.get());
#endif 
						component->Delete();

						auto& vector = list.first;
						auto& map = list.second;

						size_t index = iter->second;
						size_t lastIndex = vector.size() - 1;

						// Move the last entity to the position of the entity to be removed
						if (index != lastIndex) {
							vector[index] = std::move(vector[lastIndex]);
							map[vector[index]->getType().c_str()] = index; // Update map for the moved entity
						}

						// Remove the last element and update map
						vector.pop_back();
						map.erase(iter);

						if (name == typeid(Renderer).name())
						{
							auto it = list.second.find(typeid(Animator).name());
							if (it != list.second.end())
							{
								auto& com = list.first[it->second];
#ifdef DEBUG_TRUE
								//コンポーネントのSafePointerをNullptrにする
								PointerRegistryManager::deletePointer(com.get());
#endif 
								com->Delete();

								auto& vector = list.first;
								auto& map = list.second;

								size_t index = iter->second;
								size_t lastIndex = vector.size() - 1;

								// Move the last entity to the position of the entity to be removed
								if (index != lastIndex) {
									vector[index] = std::move(vector[lastIndex]);
									map[vector[index]->getType().c_str()] = index; // Update map for the moved entity
								}

								// Remove the last element and update map
								vector.pop_back();
								map.erase(iter);
							}
						}
					}
				}
			}

			ImVec2 buttonPos = ImGui::GetCursorScreenPos();
			buttonPos.x += 50;
			buttonPos.y += 25;
			ImGui::SetCursorScreenPos(buttonPos);
			if (ImGui::Button("Add Component", ImVec2(300, 30)))
			{
				ImGui::OpenPopup("add component");
			}

			if(ImGui::BeginPopup("add component"))
			{
				char buf[64] = {};
				ImGui::SeparatorText("Add Component");
				ImGui::InputText("filter", buf, sizeof(buf));
				ImGui::BeginChild("addComponentWindow",ImVec2(252,200), ImGuiChildFlags_Borders);
				{
					for (auto& assembly : AssemblyComponent::assemblies)
					{
						std::string name = assembly.first.substr(6);
						if(name.find(buf, strlen(buf)) != std::string::npos)
						{
							if (ImGui::Selectable(name.c_str()))
							{
								assembly.second.addComponent(selectedObject);
								ImGui::CloseCurrentPopup();
							}
						}
					}
				}
				ImGui::EndChild();
				ImGui::EndPopup();
			}
		}
		else
		{
			ImGui::Text("not selected gameobject");
		}
		
	}
	ImGui::End();
}

const float windowWidth = SCREEN_WIDTH / 4;
const float windowHeight = SCREEN_HEIGHT / 2;

// Horizontal resize constraint callback
void HorizontalResizeConstraint(ImGuiSizeCallbackData* data) {
	data->DesiredSize.y = data->CurrentSize.y; // Fix the height
	// Optionally, set minimum or maximum horizontal sizes
	if (data->DesiredSize.x < windowWidth) {
		data->DesiredSize.x = windowWidth; // Minimum width
	}
	if (data->DesiredSize.x > windowWidth * 1.5f) {
		data->DesiredSize.x = windowWidth * 1.5f; // Maximum width
	}
}

void ImGuiApp::DrawMainGui(ImGuiContext* _mainContext)
{
	static fs::path selectPath;

	if (showMainEditor)
	{
		ImGui::SetCurrentContext(_mainContext);

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowMetricsWindow(); // メトリクスウィンドウを表示

		ImGuiIO& io = ImGui::GetIO();

		// キーボードのキー入力を設定 (必要なキーを追加)
		io.AddKeyEvent(ImGuiKey_Space, GetAsyncKeyState(VK_SPACE) & 0x8000);
		io.AddKeyEvent(ImGuiKey_Enter, GetAsyncKeyState(VK_RETURN) & 0x8000);
		io.AddKeyEvent(ImGuiKey_Escape, GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		io.AddKeyEvent(ImGuiKey_Backspace, GetAsyncKeyState(VK_BACK) & 0x8000);
		io.AddKeyEvent(ImGuiKey_LeftArrow, GetAsyncKeyState(VK_LEFT) & 0x8000);
		io.AddKeyEvent(ImGuiKey_RightArrow, GetAsyncKeyState(VK_RIGHT) & 0x8000);
		io.AddKeyEvent(ImGuiKey_UpArrow, GetAsyncKeyState(VK_UP) & 0x8000);
		io.AddKeyEvent(ImGuiKey_DownArrow, GetAsyncKeyState(VK_DOWN) & 0x8000);

		static bool aniPlay = false;
		static long long deltaCount = 0;
		static long long waitCount = 0;
		static int curFrameIndex = 0;
		static fs::path clipPath = " ";
		static bool openAnimDialog = false;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		ImGui::SetNextWindowPos(ImVec2(0, 0));  // Start at the end of game viewport
		//ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight)); // Cover the remaining area
		// Set the constraints before rendering the window
		ImGui::SetNextWindowSizeConstraints(
			ImVec2(windowWidth, windowHeight),  // Minimum size
			ImVec2(FLT_MAX, 200.0f), // Maximum size (height is fixed at 200)
			HorizontalResizeConstraint  // Constraint callback
		);
		if (ImGui::Begin("Pipe", nullptr, /*ImGuiWindowFlags_NoResize | */ImGuiWindowFlags_NoMove))
		{
			ImGui::BeginGroup();
			ImGui::SeparatorText("Control");

			int uvX = aniPlay ? 12 : 17;
			int uvY = aniPlay ? 14 : 14;
			if (ImGui::ImageButton("PlayAnimation", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				aniPlay = !aniPlay;
				if (aniPlay)
				{
					curFrameIndex = 0;
					deltaCount = 0;
					if (!imAniFrames.empty())
					{
						auto& frame = imAniFrames[curFrameIndex];
						waitCount = static_cast<long long>(frame.waitTime * 10000000);
						auto iter = textureSource.find(frame.path);
						if (iter != textureSource.end())
						{
							currentImTexture.id = iter->second.second;
							float scaleX = 1.0f / frame.splitX;
							float scaleY = 1.0f / frame.splitY;
							currentImTexture.uv0 = ImVec2(scaleX * frame.uvX, scaleY * frame.uvY);
							currentImTexture.uv1 = ImVec2(scaleX * (frame.uvX + 1), scaleY * (frame.uvY + 1));
						}
					}
				}
			}
			ImGui::SetItemTooltip(aniPlay ? "stop" : "play");

			ImGui::SameLine();
			
			ImGui::BeginGroup();
			ImGui::Text(clipPath.filename().string().c_str());
			uvX = 9;
			uvY = 14;
			if (ImGui::ImageButton("writeAnimation", imIconTexture, ImVec2(25, 25), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				if (clipPath != " ")
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
					SaveAnimationClipFile(clipPath.string());
				}
				else
					openAnimDialog = true;
			}
			ImGui::SetItemTooltip("save");

			ImGui::SameLine();
			uvX = 10;
			uvY = 14;
			if (ImGui::ImageButton("OverwriteAnimation", imIconTexture, ImVec2(25, 25), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				openAnimDialog = true;
			}
			ImGui::SetItemTooltip("over save");

			ImGui::SameLine();
			uvX = 16;
			uvY = 14;
			if (ImGui::ImageButton("quickAnimation", imIconTexture, ImVec2(25, 25), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				if (!imAniFrames.empty())
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
					if (curFrameIndex >= 0 && curFrameIndex < (int)imAniFrames.size())
					{
						float& t = imAniFrames[curFrameIndex].waitTime;
						for (auto& frame : imAniFrames)
						{
							frame.waitTime = t;
						}
					}
				}
			}
			ImGui::SetItemTooltip("all change");

			ImGui::SameLine();
			uvX = 1;
			uvY = 15;
			if (ImGui::ImageButton("DeleteAnimation", imIconTexture, ImVec2(25, 25), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				if (!aniPlay)
				{
					imAniFrames.clear();
				}
			}
			ImGui::SetItemTooltip("delete");

			ImGui::SameLine();
			uvX = 0;
			uvY = 17;
			if (ImGui::ImageButton("SortAnimation", imIconTexture, ImVec2(25, 25), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				if (!imAniFrames.empty())
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
					std::sort(imAniFrames.begin(), imAniFrames.end(),
						[](const ImAnimationFrame& a, const ImAnimationFrame& b) {
							return (a.uvX + a.uvY * maxTexSplit[0]) < (b.uvX + b.uvY * maxTexSplit[0]);
						});
				}
			}
			ImGui::SetItemTooltip("sort");
			ImGui::EndGroup();
			
			if (!imAniFrames.empty())
				ImGui::DragFloat("time", &imAniFrames[curFrameIndex].waitTime, 0.01f, 0.0f, 5.0f);
			else
			{
				float dummy = 1.0f;
				ImGui::DragFloat("time", &dummy, 0.01f, 0.0f, 5.0f);
			}
			ImGui::SeparatorText("frame");
			if (ImGui::BeginChild("frameList"))
			{
				int eraseNum = -1;
				for (int num = 0;num < (int)imAniFrames.size();num++)
				{
					auto& frame = imAniFrames[num];

					ImGui::PushID(num);

					ImGui::BeginGroup();
					
					if (ImGui::Button("-") && !aniPlay)
					{
						eraseNum = num;
					}
					ImGui::SetItemTooltip("erase");
					ImGui::SameLine();
					ImGui::Text("%d.", num);
					ImGui::SameLine();
					std::string str = frame.path.filename().string() + " x :" + std::to_string(frame.uvX + 1) + " y : " + std::to_string(frame.uvY + 1);
					if (ImGui::Selectable(str.c_str(), num == curFrameIndex))
					{
						auto it = textureSource.find(frame.path);
						if (it != textureSource.end())
						{
							curFrameIndex = num;
							currentImTexture.id = it->second.second;
							float scaleX = 1.0f / frame.splitX;
							float scaleY = 1.0f / frame.splitY;
							currentImTexture.uv0 = ImVec2(scaleX * frame.uvX, scaleY * frame.uvY);
							currentImTexture.uv1 = ImVec2(scaleX * (frame.uvX + 1), scaleY * (frame.uvY + 1));
						}
					}
					// Source: Dragging starts here
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						ImGui::SetDragDropPayload("DND_LIST_ITEM", &num, sizeof(int)); // Pass the index
						ImGui::Text("%s", str.c_str());
						ImGui::EndDragDropSource();
					}
					ImGui::EndGroup();
					// Target: Dropping ends here
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_LIST_ITEM")) {
							size_t dragged_index = *(const size_t*)payload->Data;
							if (dragged_index != num) {
								// Move the dragged item to the target index
								curFrameIndex = num;
								auto dragged_item = imAniFrames[dragged_index];
								imAniFrames.erase(imAniFrames.begin() + dragged_index);
								imAniFrames.insert(imAniFrames.begin() + num, dragged_item);
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::PopID();
				}
				if (eraseNum >= 0 && eraseNum < (int)imAniFrames.size())
				{
					imAniFrames.erase(imAniFrames.begin() + eraseNum);
				}

				ImGui::EndChild();

				if (aniPlay && !imAniFrames.empty())
				{
					deltaCount += AnimatorManager::deltaCount;
					if (deltaCount > waitCount)
					{
						deltaCount -= waitCount;
						curFrameIndex++;
						curFrameIndex %= (int)imAniFrames.size();
						auto& frame = imAniFrames[curFrameIndex];
						waitCount = static_cast<long long>(frame.waitTime * 10000000);
						auto iter = textureSource.find(frame.path);
						if (iter != textureSource.end())
						{
							currentImTexture.id = iter->second.second;
							float scaleX = 1.0f / frame.splitX;
							float scaleY = 1.0f / frame.splitY;
							currentImTexture.uv0 = ImVec2(scaleX * frame.uvX, scaleY * frame.uvY);
							currentImTexture.uv1 = ImVec2(scaleX * (frame.uvX + 1), scaleY * (frame.uvY + 1));
						}
					}
				}
			}

			ImGui::EndGroup();
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CUT_TEXTURE_DRAG"))
				{
					IM_UNUSED(payload);
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);

					imAniFrames.insert(imAniFrames.end(), std::make_move_iterator(willPushFrames.begin()), std::make_move_iterator(willPushFrames.end()));
					willPushFrames.clear(); // vec2 is now empty (optional, since moved-from state is valid but unspecified)
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(SCREEN_WIDTH - windowWidth, 0));  // Start at the end of game viewport
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight)); // Cover the remaining area
		if (ImGui::Begin("Source", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::SeparatorText("Link");

			int uvX = 14;
			int uvY = 6;
			if (ImGui::ImageButton("LoadSource", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				handleUi.SetUploadFile("texture", [](GameObject* _obj, fs::path _path)
					{
						auto iter = textureSource.find(_path);
						if (iter != textureSource.end()) return;

						ComPtr<ID3D11ShaderResourceView> texture;
						TextureAssets::pLoadTexture(texture, _path.wstring().c_str());
						ImTextureID texId = (ImTextureID)texture.Get();
						textureSource.emplace(
							std::make_pair(_path, std::make_pair(texture, texId)));
					},
					{ ".png",".jpg" ,".dds" });
			}
			ImGui::SetItemTooltip("load texture");

			ImGui::SameLine();
			uvX = 15;
			uvY = 6;
			if (ImGui::ImageButton("LoadClip", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
			{
				handleUi.SetUploadFile("animation clip",
					[](GameObject* obj, fs::path _path)
					{
						ReadAnimationClipFile(_path);
						clipPath = _path;
						for (auto& frame : imAniFrames)
						{
							auto iterator = textureSource.find(frame.path);
							if (iterator == textureSource.end())
							{
								ComPtr<ID3D11ShaderResourceView> texture;
								TextureAssets::pLoadTexture(texture, frame.path.wstring().c_str());
								ImTextureID texId = (ImTextureID)texture.Get();
								textureSource.emplace(
									std::make_pair(frame.path, std::make_pair(texture, texId)));
							}

							int splitIndex = frame.splitY * maxTexSplit[0] + frame.splitX;
							auto iter = textureCutNode.find(frame.path);
							if (iter != textureCutNode.end())
							{
								auto it = iter->second.find(splitIndex);
								if (it == iter->second.end())
								{
									iter->second.insert(splitIndex);
								}
							}
							else
							{
								std::set<int> cuts;
								cuts.emplace(splitIndex);
								textureCutNode.insert(std::make_pair(frame.path, std::move(cuts)));
							}
						}
					}, { ANIM_CLIP_EXTENSION_DOT});
			}
			ImGui::SetItemTooltip("load clip");

			ImGui::SameLine();
			uvX = 9;
			uvY = 15;
			ImGui::Image(imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_SOURCE_DRAG"))
				{
					IM_UNUSED(payload);
					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
					fs::path path(std::string(static_cast<const char*>(payload->Data), payload->DataSize));
					auto iter = textureSource.find(path);
					if (iter != textureSource.end())
					{
						textureSource.erase(iter);
					}
					auto it = textureCutNode.find(path);
					if (it != textureCutNode.end())
					{
						textureCutNode.erase(it);
					}
					if (selectPath == path) selectPath = "";
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SetItemTooltip("death texture");

			ImGui::SeparatorText("Files");

			ImGui::BeginChild("textures");
			{
				bool endl = false;
				for (auto& sourceTex : textureSource)
				{
					if (endl) ImGui::SameLine();

					std::string path_str = sourceTex.first.string();
					if (ImGui::ImageButton(path_str.c_str(), sourceTex.second.second, ImVec2(100, 100), { 0,0 }, { 1,1 }, windowBgCol))
					{
						currentImTexture.id = sourceTex.second.second;
						currentImTexture.uv0 = { 0,0 };
						currentImTexture.uv1 = { 1,1 };
					}
					ImGui::SetItemTooltip(path_str.c_str());
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						ImGui::SetDragDropPayload("TEXTURE_SOURCE_DRAG", path_str.c_str(), path_str.size());
						ImGui::Text("%s", path_str.c_str());
						ImGui::EndDragDropSource();
					}
					endl = !endl;
				}
		
			}
			ImGui::EndChild();

		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(0, windowHeight));  // Start at the end of game viewport
		ImGui::SetNextWindowSize(ImVec2(SCREEN_WIDTH, windowHeight)); // Cover the remaining area
		if (ImGui::Begin("Work", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			static int split[2] = { 1,1 };

			ImGui::BeginChild("AnimationFrameList", ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, ImGui::GetContentRegionAvail().y));
			{
				ImGui::SeparatorText("Cut Node");

				int uvX = 0;
				int uvY = 0;
				std::vector<fs::path> eraseNodes;
				for (auto& node : textureCutNode)
				{
					auto iter = textureSource.find(node.first);
					if (iter == textureSource.end()) continue;

					if (ImGui::TreeNodeEx(node.first.string().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
					{
						std::string filename = node.first.filename().string();

						std::vector<int> eraseIndex;
						ImTextureID texId = iter->second.second;
						int cutCount = 0;
						for (auto& cuts : node.second)
						{
							int splitX = cuts % maxTexSplit[0];
							int splitY = cuts / maxTexSplit[0];
							float texScaleX = 1.0f / splitX;
							float texScaleY = 1.0f / splitY;
							int count = 0;
							std::string label = std::to_string(splitX) + " : " + std::to_string(splitY);
							//ImGui::BulletText("%d : %d", splitX, splitY);
							bool treeOpen = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
							if (treeOpen)
							{
								ImGui::SameLine();
								std::string splitId = filename + std::to_string(splitX + splitY * maxTexSplit[0]);
								ImGui::PushID(splitId.c_str());
								if (ImGui::Button(" erase "))
								{
									eraseIndex.push_back(cuts);
								}
								ImGui::PopID();

								for (int y = 0; y < splitY; y++)
								{
									ImGui::BulletText("%d", y);
									if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
									{
										willPushFrames.clear();
										for (int x = 0; x < splitX; x++)
										{
											willPushFrames.push_back(
												{
													node.first,splitX,splitY,x,y
												}
											);
										}

										ImGui::SetDragDropPayload("CUT_TEXTURE_DRAG", filename.c_str(), filename.size());
										ImGui::Text("x : %d y : %d", filename.c_str(), splitX, splitY);
										ImGui::EndDragDropSource();
									}
									count = 0;
									for (int x = 0; x < splitX; x++)
									{
										if (count % 10 != 0) ImGui::SameLine();
										count++;
										std::string id = filename + std::to_string(cutCount) + std::to_string(x + y * maxTexSplit[0]);
										ImGui::PushID(id.c_str());
										if (ImGui::ImageButton("cutTexButton", texId, ImVec2(52, 52), ImVec2(texScaleX * x, texScaleY * y), ImVec2(texScaleX * (x + 1), texScaleY * (y + 1)), windowBgCol))
										{
											currentImTexture.id = texId;
											currentImTexture.uv0 = ImVec2(texScaleX * x, texScaleY * y);
											currentImTexture.uv1 = ImVec2(texScaleX * (x + 1), texScaleY * (y + 1));
										}
										ImGui::SetItemTooltip("%s x : %d y :%d", filename.c_str(), x + 1, y + 1);
										if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
										{
											willPushFrames.clear();
											willPushFrames.push_back(
												{ node.first,splitX,splitY,x,y
												}
											);
											ImGui::SetDragDropPayload("CUT_TEXTURE_DRAG", id.c_str(), id.size());
											ImGui::Text("%s", filename.c_str());
											ImGui::EndDragDropSource();
										}
										ImGui::PopID();
									}
									cutCount++;
								}
								ImGui::TreePop();
							}
							if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
							{
								willPushFrames.clear();
								for (int x = 0; x < splitX; x++)
								{
									for (int y = 0; y < splitY; y++)
									{
										willPushFrames.push_back(
											{
												node.first,splitX,splitY,x,y
											}
										);
									}
								}

								ImGui::SetDragDropPayload("CUT_TEXTURE_DRAG", filename.c_str(), filename.size());
								ImGui::Text("x : %d y : %d", filename.c_str(), splitX, splitY);
								ImGui::EndDragDropSource();
							}
							if (!treeOpen)
							{
								ImGui::SameLine();
								std::string splitId = filename + std::to_string(splitX + splitY * maxTexSplit[0]);
								ImGui::PushID(splitId.c_str());
								if (ImGui::Button(" erase "))
								{
									eraseIndex.push_back(cuts);
								}
								ImGui::PopID();
							}
						}

						for (auto& i : eraseIndex)
						{
							auto it = node.second.find(i);
							if (it != node.second.end())
							{
								node.second.erase(it);
								if (node.second.empty())
									eraseNodes.push_back(node.first);
							}
						}

						ImGui::TreePop();
					}
				}
				for (auto& path : eraseNodes)
				{
					auto iter = textureCutNode.find(path);
					if (iter != textureCutNode.end())
					{
						textureCutNode.erase(iter);
					}
				}
			}
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild("Capture", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
			{			
				ImGui::SeparatorText("tool");
				ImGui::BeginGroup();
				{
					int uvX = 8;
					int uvY = 14;
					if (ImGui::ImageButton("cutTexButton", imIconTexture, ImVec2(50, 50), ImVec2(iconTexScale * uvX, iconTexScale * uvY), ImVec2(iconTexScale * (uvX + 1), iconTexScale * (uvY + 1)), windowBgCol))
					{
						if (selectPath != "")
						{
							int splitIndex = split[1] * maxTexSplit[0] + split[0];
							auto iter = textureCutNode.find(selectPath);
							if (iter != textureCutNode.end())
							{
								auto it = iter->second.find(splitIndex);
								if (it == iter->second.end())
								{
									iter->second.insert(splitIndex);
								}
							}
							else
							{
								std::set<int> cuts;
								cuts.emplace(splitIndex);
								textureCutNode.insert(std::make_pair(selectPath, std::move(cuts)));
							}
						}

					}
					ImGui::SetItemTooltip("cut texture");
					ImGui::SameLine();
					ImGui::BeginGroup();
					{
						ImGui::Text("split : %s", selectPath.filename().string().c_str());
						if (ImGui::DragInt("x", split, 0.5f, 1, maxTexSplit[0]))
						{

						}
						ImGui::DragInt("y", split + 1, 0.5f, 1, maxTexSplit[1]);
					}
					ImGui::EndGroup();

					//ImGui::SeparatorText("view");
					ImGui::Image(currentImTexture.id, ImVec2(240, 240), currentImTexture.uv0, currentImTexture.uv1);
				}
				ImGui::EndGroup();
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_SOURCE_DRAG"))
					{
						IM_UNUSED(payload);
						ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
						fs::path path(std::string(static_cast<const char*>(payload->Data), payload->DataSize));
						auto iter = textureSource.find(path);
						if (iter != textureSource.end())
						{
							currentImTexture.id = iter->second.second;
							currentImTexture.uv0 = { 0,0 };
							currentImTexture.uv1 = { 1,1 };
						}
						selectPath = std::move(path);
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();

		ImGui::PopStyleVar();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (openAnimDialog)
		{
			if (!imAniFrames.empty())
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
				SaveAnimationClipFileDialog();
			}
			openAnimDialog = false;
		}
	}
}

void ImGuiApp::SetSelectedObject(GameObject* _object)
{
	selectedObject = _object;
	if (selectedObject != nullptr)selectedObject->isSelected = GameObject::SELECTED;
}

void ImGuiApp::CopySelectedObject()
{
	if (selectedObject != nullptr)
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
		ObjectManager::Copy(selectedObject);
	}
}

void ImGuiApp::DeleteSelectedObject()
{
	//オブジェクトを削除する
	if (selectedObject != nullptr /* && ImGui::IsKeyPressed(ImGuiKey_Delete)*/)
	{
		auto& list = ObjectManager::m_currentList;
		auto iter = list->second.find(selectedObject->name);
		if (iter != list->second.end())
		{
			if (Window::IsPause())
			{
				willDeleteObjects.push(std::make_pair(std::move(list->first[iter->second]), std::move(*iter)));
				willDeleteObjects.top().first->SetActive(false);

				changes.emplace([]() {
					if (!willDeleteObjects.empty())
					{
						auto& object = willDeleteObjects.top();
						if (selectedObject != nullptr)
						{
							selectedObject->isSelected = GameObject::SELECT_NONE;
						}
						selectedObject = object.first.get();
						object.first->isSelected = GameObject::SELECTED;
						object.first->SetActive(true);
						ObjectManager::m_currentList->first.emplace_back(std::move(willDeleteObjects.top().first));
						ObjectManager::m_currentList->second.emplace(std::move(willDeleteObjects.top().second));
						willDeleteObjects.pop();
					}
					});
			}
			else
			{
				PointerRegistryManager::deletePointer(list->first[iter->second].get());
			}
			//list->erase(iter);
			ObjectManager::DeleteObject(iter);
			selectedObject = nullptr;
			
		}
	}
}

void ImGuiApp::ClearStack()
{
	updateValues.clear();
	while (!willRemoveComponents.empty())
	{
		willRemoveComponents.top()->Delete();

#ifdef DEBUG_TRUE
		PointerRegistryManager::deletePointer(willRemoveComponents.top().get());
#endif
		willRemoveComponents.pop();
	}

	while (!willDeleteObjects.empty())
	{
#ifdef DEBUG_TRUE
		PointerRegistryManager::deletePointer(willDeleteObjects.top().first.get());
#endif
		willDeleteObjects.pop();
	}
}

bool ImGuiApp::IsSceneChange()
{
	if (!updateValues.empty()) return true;
	if (!willRemoveComponents.empty()) return true;
	if (!willDeleteObjects.empty()) return true;
	return false;
}

void ImGuiApp::RewindChange()
{
	if (changes.empty()) return;

	changes.top()();
	changes.pop();
}

bool ImGuiApp::UpdateHandleUI(Vector2 _targetPos)
{
	if (!handleUi.lock)
		return handleUi.Update(_targetPos);

	return false;
}

void ImGuiApp::DrawHandleUI(const Vector2& _targetPos)
{
	if (!handleUi.lock)
		handleUi.Draw(selectedObject, _targetPos);
}

void ImGuiApp::InvalidSelectedObject()
{
	if (selectedObject != nullptr)selectedObject->isSelected = GameObject::SELECT_NONE;
	selectedObject = nullptr;
	handleUi.SetUploadFile("", {}, {""});
}

void ImGuiApp::SaveSceneFileDialog(fs::path& _path)
{
	// Initialize the OPENFILENAME structure
	OPENFILENAME ofn;
	char szFile[MAX_PATH] = ""; // Buffer for the file name
	ZeroMemory(&ofn, sizeof(ofn));

	// Set up the OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd[OPTIONS];
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Scene Files(*.json)\0*.json\0";//\0All Files (*.*)\0*.*\0";
	ofn.lpstrDefExt = "json"; // Default file extension
	ofn.lpstrTitle = "Save Scene";
	ofn.Flags = OFN_OVERWRITEPROMPT; // Prompt to overwrite existing files

	// 現在のカレントディレクトリを保存
	auto originalPath = std::filesystem::current_path();

	// Show the Save File dialog
	if (GetSaveFileName(&ofn)) {
		std::string filePath = szFile;
		_path = filePath;
	}
	else {
		std::cerr << "No file selected or operation canceled." << std::endl;
	}
	// カレントディレクトリを元に戻す
	std::filesystem::current_path(originalPath);
}

void ImGuiApp::UnInit()
{
	ClearStack();

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

#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
			// Keyboard events
		case WM_KEYDOWN:
			if (wParam < 256)
			{
				io.KeysDown[wParam] = 1;
			}
			break;
		case WM_KEYUP:
			if (wParam < 256)
			{
				io.KeysDown[wParam] = 0;
			}
			break;
#endif

		case WM_CHAR:
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			break;
		}
	}

	// Pass any unhandled messages to the default window procedure
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void SaveAnimationClipFile(const std::string& _path)
{
	//書き込みファイルを開く
	std::ofstream fout;
	fout.open(_path, std::ios::out | std::ios::binary);
	assert(fout.is_open() && "アニメーションファイル書き込みに失敗しました");

	//要素の数を書き込み
	int listSize = static_cast<int>(imAniFrames.size());
	fout.write((char*)&listSize, sizeof(listSize));

	for (auto& frame : imAniFrames)
	{
		std::string path = frame.path.string();
		//パスの大きさ
		int pathSize = static_cast<int>(path.size());
		fout.write((char*)&pathSize, sizeof(pathSize));
		//パス本体
		fout.write((char*)path.c_str(), pathSize);
		//splitX
		fout.write((char*)&frame.splitX, sizeof(frame.splitX));
		//splitY
		fout.write((char*)&frame.splitY, sizeof(frame.splitY));
		//uvX
		fout.write((char*)&frame.uvX, sizeof(frame.uvX));
		//uvY
		fout.write((char*)&frame.uvY, sizeof(frame.uvY));
		//waitCout
		long long waitCount = static_cast<long long>(frame.waitTime * 10000000);
		fout.write((char*)&waitCount, sizeof(waitCount));
	}

	//ファイルを閉じる
	fout.close();
}

void SaveAnimationClipFileDialog() {
	// Initialize the OPENFILENAME structure
	OPENFILENAME ofn;
	char szFile[MAX_PATH] = ""; // Buffer for the file name
	ZeroMemory(&ofn, sizeof(ofn));

	// Set up the OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = Window::GetMainHWnd();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "AnimationClip Files(*." ANIM_CLIP_EXTENSION ")\0*." ANIM_CLIP_EXTENSION "\0";//\0All Files (*.*)\0*.*\0";
	ofn.lpstrDefExt = ANIM_CLIP_EXTENSION; // Default file extension
	ofn.lpstrTitle = "Save AnimationClip";
	ofn.Flags = OFN_OVERWRITEPROMPT; // Prompt to overwrite existing files

	// 現在のカレントディレクトリを保存
	auto originalPath = std::filesystem::current_path();

	// Show the Save File dialog
	if (GetSaveFileName(&ofn)) {
		std::string filePath = szFile;
		SaveAnimationClipFile(filePath);
	}
	else {
		std::cerr << "No file selected or operation canceled." << std::endl;
	}

	// カレントディレクトリを元に戻す
	std::filesystem::current_path(originalPath);
}

void ReadAnimationClipFile(fs::path _path)
{
	//読み込みファイルを開く
	std::ifstream fin;
	std::string filePath = _path.string();
	fin.open(filePath, std::ios::in | std::ios::binary);

	bool open = fin.is_open();
	//読み込み失敗
	if (open)
	{
		//リストをクリアする
		imAniFrames.clear();

		//プレイヤーの数を読み込む
		int size = 0;
		fin.read((char*)&size, sizeof(size));

		for (int i = 0; i < size; i++)
		{
			//パスの大きさを読み込み
			int nameSize = 0;
			fin.read((char*)&nameSize, sizeof(nameSize));

			ImAnimationFrame frame;

			//パスを読み込み
			std::string path;
			path.resize(nameSize);
			fin.read((char*)path.c_str(), nameSize);
			fs::path texPath(path);
			frame.path = std::move(texPath);
			//splitX
			fin.read((char*)&frame.splitX, sizeof(frame.splitX));
			//splitY
			fin.read((char*)&frame.splitY, sizeof(frame.splitY));
			//uvX
			fin.read((char*)&frame.uvX, sizeof(frame.uvX));
			//uvY
			fin.read((char*)&frame.uvY, sizeof(frame.uvY));
			//waitCout
			long long waitCount;
			fin.read((char*)&waitCount, sizeof(waitCount));
			frame.waitTime = static_cast<float>(static_cast<double>(waitCount) / 10000000);

			imAniFrames.emplace_back(frame);
		}

		//読み込みファイルを閉じる
		fin.close();
	}
	else
	{
		std::cerr << "アニメーションクリップ読み込み失敗" << std::endl;
	}
}

ComPtr<ID3D11Buffer> ImGuiApp::HandleUI::m_arrowVertexBuffer;
ComPtr<ID3D11Buffer> ImGuiApp::HandleUI::m_arrowIndexBuffer;

HRESULT ImGuiApp::HandleUI::Init()
{
	if (m_arrowVertexBuffer.Get() != nullptr) return S_OK;

	HRESULT  hr;

	const auto dos = DEFAULT_OBJECT_SIZE;
	const auto qDos = QUARTER_OBJECT_SIZE;

	Vertex vertexList[] =
	{
		{ 0.0f,	  0.0f,			0.5f,	1.0f,1.0f,1.0f,1.0f,	0.0f,0.0f},
		{ 0.0f,	  dos,			0.5f,	1.0f,1.0f,1.0f,1.0f,	1.0f,0.0f},
		{-qDos,	  dos,			0.5f,	1.0f,1.0f,1.0f,1.0f,	0.0f,1.0f},
		{ 0.0f,	  dos + qDos,	0.5f,	1.0f,1.0f,1.0f,1.0f,	1.0f,1.0f},
		{ qDos,	  dos,			0.5f,	1.0f,1.0f,1.0f,1.0f,	1.0f,1.0f}
	};

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertexList);// 確保するバッファサイズを指定
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// 頂点バッファ作成を指定
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = vertexList;// VRAMに送るデータを指定
	subResourceData.SysMemPitch = 0;
	subResourceData.SysMemSlicePitch = 0;

	hr = DirectX11::m_pDevice->CreateBuffer(&bufferDesc, &subResourceData, m_arrowVertexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "頂点バッファー作成失敗", "エラー", MB_ICONERROR | MB_OK);
		return hr;
	}

	WORD indexList[]{
		0,1,1,2,2,3,3,4,4,1
	};

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = sizeof(indexList);
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA irData;
	irData.pSysMem = indexList;
	irData.SysMemPitch = 0;
	irData.SysMemSlicePitch = 0;

	hr = DirectX11::m_pDevice->CreateBuffer(&ibDesc, &irData, m_arrowIndexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "インデックスバッファー作成失敗", "エラー", MB_ICONERROR | MB_OK);

		return hr;
	}

	return S_OK;
}

// Function to calculate the squared distance between two points
float squaredDistance(const Vector2& p1, const Vector2& p2) {
	return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

// Function to project a point onto a line segment and return the closest point
Vector2 closestPointOnSegment(const Vector2& p, const Vector2& a, const Vector2& b) {
	Vector2 ab = { b.x - a.x, b.y - a.y };
	Vector2 ap = { p.x - a.x, p.y - a.y };

	float abSquared = ab.x * ab.x + ab.y * ab.y;
	float projection = (ap.x * ab.x + ap.y * ab.y) / abSquared;

	// Clamp the projection to the [0, 1] range
	projection = (std::max)(0.0f, (std::min)(1.0f, projection));

	return { a.x + projection * ab.x, a.y + projection * ab.y };
}

// Function to find the closest intersection
Vector2 closestIntersection(const Vector2& p, const Vector2 quad[4]) {
	float minDist = (std::numeric_limits<float>::max)();
	Vector2 closestPoint;

	// Iterate through each edge of the quadrilateral
	for (int i = 0; i < 4; ++i) {
		Vector2 a = quad[i];
		Vector2 b = quad[(i + 1) % 4]; // Wrap around to the first vertex

		// Find the closest point on the edge
		Vector2 candidate = closestPointOnSegment(p, a, b);

		// Update the closest point if the distance is smaller
		float dist = squaredDistance(p, candidate);
		if (dist < minDist) {
			minDist = dist;
			closestPoint = candidate;
		}
	}

	return closestPoint;
}

float moveScaleRad;

bool ImGuiApp::HandleUI::Update(Vector2 _targetPos)
{
	if (selectedObject == nullptr) return false;

	const float scaleX = HALF_OBJECT_SIZE / RenderManager::renderZoom.x;
	const float scaleY = HALF_OBJECT_SIZE / RenderManager::renderZoom.y;
	const float sizeX = QUARTER_OBJECT_SIZE * scaleX;
	const float sizeY = QUARTER_OBJECT_SIZE * scaleY;

	switch (handleMode)
	{
	case POSITION:
	{
		static Vector2 offset;
		static Vector2 beforePos;

		switch (moveMode)
		{
		case NONE:
		case VERTICAL_POS_ON_MOUSE:
		case HORIZON_POS_ON_MOUSE:
		case MOVE_POS_ON_MOUSE:
		{
			Vector2 objectPos = selectedObject->transform.position;
			objectPos.y += DEFAULT_OBJECT_SIZE * scaleY;

			if ((objectPos.x - sizeX) < _targetPos.x &&
				(objectPos.x + sizeX) > _targetPos.x &&
				(objectPos.y - sizeY) < _targetPos.y &&
				(objectPos.y + sizeY) > _targetPos.y)
			{
				moveMode = VERTICAL_POS_ON_MOUSE;
				if (Input::Get().MouseLeftTrigger())
				{
					moveMode = VERTICAL_POS;
					beforePos = selectedObject->transform.position;
					offset = (Vector2)selectedObject->transform.position - _targetPos;
				}
				return true;
			}
			objectPos.y = selectedObject->transform.position.y;
			objectPos.x += DEFAULT_OBJECT_SIZE * scaleX;

			if ((objectPos.x - sizeX) < _targetPos.x &&
				(objectPos.x + sizeX) > _targetPos.x &&
				(objectPos.y - sizeY) < _targetPos.y &&
				(objectPos.y + sizeY) > _targetPos.y)
			{
				moveMode = HORIZON_POS_ON_MOUSE;
				if (Input::Get().MouseLeftTrigger())
				{
					moveMode = HORIZON_POS;
					beforePos = selectedObject->transform.position;
					offset = (Vector2)selectedObject->transform.position - _targetPos;
				}
				return true;
			}
			objectPos.x = selectedObject->transform.position.x;
			objectPos.x += QUARTER_OBJECT_SIZE * scaleX;
			objectPos.y += QUARTER_OBJECT_SIZE * scaleY;

			if ((objectPos.x - sizeX) < _targetPos.x &&
				(objectPos.x + sizeX) > _targetPos.x &&
				(objectPos.y - sizeY) < _targetPos.y &&
				(objectPos.y + sizeY) > _targetPos.y)
			{
				moveMode = MOVE_POS_ON_MOUSE;
				if (Input::Get().MouseLeftTrigger())
				{
					moveMode = MOVE_POS;
					beforePos = selectedObject->transform.position;
					offset = (Vector2)selectedObject->transform.position - _targetPos;
				}
				return true;
			}

			moveMode = NONE;
		}
		break;
		case VERTICAL_POS:
		{
			//if (selectedObject != nullptr)
			{
				selectedObject->transform.position.y = _targetPos.y + offset.y;
			}
			if (Input::Get().MouseLeftRelease())
			{
				changes.emplace(std::bind([](GameObject* obj, Vector2 pos,std::function<void()> func) {
					if (obj != nullptr)
					{
						obj->transform.position = pos;
						func();
					}
					}, selectedObject, beforePos,std::move(SetChangeValue(selectedObject, moveMode))));
				moveMode = NONE;
			}

			return true;
		}
		break;
		case HORIZON_POS:
		{
			//if (selectedObject != nullptr)
			{
				selectedObject->transform.position.x = _targetPos.x + offset.x;
			}
			if (Input::Get().MouseLeftRelease())
			{
				changes.emplace(std::bind([](GameObject* obj, Vector2 pos, std::function<void()> func) {
					if (obj != nullptr)
					{
						obj->transform.position = pos;
						func();
					}
					}, selectedObject, beforePos, std::move(SetChangeValue(selectedObject, moveMode))));
				moveMode = NONE;
			}

			return true;
		}
		break;
		case MOVE_POS:
		{
			//if (selectedObject != nullptr)
			{
				selectedObject->transform.position = _targetPos + offset;
			}
			if (Input::Get().MouseLeftRelease())
			{
				changes.emplace(std::bind([](GameObject* obj, Vector2 pos, std::function<void()> func) {
					if (obj != nullptr)
					{
						obj->transform.position = pos;
						func();
					}
					}, selectedObject, beforePos, std::move(SetChangeValue(selectedObject, moveMode))));
				moveMode = NONE;
			}

			return true;
		}
		break;
		}		
	}
	break;
	case ROTATION:
	{
		static Vector2 oldPos;
		static double beforeRad;

		switch (moveMode)
		{
		case NONE:
		case MOVE_ROTATION_ON_MOUSE:
		{
			const Vector2 center = selectedObject->transform.position;
			Vector2 dis = _targetPos - center;
			float length = 
				sqrtf(powf(dis.x * RenderManager::renderZoom.x, 2) + powf(dis.y * RenderManager::renderZoom.y, 2));
			const float radiusX = HALF_OBJECT_SIZE * HALF_OBJECT_SIZE;
			if(length > radiusX * 1.5 && length < radiusX * 2.5)
			{
				moveMode = MOVE_ROTATION_ON_MOUSE;
				if (Input::Get().MouseLeftTrigger())
				{
					moveMode = MOVE_ROTATION;
					oldPos = _targetPos;
					beforeRad = selectedObject->transform.angle.z.Get();
				}

				return true;
			}
			moveMode = NONE;
		}
		break;
		case MOVE_ROTATION:
		{
			//if (selectedObject != nullptr)
			{
				Vector2 dis = _targetPos - oldPos;
				const auto& center = selectedObject->transform.position;
				float rad = atan2f(_targetPos.y - center.y, _targetPos.x - center.x); // Calculate angle in radians
				selectedObject->transform.angle.z += 
					dis.x * -sin(rad) * RenderManager::renderZoom.x + dis.y * cos(rad) * RenderManager::renderZoom.y;
				oldPos = _targetPos;
			}
			if (Input::Get().MouseLeftRelease())
			{
				changes.emplace(std::bind([](GameObject* obj, double rad, std::function<void()> func) {
					if (obj != nullptr)
					{
						obj->transform.angle.z.Set(rad);
						func();
					}
					}, selectedObject, beforeRad, std::move(SetChangeValue(selectedObject, moveMode))));
				moveMode = NONE;
			}

			return true;
		}
		break;
		}
	}
	break;
	case SCALE:
	{
		static Vector2 oldPos;
		static Vector2 beforeScale;

		switch (moveMode)
		{
		case NONE:
		case MOVE_SCALE_ON_MOUSE:
		{
			const auto& center = selectedObject->transform.position;
			Vector2 size = selectedObject->transform.scale * HALF_OBJECT_SIZE * 1.2f;
		/*	size.x /= RenderManager::renderZoom.x;
			size.y /= RenderManager::renderZoom.y;*/
			Vector2 quad[4] =
			{
				{center.x - size.x,center.y + size.y},
				{center.x + size.x,center.y + size.y},
				{center.x + size.x,center.y - size.y},
				{center.x - size.x,center.y - size.y}
			};
			Vector2 closePos = closestIntersection(_targetPos, quad);
			Vector2 dis = _targetPos - closePos;
			float length = sqrtf(powf(dis.x * RenderManager::renderZoom.x, 2) + powf(dis.y * RenderManager::renderZoom.y, 2));
			if (length < 10)
			{
				moveMode = MOVE_SCALE_ON_MOUSE;
				moveScaleRad = atan2f(closePos.y / size.y - center.y / size.y, closePos.x / size.x - center.x / size.x);
				moveScaleRad = fmod(Math::PI2 + moveScaleRad, Math::PI2);
				moveScaleRad = (int)((moveScaleRad + Math::qPI / 2) / Math::qPI) * Math::qPI;
				if (Input::Get().MouseLeftTrigger())
				{
					moveMode = MOVE_SCALE;
					oldPos = _targetPos;
					beforeScale = selectedObject->transform.scale;
				}

				return true;
			}
			moveMode = NONE;
		}
		break;
		case MOVE_SCALE:
		{
			Vector2 dis = _targetPos - oldPos;
			auto& scale = selectedObject->transform.scale;
			scale.x += (dis.x * cos(moveScaleRad)) / (HALF_OBJECT_SIZE/*/ RenderManager::renderZoom.x*/);
			scale.y += (dis.y * sin(moveScaleRad)) / (HALF_OBJECT_SIZE /*/ RenderManager::renderZoom.y*/);
			oldPos = _targetPos;

			if (Input::Get().MouseLeftRelease())
			{
				changes.emplace(std::bind([](GameObject* obj, Vector2 scale, std::function<void()> func) {
					if (obj != nullptr)
					{
						obj->transform.scale = scale;
						func();
					}
					}, selectedObject, beforeScale, std::move(SetChangeValue(selectedObject, moveMode))));
				moveMode = NONE;
			}

			return true;
		}
		break;
		}
	}
	break;
	}
	return false;
}

void ImGuiApp::HandleUI::Draw(GameObject* _target,const Vector2 _targetPos)
{
	if (_target == nullptr) return;

	// 描画先のキャンバスと使用する深度バッファを指定する
	DirectX11::m_pDeviceContext->OMSetRenderTargets(1,
		DirectX11::m_pRenderTargetViewList[Window::GetMainHWnd()].first.GetAddressOf(), DirectX11::m_pDepthStencilView.Get());

	RenderManager::SetMainCameraMatrix();

	DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	//テクスチャをピクセルシェーダーに渡す
	DirectX11::m_pDeviceContext->PSSetShaderResources(0, 1, DirectX11::m_pTextureView.GetAddressOf());

	UINT strides = sizeof(Vertex);
	UINT offsets = 0;

	static VSObjectConstantBuffer cb;

	const auto& transform = _target->transform;

	const float scaleX = HALF_OBJECT_SIZE / RenderManager::renderZoom.x;
	const float scaleY = HALF_OBJECT_SIZE / RenderManager::renderZoom.y;

	switch (handleMode)
	{
	case POSITION:
	{
		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, m_arrowVertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(m_arrowIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//ワールド変換行列の作成
		//ー＞オブジェクトの位置・大きさ・向きを指定
		cb.world = DirectX::XMMatrixScaling(scaleX, scaleY, 1.0f);
		cb.world *= DirectX::XMMatrixRotationZ(0.0f);
		cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);
		cb.world = DirectX::XMMatrixTranspose(cb.world);
		if (moveMode == VERTICAL_POS_ON_MOUSE || moveMode == VERTICAL_POS)
			cb.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		else
			cb.color = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	
		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(10, 0, 0);

		//ワールド変換行列の作成
		//ー＞オブジェクトの位置・大きさ・向きを指定
		cb.world = DirectX::XMMatrixScaling(scaleY,scaleX,1.0f);
		cb.world *= DirectX::XMMatrixRotationZ(-Math::PI / 2);
		cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);
		cb.world = DirectX::XMMatrixTranspose(cb.world);
		if (moveMode == HORIZON_POS_ON_MOUSE || moveMode == HORIZON_POS)
			cb.color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		else
			cb.color = XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f);
	
		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(10, 0, 0);

		//設定戻す
		//======================================================================================================================
		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		DirectX11::m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//======================================================================================================================

		//ワールド変換行列の作成
		//ー＞オブジェクトの位置・大きさ・向きを指定
		cb.world = DirectX::XMMatrixScaling(scaleX / 2, scaleY / 2, 1.0f);
		cb.world *= DirectX::XMMatrixRotationZ(0.0f);
		cb.world *= DirectX::XMMatrixTranslation(transform.position.x + scaleX * 2.5f, transform.position.y + scaleY * 2.5f, 0.0f);
		cb.world = DirectX::XMMatrixTranspose(cb.world);
		if (moveMode == MOVE_POS_ON_MOUSE || moveMode == MOVE_POS)
			cb.color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.75f);
		else
			cb.color = XMFLOAT4(0.5f, 0.5f, 0.0f, 0.75f);

		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(6, 0, 0);
	}
	break;
	case ROTATION:
	{
		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, Box2DBodyManager::m_circleVertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(Box2DBodyManager::m_circleIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//ワールド変換行列の作成
		//ー＞オブジェクトの位置・大きさ・向きを指定
		cb.world = DirectX::XMMatrixScaling(scaleX * 2, scaleY* 2, 1.0f);
		cb.world *= DirectX::XMMatrixRotationZ(0.0f);
		cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);
		cb.world = DirectX::XMMatrixTranspose(cb.world);
		if (moveMode == MOVE_ROTATION_ON_MOUSE || moveMode == MOVE_ROTATION)
			cb.color = XMFLOAT4(1.0f, 0.0f, 1.0f, 0.75f);
		else
			cb.color = XMFLOAT4(0.5f, 0.0f, 0.5f, 0.75f);

		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(Box2DBodyManager::numSegments * 2, 0, 0);
	}
	break;
	case SCALE:
	{
		DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, RenderManager::m_vertexBuffer.GetAddressOf(), &strides, &offsets);
		DirectX11::m_pDeviceContext->IASetIndexBuffer(RenderManager::m_lineBoxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		//ワールド変換行列の作成
		//ー＞オブジェクトの位置・大きさ・向きを指定
		cb.world = DirectX::XMMatrixScaling(_target->transform.scale.x * 1.2f, 
			_target->transform.scale.y * 1.2f, 1.0f);
		cb.world *= DirectX::XMMatrixRotationZ(0.0f);
		cb.world *= DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);
		cb.world = DirectX::XMMatrixTranspose(cb.world);
		bool pick = moveMode == MOVE_SCALE_ON_MOUSE || moveMode == MOVE_SCALE;
		if (pick)
			cb.color = XMFLOAT4(1.0f, 0.56f, 0.0f, 0.75f);
		else
			cb.color = XMFLOAT4(0.54f, 0.27f, 0.07f, 0.75f);

		//行列をシェーダーに渡す
		DirectX11::m_pDeviceContext->UpdateSubresource(
			DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

		DirectX11::m_pDeviceContext->DrawIndexed(8, 0, 0);

		if (pick)
		{
			DirectX11::m_pDeviceContext->IASetVertexBuffers(0, 1, m_arrowVertexBuffer.GetAddressOf(), &strides, &offsets);
			DirectX11::m_pDeviceContext->IASetIndexBuffer(m_arrowIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

			//ワールド変換行列の作成
			//ー＞オブジェクトの位置・大きさ・向きを指定
			cb.world = DirectX::XMMatrixScaling(scaleX / 2, scaleY / 2, 1.0f);
			cb.world *= DirectX::XMMatrixRotationZ(moveScaleRad - Math::hPI);
			cb.world *= DirectX::XMMatrixTranslation(_targetPos.x, _targetPos.y, 0.0f);
			cb.world = DirectX::XMMatrixTranspose(cb.world);
			cb.color = XMFLOAT4(1.0f, 0.56f, 0.0f, 0.75f);
	
			//行列をシェーダーに渡す
			DirectX11::m_pDeviceContext->UpdateSubresource(
				DirectX11::m_pVSObjectConstantBuffer.Get(), 0, NULL, &cb, 0, 0);

			DirectX11::m_pDeviceContext->DrawIndexed(10, 0, 0);
		}
	}
	break;
	}

	
}

std::function<void()> ImGuiApp::HandleUI::SetChangeValue(GameObject* _object, MOVE_MODE _moveMode)
{
	std::vector<UPDATE_VALUE_TYPE> types;
	switch (_moveMode)
	{
	case VERTICAL_POS:
	case HORIZON_POS:
	case MOVE_POS:
		if (_object->ExistComponent<Box2DBody>())
			types.emplace_back(BOX2D_POSITION);
		if (_object->ExistComponent<SubWindow>())
			types.emplace_back(WINDOW_POSITION);
		break;
	case MOVE_ROTATION:
		if (_object->ExistComponent<Box2DBody>())
			types.emplace_back(BOX2D_ROTATION);
		break;
	}

	decltype(updateValues)::iterator iter = updateValues.find(_object->GetName());
	if (iter == updateValues.end())
	{
		iter = updateValues.emplace(std::make_pair(_object->GetName(), std::array<int, VALUE_TYPE_MAX>{})).first;
	}

	for (auto& type : types)
	{
		iter->second[type]++;
	}
	
	return std::move(std::bind([](std::string _name, std::vector<UPDATE_VALUE_TYPE> _types) {
		auto it = updateValues.find(_name);
		if (it != updateValues.end())
			for (auto& type : _types)
			{
				it->second[type]--;
			}
		}, _object->GetName(), std::move(types)));
}

void ImGuiApp::HandleUI::SetUploadFile(std::string _uploadStr, std::function<void(GameObject*, fs::path)>&& _func, std::vector<std::string>&& _extensions)
{
	uploadStr = _uploadStr;
	linkFunc = _func;
	extensions = _extensions;
}

void ImGuiApp::HandleUI::LockHandle(bool _lock, const char* _lockName)
{
	std::string name(_lockName);

	if (_lock)
	{
		lock = true;
		if (std::find(lockNames.begin(), lockNames.end(), name) == lockNames.end())
		{
			lockNames.push_back(std::move(name));
		}
		return;
	}

	auto iter = std::find(lockNames.begin(), lockNames.end(), name);
	if (iter != lockNames.end())
	{
		lockNames.erase(iter);
	}

	if (lockNames.empty()) lock = false;
}

void ImGuiApp::ImGuiSetKeyMap(ImGuiContext* _imguiContext)
{
	ImGui::SetCurrentContext(_imguiContext);
	ImGuiIO& io = ImGui::GetIO();
#if defined(_WIN32) && !defined(IMGUI_DISABLE_OBSOLETE_KEYIO)
	// キーボードの標準キー
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Space] = VK_SPACE;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	// コントローラー入力のマッピング
	// Aボタン -> Space、Bボタン -> Escape など
	//io.KeyMap[ImGuiKey_Space] = XINPUT_GAMEPAD_A; // AボタンをSpaceキーとする
	//io.KeyMap[ImGuiKey_Escape] = XINPUT_GAMEPAD_B; // BボタンをEscapeキーとする
	//io.KeyMap[ImGuiKey_Enter] = XINPUT_GAMEPAD_START; // StartボタンをEnterキーとする
	//io.KeyMap[ImGuiKey_Backspace] = XINPUT_GAMEPAD_BACK;  // BackボタンをBackspaceキーとする
	//io.KeyMap[ImGuiKey_UpArrow] = XINPUT_GAMEPAD_DPAD_UP;
	//io.KeyMap[ImGuiKey_DownArrow] = XINPUT_GAMEPAD_DPAD_DOWN;
	//io.KeyMap[ImGuiKey_LeftArrow] = XINPUT_GAMEPAD_DPAD_LEFT;
	//io.KeyMap[ImGuiKey_RightArrow] = XINPUT_GAMEPAD_DPAD_RIGHT;
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
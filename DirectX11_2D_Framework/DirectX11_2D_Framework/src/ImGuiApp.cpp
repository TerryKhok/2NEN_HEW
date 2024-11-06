
int ImGuiApp::worldFpsCounter;
int ImGuiApp::updateFpsCounter = 0;


//window関連
HWND ImGuiApp::m_hWnd[TYPE_MAX];
ImGuiContext* ImGuiApp::context[TYPE_MAX];
std::unordered_map<HWND, ImGuiContext*> ImGuiApp::m_hWndContexts;
ComPtr<IDXGISwapChain> ImGuiApp::m_pSwapChain[TYPE_MAX];
ComPtr<ID3D11RenderTargetView> ImGuiApp::m_pRenderTargetView[TYPE_MAX];
void(*ImGuiApp::pDrawImGui[ImGuiApp::TYPE_MAX])() = {};

void ImGuiSetKeyMap(ImGuiContext* _imguiContext);

HRESULT ImGuiApp::Init(HINSTANCE hInstance)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

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
	windowPos[OPTIONS] = { MONITER_HALF_WIDTH / -1.35f,0 };
	windowPos[INSPECTER] = { MONITER_HALF_WIDTH / 1.35f,0 };

	// Use std::fill to set all elements to the same function
	std::fill(std::begin(pDrawImGui), std::end(pDrawImGui), []() {});
	pDrawImGui[OPTIONS] = DrawOptionGui;
	pDrawImGui[INSPECTER] = DrawInspecterGui;

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
	if (ImGui::Begin("Status"))
	{
		ImGui::Text("world average %.3f ms/frame (%d FPS)", 1000.0f / worldFpsCounter, worldFpsCounter);
		ImGui::Text("update average %.3f ms/frame (%d FPS)", 1000.0f / updateFpsCounter, updateFpsCounter);
	}
	ImGui::End();

	if (ImGui::Begin("Menu"))
	{
		ImGui::ColorEdit3("clear color", clearColor); // Edit 3 floats representing a color
		ImGui::Checkbox("HitBox", &RenderManager::drawHitBox);
		ImGui::Checkbox("Ray", &RenderManager::drawRay);
		if (ImGui::Button("Button"))
		{
			OpenFileDialog();
		}
	}
	ImGui::End();

	if (ImGui::Begin("Object List", nullptr, ImGuiWindowFlags_None))
	{
		for (auto& object : *ObjectManager::m_currentList) {
			if (ImGui::Selectable(object.second->GetName().c_str())) {
				// Handle selection, e.g., highlighting the object or showing more details
				selectedObject = object.second.get();
			}
		}
	}
	ImGui::End();
}

void ImGuiApp::DrawInspecterGui()
{
	if (ImGui::Begin("Inspecter", nullptr, ImGuiWindowFlags_None))
	{
		if (selectedObject != nullptr)
		{
			ImGui::Text("selected \n: %s", selectedObject->name.c_str());

			ImGui::SeparatorText("Component");

			if (ImGui::TreeNode("Transform"))
			{
				bool enabled = true;
				ImGui::Checkbox("Enabled", &enabled);

				ImGui::InputFloat3("Position", selectedObject->transform.position.data(), "%.1f");
				ImGui::InputFloat3("Scale", selectedObject->transform.scale.data(), "%.1f");
				auto& angle = selectedObject->transform.angle;
				float angles[3] = {
					static_cast<float>(angle.x),
					static_cast<float>(angle.y),
					static_cast<float>(angle.z)
				};
				ImGui::InputFloat3("Angle", angles, "%.1f");
				angle.x = angles[0];
				angle.x = angles[1];
				angle.x = angles[2];

				ImGui::TreePop();
			}

			for (auto& component : selectedObject->m_componentList)
			{
				std::string componentName = component.first;

				if (ImGui::TreeNode(componentName.substr(6).c_str()))
				{
					component.second->DrawImGui();
					ImGui::TreePop();
				}
			}
		}
		else
		{
			ImGui::Text("not selected gameobject");
		}
		
	}
	ImGui::End();
}

void ImGuiApp::Uninit()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
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
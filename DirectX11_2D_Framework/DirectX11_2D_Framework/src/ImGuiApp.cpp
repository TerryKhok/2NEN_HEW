
int ImGuiApp::worldFpsCounter;
int ImGuiApp::updateFpsCounter = 0;

void ImGuiApp::Init(HWND hWnd)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGui_ImplDX11_Init(DirectX11::m_pDevice.Get(), DirectX11::m_pDeviceContext.Get());
	ImGui_ImplWin32_Init(hWnd); // hWnd is your main window handle
	ImGui::StyleColorsDark(); // Set ImGui style (optional)
}

void ImGuiApp::Begin()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiApp::Draw()
{	
	ImVec2 newSize(300, 70); // Set the new size (width, height)
	ImGui::SetNextWindowSize(newSize, ImGuiCond_Once); // ImGuiCond_Once makes it set the size only on the first frame
	// Your ImGui code (e.g., create windows, buttons, etc.)
	ImGui::Begin("Example Window");
	ImGui::Text("world average %.3f ms/frame (%d FPS)",1000.0f / worldFpsCounter, worldFpsCounter);
	ImGui::Text("update average %.3f ms/frame (%d FPS)", 1000.0f / updateFpsCounter, updateFpsCounter);
	
	ImGui::End();
}

void ImGuiApp::Rend()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiApp::Uninit()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

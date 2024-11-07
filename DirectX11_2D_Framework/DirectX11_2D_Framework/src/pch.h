#pragma once

#ifndef PCH_H
#define PCH_H

#include <d3d11.h>  // DirectX11を使うためのヘッダーファイル
#include <dxgi.h>
#include<DirectXMath.h>

#include<windows.h>
#include <wincodec.h>
#include <d3dcompiler.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma comment(lib, "windowscodecs.lib")

#include <Xinput.h> //XInputを使うためのヘッダーファイル
#pragma comment (lib, "xinput.lib") //XInputを使うために必要


//フォント用ライブラリ
//#pragma warning(push)
//#pragma warning(disable:4005)
//
//#include <dwrite.h>  
//#include <d2d1_2.h>						// Direct2D
//#include <DWrite.h>						// DirectWrite
//
//#pragma comment(lib, "D2d1.lib")		// Direct2D用
//#pragma comment(lib,"Dwrite.lib")		// DirectWrite用
//
//#pragma warning(pop)

#include <locale.h>
#include <atltypes.h> // CRectを使うためのヘッダーファイル
#include <io.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <wrl/client.h>
#include <time.h>
#include <io.h>
#include <Fcntl.h>
#include <cstdio>
#include <iostream>
#include <cassert>
#include <concurrent_queue.h>
#include <thread>
#include <atomic>
#include <future>
#include <chrono>
#include <mutex>
#include <variant>
#include <windowsx.h>
#include <condition_variable>
#include <type_traits>
#include <codecvt>
#include <cstdlib>  // For wcstombs_s
#include <cwchar>   // For wchar_t
#include <commdlg.h>  // For common dialogs (GetSaveFileName)
#include <fstream>
#include <exception>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "../../DirectX11_2D_Framework/math/Math.h"
#include "../../DirectX11_2D_Framework/math/Vector.h"
#include "../../DirectX11_2D_Framework/math/Angle.h"
#include "../../DirectX11_2D_Framework/inc/Debug.h"
#include "../../DirectX11_2D_Framework/inc/Window.h"
#include "../../DirectX11_2D_Framework/inc/directX3D.h"
#include "../../DirectX11_2D_Framework/inc/ImGuiApp.h"
#include "../../DirectX11_2D_Framework/inc/Input.h"
#include "../../DirectX11_2D_Framework/inc/TextureAsset.h"
#include "../../DirectX11_2D_Framework/src/Layer.h"
#include "../../DirectX11_2D_Framework/inc/Box2D.h"
#include "../../DirectX11_2D_Framework/inc/Component.h"
#include "../../DirectX11_2D_Framework/inc/Animator.h"
#include "../../DirectX11_2D_Framework/inc/Object.h"
#include "../../DirectX11_2D_Framework/inc/Renderer.h"
#include "../../DirectX11_2D_Framework/inc/SFText.h"
#include "../../DirectX11_2D_Framework/inc/Scene.h"
#include "../../DirectX11_2D_Framework/inc/Box2DBody.h"
#include "../../DirectX11_2D_Framework/inc/Camera.h"
#include "../../DirectX11_2D_Framework/inc/WindowRect.h"
#include "../../DirectX11_2D_Framework/inc/Button.h"
#endif



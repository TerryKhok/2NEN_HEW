#pragma once

#include <d3d11.h>  // DirectX11を使うためのヘッダーファイル
#include<DirectXMath.h>

#include<windows.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <Xinput.h> //XInputを使うためのヘッダーファイル
#pragma comment (lib, "xinput.lib") //XInputを使うために必要

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
#include<algorithm>
#include <memory>
#include <wrl/client.h>
#include <time.h>
#include <io.h>
#include <Fcntl.h>
#include <cstdio>
#include <iostream>
#include <cassert>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//デバッグフラグ
//================================================================
#define DEBUG_TRUE
//================================================================


#include "WICTextureLoader.h"
#include "Math.h"
#include "Vector.h"
#include "Window.h"
#include "directX3D.h"
#include "Input.h"
#include "Asset.h"
#include "Layer.h"
#include "Component.h"
#include "Object.h"
#include "Renderer.h"
#include "Collider2D.h"



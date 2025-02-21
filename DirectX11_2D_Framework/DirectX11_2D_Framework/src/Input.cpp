
POINT Input::mousePoint = {};
bool Input::isMouseLeft = false;
bool Input::isMouseRight = false;
bool Input::isMouseMiddle = false;
bool Input::isMouseDownLeft = false;
bool Input::isMouseDownRight = false;
bool Input::isMouseDownMiddle = false;
bool Input::isMouseUpLeft = false;
bool Input::isMouseUpRight = false;
bool Input::isMouseUpMiddle = false;
int Input::mouseWheelDelta = 0;

//InputState::INPUT_TYPE InputState::iState;

//コンストラクタ
Input::Input()
{
	VibrationTime = 0;
}

//デストラクタ
Input::~Input()
{
	//振動を終了させる
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = 0;
	vibration.wRightMotorSpeed = 0;
	XInputSetState(0, &vibration);
}

Input& Input::Get()
{
	static Input instance;
	return instance;
}

void Input::Update()
{
	//1フレーム前の入力を記録しておく
	for (int i = 0; i < 256; i++) { keyState_old[i] = keyState[i]; }
	controllerState_old = controllerState;

	//キー入力を更新
	BOOL hr = GetKeyboardState(keyState);

	//for (int type = 0; type < ImGuiApp::TYPE_MAX; type++)
	//{
	//	ImGui::SetCurrentContext(ImGuiApp::context[type]);

	//	ImGuiIO& io = ImGui::GetIO();

	//	// キーボードのキー入力を設定 (必要なキーを追加)
	//	io.AddKeyEvent(ImGuiKey_Space, keyState[VK_SPACE] & 0x8000);
	//	io.AddKeyEvent(ImGuiKey_Enter, keyState[VK_RETURN] & 0x8000);
	//	io.AddKeyEvent(ImGuiKey_Escape, keyState[VK_ESCAPE] & 0x8000);
	//	io.AddKeyEvent(ImGuiKey_LeftArrow, keyState[VK_LEFT] & 0x8000);
	//	io.AddKeyEvent(ImGuiKey_RightArrow, keyState[VK_RIGHT] & 0x8000);
	//	io.AddKeyEvent(ImGuiKey_UpArrow, keyState[VK_UP] & 0x8000);
	//	io.AddKeyEvent(ImGuiKey_DownArrow, keyState[VK_DOWN] & 0x8000);
	//}

	//コントローラー入力を更新(XInput)
	controllerConnect = XInputGetState(0, &controllerState);

	//振動継続時間をカウント
	if (VibrationTime > 0) {
		VibrationTime--;
		if (VibrationTime == 0) { //振動継続時間が経った時に振動を止める
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
			vibration.wLeftMotorSpeed = 0;
			vibration.wRightMotorSpeed = 0;
			XInputSetState(0, &vibration);
		}
	}

	//マウス情報更新
	mouseLeftTrigger = isMouseDownLeft;
	mouseRightTrigger = isMouseDownRight;
	mouseMiddleTrigger = isMouseDownMiddle;
	mouseLeftRelease = isMouseUpLeft;
	mouseRightRelease = isMouseUpRight;
	mouseMiddleRelease = isMouseUpMiddle;

	isMouseDownLeft = false;
	isMouseDownRight = false;
	isMouseDownMiddle = false;
	isMouseUpLeft = false;
	isMouseUpRight = false;
	isMouseUpMiddle = false;

	//ゼロに近づける
	mouseWheelDelta -= (mouseWheelDelta > 0) * 8 - (mouseWheelDelta < 0) * 8;
}

//キー入力
bool Input::KeyPress(int key) //プレス
{
	return keyState[key] & 0x80;
}
bool Input::KeyTrigger(int key) //トリガー
{
	return (keyState[key] & 0x80) && !(keyState_old[key] & 0x80);
}
bool Input::KeyRelease(int key) //リリース
{
	return !(keyState[key] & 0x80) && (keyState_old[key] & 0x80);
}

Vector2 Input::MousePoint()
{
	return { static_cast<float>(mousePoint.x),static_cast<float>(mousePoint.y)};
}

bool Input::MouseLeftPress()
{
	return isMouseLeft;
}

bool Input::MouseRightPress()
{
	return isMouseRight;
}

bool Input::MouseMiddlePress()
{
	return isMouseMiddle;
}

bool Input::MouseLeftTrigger()
{
	return mouseLeftTrigger;
}

bool Input::MouseRightTrigger()
{
	return mouseRightTrigger;
}

bool Input::MouseMiddleTrigger()
{
	return mouseMiddleTrigger;
}

bool Input::MouseLeftRelease()
{
	return mouseLeftRelease;
}

bool Input::MouseRightRelease()
{
	return mouseRightRelease;
}

bool Input::MouseMiddleRelease()
{
	return mouseMiddleRelease;
}

int Input::MouseWheelDelta()
{
	return mouseWheelDelta;
}


//左アナログスティック
Vector2 Input::LeftAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbLX; // -32768〜32767
	SHORT y = controllerState.Gamepad.sThumbLY; // -32768〜32767

	Vector2 res;
	res.x = x / 32767.0f; //-1〜1
	res.y = y / 32767.0f; //-1〜1
	return res;
}
//右アナログスティック
Vector2 Input::RightAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbRX; // -32768〜32767
	SHORT y = controllerState.Gamepad.sThumbRY; // -32768〜32767

	Vector2 res;
	res.x = x / 32767.0f; //-1〜1
	res.y = y / 32767.0f; //-1〜1
	return res;
}

//左トリガー
float Input::LeftTrigger(void)
{
	BYTE t = controllerState.Gamepad.bLeftTrigger; // 0〜255
	return t / 255.0f;
}
//右トリガー
float Input::RightTrigger(void)
{
	BYTE t = controllerState.Gamepad.bRightTrigger; // 0〜255
	return t / 255.0f;
}

//ボタン入力
bool Input::ButtonPress(WORD btn) //プレス
{
	return (controllerState.Gamepad.wButtons & btn) != 0;
}
bool Input::ButtonTrigger(WORD btn) //トリガー
{
	return (controllerState.Gamepad.wButtons & btn) != 0 && (controllerState_old.Gamepad.wButtons & btn) == 0;
}
bool Input::ButtonRelease(WORD btn) //リリース
{
	return (controllerState.Gamepad.wButtons & btn) == 0 && (controllerState_old.Gamepad.wButtons & btn) != 0;
}

//振動
void Input::SetVibration(int frame, float powor)
{
	// XINPUT_VIBRATION構造体のインスタンスを作成
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	// モーターの強度を設定（0〜65535）
	vibration.wLeftMotorSpeed = (WORD)(powor * 65535.0f);
	vibration.wRightMotorSpeed = (WORD)(powor * 65535.0f);
	XInputSetState(0, &vibration);

	//振動継続時間を代入
	VibrationTime = frame;
}

bool Input::IsConnectController() const
{
	return controllerConnect == ERROR_SUCCESS;
}

//void InputState::AddKeyBoardMap(const char* _name, int _key)
//{
//	auto iter = inputMap.find(_name);
//	if (iter != inputMap.end())
//	{
//		iter->second.first = _key;
//		return;
//	}
//
//	inputMap.emplace(_name, std::make_pair(_key, 0));
//}
//
//void InputState::AddControllerMap(const char* _name, int _btn)
//{
//	auto iter = inputMap.find(_name);
//	if (iter != inputMap.end())
//	{
//		iter->second.second = _btn;
//		return;
//	}
//
//	inputMap.emplace(_name, std::make_pair(0, _btn));
//}

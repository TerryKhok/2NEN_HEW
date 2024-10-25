
POINT Input::mousePoint = {};
bool Input::isMouseLeft = false;
bool Input::isMouseRight = false;
bool Input::isMouseDownLeft = false;
bool Input::isMouseDownRight = false;
bool Input::isMouseUpLeft = false;
bool Input::isMouseUpRight = false;
int Input::mouseWheelDelta = 0;

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

	//コントローラー入力を更新(XInput)
	XInputGetState(0, &controllerState);

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
	mouseLeftRelease = isMouseUpLeft;
	mouseRightRelease = isMouseUpRight;

	isMouseDownLeft = false;
	isMouseDownRight = false;
	isMouseUpLeft = false;
	isMouseUpRight = false;

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

bool Input::MouseLeftTrigger()
{
	return mouseLeftTrigger;
}

bool Input::MouseRightTrigger()
{
	return mouseRightTrigger;
}

bool Input::MouseLeftRelease()
{
	return mouseLeftRelease;
}

bool Input::MouseRightRelease()
{
	return mouseRightRelease;
}

int Input::MouseWheelDelta()
{
	return mouseWheelDelta;
}


//左アナログスティック
DirectX::XMFLOAT2 Input::LeftAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbLX; // -32768～32767
	SHORT y = controllerState.Gamepad.sThumbLY; // -32768～32767

	DirectX::XMFLOAT2 res;
	res.x = x / 32767.0f; //-1～1
	res.y = y / 32767.0f; //-1～1
	return res;
}
//右アナログスティック
DirectX::XMFLOAT2 Input::RightAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbRX; // -32768～32767
	SHORT y = controllerState.Gamepad.sThumbRY; // -32768～32767

	DirectX::XMFLOAT2 res;
	res.x = x / 32767.0f; //-1～1
	res.y = y / 32767.0f; //-1～1
	return res;
}

//左トリガー
float Input::LeftTrigger(void)
{
	BYTE t = controllerState.Gamepad.bLeftTrigger; // 0～255
	return t / 255.0f;
}
//右トリガー
float Input::RightTrigger(void)
{
	BYTE t = controllerState.Gamepad.bRightTrigger; // 0～255
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

	// モーターの強度を設定（0～65535）
	vibration.wLeftMotorSpeed = (WORD)(powor * 65535.0f);
	vibration.wRightMotorSpeed = (WORD)(powor * 65535.0f);
	XInputSetState(0, &vibration);

	//振動継続時間を代入
	VibrationTime = frame;
}
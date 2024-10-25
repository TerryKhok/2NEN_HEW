
POINT Input::mousePoint = {};
bool Input::isMouseLeft = false;
bool Input::isMouseRight = false;
bool Input::isMouseDownLeft = false;
bool Input::isMouseDownRight = false;
bool Input::isMouseUpLeft = false;
bool Input::isMouseUpRight = false;
int Input::mouseWheelDelta = 0;

//�R���X�g���N�^
Input::Input()
{
	VibrationTime = 0;
}

//�f�X�g���N�^
Input::~Input()
{
	//�U�����I��������
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
	//1�t���[���O�̓��͂��L�^���Ă���
	for (int i = 0; i < 256; i++) { keyState_old[i] = keyState[i]; }
	controllerState_old = controllerState;

	//�L�[���͂��X�V
	BOOL hr = GetKeyboardState(keyState);

	//�R���g���[���[���͂��X�V(XInput)
	XInputGetState(0, &controllerState);

	//�U���p�����Ԃ��J�E���g
	if (VibrationTime > 0) {
		VibrationTime--;
		if (VibrationTime == 0) { //�U���p�����Ԃ��o�������ɐU�����~�߂�
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
			vibration.wLeftMotorSpeed = 0;
			vibration.wRightMotorSpeed = 0;
			XInputSetState(0, &vibration);
		}
	}

	//�}�E�X���X�V
	mouseLeftTrigger = isMouseDownLeft;
	mouseRightTrigger = isMouseDownRight;
	mouseLeftRelease = isMouseUpLeft;
	mouseRightRelease = isMouseUpRight;

	isMouseDownLeft = false;
	isMouseDownRight = false;
	isMouseUpLeft = false;
	isMouseUpRight = false;

	//�[���ɋ߂Â���
	mouseWheelDelta -= (mouseWheelDelta > 0) * 8 - (mouseWheelDelta < 0) * 8;
}

//�L�[����
bool Input::KeyPress(int key) //�v���X
{
	return keyState[key] & 0x80;
}
bool Input::KeyTrigger(int key) //�g���K�[
{
	return (keyState[key] & 0x80) && !(keyState_old[key] & 0x80);
}
bool Input::KeyRelease(int key) //�����[�X
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


//���A�i���O�X�e�B�b�N
DirectX::XMFLOAT2 Input::LeftAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbLX; // -32768�`32767
	SHORT y = controllerState.Gamepad.sThumbLY; // -32768�`32767

	DirectX::XMFLOAT2 res;
	res.x = x / 32767.0f; //-1�`1
	res.y = y / 32767.0f; //-1�`1
	return res;
}
//�E�A�i���O�X�e�B�b�N
DirectX::XMFLOAT2 Input::RightAnalogStick(void)
{
	SHORT x = controllerState.Gamepad.sThumbRX; // -32768�`32767
	SHORT y = controllerState.Gamepad.sThumbRY; // -32768�`32767

	DirectX::XMFLOAT2 res;
	res.x = x / 32767.0f; //-1�`1
	res.y = y / 32767.0f; //-1�`1
	return res;
}

//���g���K�[
float Input::LeftTrigger(void)
{
	BYTE t = controllerState.Gamepad.bLeftTrigger; // 0�`255
	return t / 255.0f;
}
//�E�g���K�[
float Input::RightTrigger(void)
{
	BYTE t = controllerState.Gamepad.bRightTrigger; // 0�`255
	return t / 255.0f;
}

//�{�^������
bool Input::ButtonPress(WORD btn) //�v���X
{
	return (controllerState.Gamepad.wButtons & btn) != 0;
}
bool Input::ButtonTrigger(WORD btn) //�g���K�[
{
	return (controllerState.Gamepad.wButtons & btn) != 0 && (controllerState_old.Gamepad.wButtons & btn) == 0;
}
bool Input::ButtonRelease(WORD btn) //�����[�X
{
	return (controllerState.Gamepad.wButtons & btn) == 0 && (controllerState_old.Gamepad.wButtons & btn) != 0;
}

//�U��
void Input::SetVibration(int frame, float powor)
{
	// XINPUT_VIBRATION�\���̂̃C���X�^���X���쐬
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	// ���[�^�[�̋��x��ݒ�i0�`65535�j
	vibration.wLeftMotorSpeed = (WORD)(powor * 65535.0f);
	vibration.wRightMotorSpeed = (WORD)(powor * 65535.0f);
	XInputSetState(0, &vibration);

	//�U���p�����Ԃ���
	VibrationTime = frame;
}
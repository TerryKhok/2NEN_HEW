#pragma once

#ifndef INPUT_A
#define INPUT_A

#define XINPUT_A              0x1000
#define XINPUT_B              0x2000
#define XINPUT_X              0x4000
#define XINPUT_Y              0x8000
#define XINPUT_UP             0x0001
#define XINPUT_DOWN           0x0002
#define XINPUT_LEFT           0x0004
#define XINPUT_RIGHT          0x0008
#define XINPUT_START          0x0010
#define XINPUT_BACK           0x0020
#define XINPUT_LEFT_THUMB     0x0040 //左スティック押し込み
#define XINPUT_RIGHT_THUMB    0x0080 //右スティック押し込み
#define XINPUT_LEFT_SHOULDER  0x0100 //L
#define XINPUT_RIGHT_SHOULDER 0x0200 //R

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x45
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A


//シングルトンクラス
class Input final
{
	//ウィンドウで更新する・マウス情報受け取る
	friend class Window;

private:
	//生成禁止
	Input();
	//更新処理
	void Update();
public:
	~Input(); //デストラクタ

	//インスタンス取得
	static Input& Get();

	//キー入力
	bool KeyPress(int key);   //プレス(押している間ずっと)
	bool KeyTrigger(int key); //トリガー(押し始めた時)
	bool KeyRelease(int key); //リリース(押し終わった時)

	//マウス位置
	POINT MousePoint();
	//マウス左ボタンプレス
	bool MouseLeftPress();
	//マウス右ボタンプレス
	bool MouseRightPress();
	//マウス左ボタントリガー
	bool MouseLeftTrigger();
	//マウス右ボタントリガー
	bool MouseRightTrigger();
	//マウス右ボタンリリース
	bool MouseLeftRelease();
	//マウス右ボタンリリース
	bool MouseRightRelease();

	//アナログスティック(コントローラー)
	DirectX::XMFLOAT2 LeftAnalogStick(void);
	DirectX::XMFLOAT2 RightAnalogStick(void);

	//トリガー(コントローラー)
	float LeftTrigger(void);
	float RightTrigger(void);

	//ボタン入力(コントローラー)
	bool ButtonPress(WORD btn);   //プレス(押している間ずっと)
	bool ButtonTrigger(WORD btn); //トリガー(押し始めた時)
	bool ButtonRelease(WORD btn); //リリース(押し終わった時)

	//振動(コントローラー)
	//flame：振動を継続する時間(単位：フレーム)
	//powoe：振動の強さ(0～1)
	void SetVibration(int frame = 1, float powor = 1);
private:
	//キー入力情報を保存する変数
	BYTE keyState[256] = {};
	BYTE keyState_old[256] = {};

	//コントローラー入力情報を保存する変数
	XINPUT_STATE controllerState = {};
	XINPUT_STATE controllerState_old = {};

	int VibrationTime; //振動継続時間をカウントする変数

	static POINT mousePoint;
	static bool isMouseLeft;
	static bool isMouseRight;
	static bool isMouseDownLeft;
	static bool isMouseDownRight;
	static bool isMouseUpLeft;
	static bool isMouseUpRight;
	bool mouseLeftTrigger = false;
	bool mouseRightTrigger = false;
	bool mouseLeftRelease = false;
	bool mouseRightRelease = false;
};

#endif

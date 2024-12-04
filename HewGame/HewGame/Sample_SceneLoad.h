#include "Sample_Animation.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
//  シーンの遷移サンプル
// 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//こんなクラスがあることを知らせるために先に定義する
class SampleScene_Loading;
class SampleScene02;

//=============================================================================
//※シーンの登録はmain.cppでとりあえずやってます
//=============================================================================


class SampleScene_Title : public Scene
{
	//ロード中にすることをかく(オーバライド)
	void Load() override
	{
		//オブジェクト生成(名前、テクスチャ指定)
		auto object = Instantiate("HartR", L"asset/pic/hartR.png");

	}

	//更新処理(オーバライド)
	void Update() override
	{
		//スペースキーを押したとき
		if (Input::Get().KeyTrigger(VK_SPACE))
 		{
			//ロード中のシーンに遷移(同期) ※なくてもいい
			SceneManager::LoadScene<SampleScene_Loading>();
		}
	}
};

class SampleScene_Loading : public Scene
{
	//オブジェクトポインター
	GameObject* arrow = nullptr;

	void Init()
	{
		arrow = Instantiate("Arrow", L"asset/pic/rollArrow.png");

		//シーンのローディング(非同期)
		SceneManager::LoadScene<SampleScene_Box2D>();
		//ロードしたシーンに遷移
		//SceneManager::ChangeScene();
	}

	void Update()
	{
		//回転させてるだけ
		static float angle = 0.0f;
		angle -= 1.0f;
		if (abs(angle) > 360.0f)
			angle = 0.0f;
		
		arrow->transform.angle.z = angle;
	}
};
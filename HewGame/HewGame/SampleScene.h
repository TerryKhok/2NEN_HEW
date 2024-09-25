
//================================================================
//シーンの遷移サンプル
//================================================================


class LoadScene;
class SampleScene02;


class SampleScene01 : public Scene
{
	//ロード中にすることをかく(オーバライド)
	void Load() override
	{
		//オブジェクト生成
		auto object = Instantiate("HartR", L"asset/pic/hartR.png");
	}

	//更新処理(オーバライド)
	void Update() override
	{
		if (Input::Get().KeyTrigger(VK_SPACE))
 		{
			//ロード中のシーンに遷移(同期)
			SceneManager::LoadScene<LoadScene>();
			//シーンのローディング(非同期)
			SceneManager::LoadingScene<SampleScene02>();
			//ロードしたシーンの切り替え
			SceneManager::ChangeScene();
		}
	}
};

class LoadScene : public Scene
{
	GameObject* object = nullptr;

	void Load()
	{
		object = Instantiate("HartR", L"asset/pic/rollArrow.png");
	}

	void Update()
	{
		//回転させてるだけ
		static float angle = 0.0f;
		angle += 0.1f;
		if (angle > 360.0f)
			angle = 0.0f;
		
		object->transform.angle.z -= 0.1f;
	}
};

class SampleScene02 : public Scene
{
	void Load()
	{
		auto object = Instantiate("HartR", L"asset/pic/hartG.png");
		//疑似的に重い処理を表現(だだのスリープ)
		std::this_thread::sleep_for(std::chrono::seconds(4));
	}

	void Update()
	{
		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			//シーンの切り替え(同期)
			SceneManager::LoadScene<SampleScene01>();
		}
	}
};
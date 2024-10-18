
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
//  シーンの遷移サンプル
// 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//こんなクラスがあることを知らせるために先に定義する
class LoadScene;
class SampleScene02;

//=============================================================================
//※シーンの登録はmain.cppでとりあえずやってます
//=============================================================================


class SampleScene01 : public Scene
{
	//ロード中にすることをかく(オーバライド)
	void Load() override
	{
		//オブジェクト生成(名前、テクスチャ指定)
		Instantiate("HartR", L"asset/pic/hartR.png");
	}

	//更新処理(オーバライド)
	void Update() override
	{
		//スペースキーを押したとき
		if (Input::Get().KeyTrigger(VK_SPACE))
 		{
			//ロード中のシーンに遷移(同期) ※なくてもいい
			SceneManager::LoadScene<LoadScene>();

			//シーンのローディング(非同期)
			SceneManager::LoadingScene<Box2D_SampleScene>();
			//ロードしたシーンに遷移
			SceneManager::ChangeScene();
		}
	}
};

class LoadScene : public Scene
{
	//オブジェクトポインター
	GameObject* arrow = nullptr;

	void Load()
	{
		arrow = Instantiate("Arrow", L"asset/pic/rollArrow.png");
	}

	void Update()
	{
		//回転させてるだけ
		static float angle = 0.0f;
		angle -= 0.1f;
		if (abs(angle) > 360.0f)
			angle = 0.0f;
		
		arrow->transform.angle.z = angle;
	}
};

class SampleScene02 : public Scene
{
	void Load()
	{
		//オブジェクト生成
		auto object = Instantiate("HartR", L"asset/pic/hartR.png");

		//名前の変更
		object->SetName("HartG");

		//無駄に処理を重くする
		ObjectManager::Find("HartG")->GetComponent<Transform>()->gameobject->GetComponent<Renderer>()->SetTexture(L"asset/pic/hartG.png");

		//疑似的に重い処理を表現(だだのスリープ)
		std::this_thread::sleep_for(std::chrono::seconds(4));
	}

	void Update()
	{
		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			//シーンの切り替え(同期)
			SceneManager::LoadScene<SampleScene01>();

			return; //※ロード後の処理を飛ばしたい場合はreturnを使う

			//↓実行されない
			LOG("after LoadScene!!!");
		}
	}
};
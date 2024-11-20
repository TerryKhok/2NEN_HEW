#pragma once

class TitleScene : public Scene
{
	void Load() override
	{
		Sound sound;
		//sound.LoadWavFile("asset/sound/se/ƒWƒƒƒ“ƒv_1.wav");
		auto object = Instantiate("object");
		auto render = object->AddComponent<Renderer>(/*L"asset/pic/rollArrow.png"*/);
		auto text = object->AddComponent<SFText>();
		text->SetString("aaaaa");
		render->SetLayer(LAYER_UI);
		auto button = object->AddComponent<Button>();
		button->SetEvent([&](){
			//SceneManager::LoadScene<Scene_ProtoType>();
			//sound.SoundPlay(1.0f, false);
			});
	}
};

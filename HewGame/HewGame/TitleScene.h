#pragma once

class TitleScene : public Scene
{
	void Load() override
	{
		auto object = Instantiate("object");
		auto text = object->AddComponent<SFText>("Title");
		auto button = object->AddComponent<Button>();
		button->SetEvent([&](){LOG("Press");});
	}
};


SetReflectionScene(TitleScene);

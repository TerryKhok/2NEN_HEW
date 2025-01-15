#pragma once

class MoveSubWindow : public Component
{
	void Start() override
	{
		moveWindows.push_back(this);
		selfIndex = (int)moveWindows.size() - 1;
	}

	void Update()
	{
		if (selfIndex == selectIndex)
		{
			auto& input = Input::Get();
			if (input.KeyPress(VK_D))
			{
				Vector2 pos = GetWindowPosition(_hWnd);
				rb->SetPosition(pos);
			}
		}
	}

private:
	static inline int selectIndex = 0;
	static std::vector<MoveSubWindow*> moveWindows;
	int selfIndex = -1;
};

#pragma once

class GameManager final
{
	static constexpr int stageNum = 12;
	static constexpr const char* stageName = "stage";

	enum SYSTEM_STATE
	{
		TITLE,
		IN_GAME,
		PAUSE_GAME,
	};
	
	GameManager() = default;

private:
	int currentStage = 0;
}

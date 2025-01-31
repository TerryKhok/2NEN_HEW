#pragma once

#ifndef A_GAMEMANAGER
#define A_GAMEMANAGER

namespace GameManager
{
	enum SYSTEM_STATE
	{
		TITLE,
		IN_GAME,
		PAUSE_GAME,
	};

	static constexpr int stageNum = 12;
	static constexpr const char* stageName = "stage";

	static void ChangeStage();

	extern int currentStage;

	static void ChangeNextStage();
};


#endif

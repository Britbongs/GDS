#include <iostream>
#include <Krawler.h>
#include <KApplication.h>
#include <stdexcept>

#include "GameBlackboard.h"

#define EDITOR_ENABLED

#ifndef EDITOR_ENABLED
#include "LevelSetup.h"
#else
#include "EditorSetup.h"
#endif 

using namespace std;

using namespace Krawler;

#ifndef _DEBUG
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
#else
int main(void)
#endif
{
#ifndef EDITOR_ENABLED
	KScene* mainScene = new KScene(KTEXT("play scene"), SCENE_BOUNDS);
#else
	KScene* mainScene = new KScene(KTEXT("edit scene"), SCENE_BOUNDS);
#endif
	KApplicationInitialise appInit(false);
	appInit.width = 1024;
	appInit.height = 576;
	appInit.gameFps = 30;
	appInit.physicsFps = 60;
	appInit.windowTitle = KTEXT("GDS - Prototype");
	appInit.windowStyle = KWindowStyle::Windowed_Fixed_Size;
	StartupEngine(&appInit);

	KApplication::getApp()->getSceneDirector().addScene(mainScene);
#ifndef EDITOR_ENABLED
	KApplication::getApp()->getSceneDirector().setCurrentScene(KTEXT("play scene"));
#else
	KApplication::getApp()->getSceneDirector().setCurrentScene(KTEXT("edit scene"));
#endif

	KEntity* const pEntity = mainScene->addEntityToScene();
	if (!pEntity)
	{
		KPrintf(KTEXT("Unable to create level management entity!\n"));
		return -1;
	}

	pEntity->setEntityTag(KTEXT("level setup"));
#ifndef EDITOR_ENABLED
	pEntity->addComponent(new LevelSetup(pEntity));
#else
	pEntity->addComponent(new EditorSetup(pEntity));
#endif
	const KInitStatus initResult = InitialiseSubmodules();
	if (initResult != KInitStatus::Success)
	{
		system("pause");
		return -1;
	}
	RunApplication();

	ShutdownEngine();

	return 0;
}

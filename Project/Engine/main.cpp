//============================================================================
//	include
//============================================================================
#include <Engine/Core/Framework.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	std::unique_ptr<SakuEngine::Framework> game = std::make_unique<SakuEngine::Framework>();
	game->Run();

	return 0;
}
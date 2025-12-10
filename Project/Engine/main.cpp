//============================================================================
//	include
//============================================================================
#include <Engine/Core/Framework.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	std::unique_ptr<Framework> game = std::make_unique<Framework>();
	game->Run();

	return 0;
}
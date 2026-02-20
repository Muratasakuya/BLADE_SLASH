//============================================================================
//	include
//============================================================================
#include <Engine/Core/Framework.h>
#include <Engine/Object/State/DenseStateStorage.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// フレームワーク
	std::unique_ptr<SakuEngine::Framework> game = std::make_unique<SakuEngine::Framework>();
	game->Run();

	return 0;
}
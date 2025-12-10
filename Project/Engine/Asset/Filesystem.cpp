#include "Filesystem.h"

//============================================================================
//	Filesystem namespaceMethods
//============================================================================

bool Filesystem::Found(const std::filesystem::path& basePath,
	const std::string& fileName, std::filesystem::path& fullPath) {

	bool found = false;

	// ディレクトリを再帰的に探索
	for (const auto& entry : std::filesystem::recursive_directory_iterator(basePath)) {
		if (entry.is_regular_file() && entry.path().filename() == fileName) {

			fullPath = entry.path();
			found = true;
			break;
		}
	}
	return found;
}

bool Filesystem::FindByStem(const std::filesystem::path& basePath,
	const std::string& stem, const std::vector<std::string>& extensions,
	std::filesystem::path& outPath) {

	for (const auto& entry : std::filesystem::recursive_directory_iterator(basePath)) {
		if (!entry.is_regular_file()) {

			continue;
		}
		if (entry.path().stem().string() != stem) {

			continue;
		}
		const auto entryPath = entry.path().extension().string();
		for (auto& ok : extensions) {
			if (entryPath == ok) {

				outPath = entry.path();
				return true;
			}
		}
	}
	return false;
}

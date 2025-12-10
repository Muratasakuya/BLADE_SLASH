#include "Assert.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/SpdLogger.h>

//============================================================================
//	Assert classMethods
//============================================================================

void Assert::DebugAssert(
	[[maybe_unused]] bool condition,
	[[maybe_unused]] const std::string& message,
	[[maybe_unused]] const char* function) {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	if (!condition) {

		std::ostringstream oss;
		oss << message << "\n"
			<< "Function: " << function << "\n";

		std::string msg = oss.str();

		// コンソール出力
		SpdLogger::Log(msg, SpdLogger::LogLevel::ASSERT_ERROR);

		// wstringに変換
		std::wstring wmsg(msg.begin(), msg.end());

		// Assert
		_ASSERT_EXPR(condition, wmsg.c_str());
	}
#endif
}

void Assert::ReleaseAssert(
	[[maybe_unused]] bool condition,
	[[maybe_unused]] const std::string& message,
	[[maybe_unused]] const char* function) {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	if (!condition) {

		std::ostringstream oss;
		oss << "Fatal Error!\n"
			<< message << "\n"
			<< "Function: " << function << "\n";

		// エラーログ出力
		SpdLogger::Log(message, SpdLogger::LogLevel::ASSERT_ERROR);

		// プログラム強制終了
		exit(EXIT_FAILURE);
	}
#endif
}

void Assert::AssertHandler(bool condition, const std::string& message, const char* function) {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)
	DebugAssert(condition, message, function);
#else
	ReleaseAssert(condition, message, function);
#endif
}
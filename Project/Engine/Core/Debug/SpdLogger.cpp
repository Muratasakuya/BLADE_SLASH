#include "SpdLogger.h"

using namespace SakuEngine;

//============================================================================
//	SpdLogger classMethods
//============================================================================

void SpdLogger::Init(const std::string& fileName, bool truncate) {

	// フォルダ作成
	const std::string logDir = "./Log/";
	if (!std::filesystem::exists(logDir)) {

		std::filesystem::create_directories(logDir);
	}

	//--- sink 構築 ------------------------------------------------------
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#if defined(_MSC_VER)
	auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#endif
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
		logDir + fileName, truncate);

	std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileSink };
#if defined(_MSC_VER)
	sinks.push_back(msvcSink);
#endif

	//--- logger インスタンス作成 ---------------------------------------
	logger_ = std::make_shared<spdlog::logger>("engine", sinks.begin(), sinks.end());
	spdlog::register_logger(logger_);
	spdlog::set_default_logger(logger_);

	logger_->set_level(spdlog::level::trace); // 全レベルを許可
	logger_->flush_on(spdlog::level::err);    // error 以上で自動 flush

	// 出力フォーマット
	spdlog::set_pattern("[%H:%M:%S.%e] %v");
}

void SpdLogger::InitGame(const std::string& fileName, bool truncate) {

	// フォルダ作成
	const std::string logDir = "./Log/";
	if (!std::filesystem::exists(logDir)) {
		std::filesystem::create_directories(logDir);
	}

	//--- sink 構築 ------------------------------------------------------
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
		logDir + fileName, truncate);
	std::vector<spdlog::sink_ptr> sinks{ fileSink };

	//--- logger インスタンス作成 ---------------------------------------
	gameLogger_ = std::make_shared<spdlog::logger>("game", sinks.begin(), sinks.end());
	spdlog::register_logger(gameLogger_);

	gameLogger_->set_level(spdlog::level::trace);
	gameLogger_->flush_on(spdlog::level::err);
	gameLogger_->set_pattern("[%H:%M:%S.%e] [%n] %v");
}


void SpdLogger::InitAsset(const std::string& fileName, bool truncate) {

	// フォルダ作成
	const std::string logDir = "./Log/";
	if (!std::filesystem::exists(logDir)) {

		std::filesystem::create_directories(logDir);
	}

	//--- sink 構築 ------------------------------------------------------
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logDir + fileName, truncate);
	std::vector<spdlog::sink_ptr> sinks{ fileSink };

	//--- logger インスタンス作成 ---------------------------------------
	assetLogger_ = std::make_shared<spdlog::logger>("asset", sinks.begin(), sinks.end());
	spdlog::register_logger(assetLogger_);

	assetLogger_->set_level(spdlog::level::trace);
	assetLogger_->flush_on(spdlog::level::err);

	// 出力フォーマット
	assetLogger_->set_pattern("[%H:%M:%S.%e] [%n] %v");
}

void SpdLogger::Log(const std::string& message, LogLevel level) {

	switch (level) {
	case LogLevel::INFO:         logger_->info(message);    break;
	case LogLevel::ASSERT_ERROR: logger_->critical(message); break;
	}
}

void SpdLogger::LogGame(const std::string& message, LogLevel level) {

	if (!gameLogger_) return;
	switch (level) {
	case LogLevel::INFO:         gameLogger_->info(message);    break;
	case LogLevel::ASSERT_ERROR: gameLogger_->critical(message); break;
	}
}

#include "DxShaderCompiler.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Asset/Filesystem.h>

//============================================================================
//	DxShaderCompiler classMethods
//============================================================================

void DxShaderCompiler::Init() {

	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));

	includeHandler_ = nullptr;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

void DxShaderCompiler::Compile(const Json& json, CompiledShaders& outShaders) {

	// リセット
	outShaders = {};

	// 基底シェーダーファイルパス
	const fs::path basePath = "./Assets/Engine/Shaders/";
	fs::path fullPath;

	// シェーダーステージごとにコンパイル処理を行うラムダ関数
	auto CompileStage = [&](const char* key, const wchar_t* profile, ComPtr<IDxcBlob>& dst) {

		// JSONに該当ステージの定義がない場合はスキップ
		if (!key || !json.contains("ShaderPass")) {
			return;
		}
		// JSONのShaderPass配列を走査して、該当ステージの定義を探す
		for (const auto& shaderPass : json["ShaderPass"]) {

			// パスのタイプが定義されていない場合はスキップ
			if (!shaderPass.contains("Type")) {
				continue;
			}
			// Graphics/Compute/DXR以外のパスはスキップ
			if (std::string(shaderPass["Type"]) != "Graphics" &&
				std::string(shaderPass["Type"]) != "Compute" &&
				std::string(shaderPass["Type"]) != "DXR") {
				continue;
			}
			// 該当ステージの定義がない場合はスキップ
			if (!shaderPass.contains(key)) {
				continue;
			}

			// ファイルパスを取得して、存在するか確認
			std::string file = shaderPass[key];
			if (!Filesystem::Found(basePath, file, fullPath)) {
				ASSERT(false, "Failed to find HLSL file: " + file);
			}

			// シェーダーをコンパイルしてBlobを生成
			ComPtr<IDxcBlob> blob;
			CompileShader(fullPath.string(), fullPath.wstring(), profile, blob, L"main");
			dst = blob;
			return;
		}
		};

	// 各ステージのコンパイル処理を実行
	CompileStage("VertexShader", L"vs_6_0", outShaders.vs);
	CompileStage("AmplificationShader", L"as_6_5", outShaders.as);
	CompileStage("MeshShader", L"ms_6_5", outShaders.ms);

	// 一度クリアする
	fullPath.clear();

	// ピクセルはプロファイルに応じてで切替
	{
		for (const auto& shaderPass : json["ShaderPass"]) {

			// パスのタイプが定義されていない場合はスキップ
			if (!shaderPass.contains("Type") ||
				std::string(shaderPass["Type"]) != "Graphics" ||
				!shaderPass.contains("PixelShader")) {
				continue;
			}

			const wchar_t* profile = L"ps_6_0";
			// DXRパスならps_6_6、そうでなくてもIsDXRフラグがあればps_6_5を使う
			if (shaderPass.contains("PSProfile")) { 
				profile = L"ps_6_6";
			}

			std::string file = shaderPass["PixelShader"];
			if (!Filesystem::Found(basePath, file, fullPath)) {
				ASSERT(false, "Failed to find HLSL file: " + file);
			}

			// シェーダーをコンパイルしてBlobを生成
			ComPtr<IDxcBlob> blob;
			CompileShader(fullPath.string(), fullPath.wstring(), profile, blob, L"main");
			outShaders.ps = blob;
			break;
		}
	}

	// 一度クリアする
	fullPath.clear();

	// Compute/DXRシェーダーは両方ともcs_6_0以上を使うが、DXRパスが優先される
	{
		for (const auto& shaderPass : json["ShaderPass"]) {

			// パスのタイプが定義されていない場合はスキップ
			if (!shaderPass.contains("Type")) {
				continue;
			}

			std::string type = shaderPass["Type"];
			if ((type == "Compute" || type == "DXR") &&
				shaderPass.contains("ComputeShader")) {

				// ファイルパスを取得して、存在するか確認
				std::string file = shaderPass["ComputeShader"];
				if (!Filesystem::Found(basePath, file, fullPath)) {
					ASSERT(false, "Failed to find HLSL file: " + file);
				}

				// シェーダーをコンパイルしてBlobを生成
				const wchar_t* profile = L"cs_6_0";
				if (type == "DXR") {
					profile = L"cs_6_6";
				} else if (shaderPass.contains("IsDXR")) {
					profile = L"cs_6_5";
				}
				ComPtr<IDxcBlob> blob;
				CompileShader(fullPath.string(), fullPath.wstring(), profile, blob, L"main");
				outShaders.cs = blob;
				break;
			}
		}
	}
}

void DxShaderCompiler::CompileShader(
	const std::string& fileName,
	const std::wstring& filePath, const wchar_t* profile,
	ComPtr<IDxcBlob>& shaderBlob, const wchar_t* entry) {

	// hlslファイルを読み込む
	IDxcBlobEncoding* shaderSouce = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSouce);
	// 読めなかったら止める
	ASSERT(SUCCEEDED(hr), "Failed to load HLSL file: " + fileName);
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSouce->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSouce->GetBufferSize();
	// UTF8の文字コードであることを通知
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR level = L"";
	// shader最適化設定
	level = L"-O3";

	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E",entry,
		L"-T",profile,
		L"-Zi",L"-Qembed_debug",
		level,
		L"-Zpr",
	};
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler_.Get(),
		IID_PPV_ARGS(&shaderResult));
	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	ComPtr<IDxcBlobUtf8> shaderError = nullptr;

	// エラー情報を取得
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);

	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		// エラーの内容
		const char* errorMessage = reinterpret_cast<const char*>(shaderError->GetBufferPointer());
		errorMessage;

		ASSERT(false, "Failed to compile HLSL file: " + std::string(errorMessage));
	}

	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// もう使わないリソースを解放
	shaderSouce->Release();
	shaderResult->Release();
	shaderError.Reset();
}

void DxShaderCompiler::CompileShaderLibrary(const std::wstring& filePath,
	const std::wstring& exports, ComPtr<IDxcBlob>& shaderBlob) {

	// hlslファイルを読み込む
	IDxcBlobEncoding* shaderSouce = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSouce);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSouce->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSouce->GetBufferSize();
	// UTF8の文字コードであることを通知
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR level = L"";
#if defined(_DEBUG)
	level = L"-Od";
#else
	level = L"-O3";
#endif

	std::wstring exportArg = L"-exports";
	DxcBuffer srcBuf{};

	LPCWSTR args[] = {
		filePath.c_str(),
		L"-T", L"lib_6_6",
		exportArg.c_str(), exports.c_str(),
		L"-Zi", L"-Qembed_debug",
		level,
		L"-Zpr",
	};

	ComPtr<IDxcResult> result;
	hr = dxcCompiler_->Compile(&shaderSourceBuffer,
		args, _countof(args),
		includeHandler_.Get(),
		IID_PPV_ARGS(&result));

	ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	// エラー情報を取得
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);

	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		// エラーの内容
		const char* errorMessage = reinterpret_cast<const char*>(shaderError->GetBufferPointer());
		errorMessage;
		assert(false);
	}

	hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// もう使わないリソースを解放
	shaderSouce->Release();
	result->Release();
	shaderError.Reset();
}

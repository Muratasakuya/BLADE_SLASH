#include "DxShaderCompiler.h"

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

void DxShaderCompiler::Compile(const Json& json, std::vector<ComPtr<IDxcBlob>>& shaderBlobs) {

	// baseShaderPath
	const fs::path basePath = "./Assets/Engine/Shaders/";
	fs::path fullPath;

	for (const auto& shaderPass : json["ShaderPass"]) {
		if (shaderPass.contains("Type")) {

			std::string type = shaderPass["Type"];
			if (type == "Graphics") {

				// vertexShaderのcompile
				if (shaderPass.contains("VertexShader")) {

					std::string vertexShader = shaderPass["VertexShader"];
					if (Filesystem::Found(basePath, vertexShader, fullPath)) {
						ComPtr<IDxcBlob> vertexShaderBlob;
						CompileShader(fullPath.string(), fullPath.wstring(), L"vs_6_0", vertexShaderBlob, L"main");
						shaderBlobs.emplace_back(vertexShaderBlob);
					} else {

						ASSERT(false, "Failed to find HLSL file: " + vertexShader);
					}
				}

				// meshShaderのcompile
				if (shaderPass.contains("MeshShader")) {

					std::string meshShader = shaderPass["MeshShader"];
					if (Filesystem::Found(basePath, meshShader, fullPath)) {

						ComPtr<IDxcBlob> meshShaderBlob;
						CompileShader(fullPath.string(), fullPath.wstring(), L"ms_6_5", meshShaderBlob, L"main");
						shaderBlobs.emplace_back(meshShaderBlob);
					} else {

						ASSERT(false, "Failed to find HLSL file: " + meshShader);
					}
				}

				// pixelShaderのcompile
				if (shaderPass.contains("PixelShader")) {

					const wchar_t* profile = L"ps_6_0";
					if (shaderPass.contains("PSProfile")) {

						profile = L"ps_6_6";
					}

					std::string pixelShader = shaderPass["PixelShader"];
					if (Filesystem::Found(basePath, pixelShader, fullPath)) {

						ComPtr<IDxcBlob> pixelShaderBlob;
						CompileShader(fullPath.string(), fullPath.wstring(), profile, pixelShaderBlob, L"main");
						shaderBlobs.emplace_back(pixelShaderBlob);
					} else {

						ASSERT(false, "Failed to find HLSL file: " + pixelShader);
					}
				}
			} else if (type == "Compute" && shaderPass.contains("ComputeShader")) {

				// computeShaderのcompile
				std::string computeShader = shaderPass["ComputeShader"];
				if (Filesystem::Found(basePath, computeShader, fullPath)) {
					if (shaderPass.contains("IsDXR")) {

						ComPtr<IDxcBlob> computeShaderBlob;
						CompileShader(fullPath.string(), fullPath.wstring(), L"cs_6_5", computeShaderBlob, L"main");
						shaderBlobs.push_back(computeShaderBlob);
					} else {

						ComPtr<IDxcBlob> computeShaderBlob;
						CompileShader(fullPath.string(), fullPath.wstring(), L"cs_6_0", computeShaderBlob, L"main");
						shaderBlobs.push_back(computeShaderBlob);
					}
				} else {

					ASSERT(false, "Failed to find HLSL file: " + computeShader);
				}
			} else if (type == "DXR" && shaderPass.contains("ComputeShader")) {

				// DXRのcompile
				std::string computeShader = shaderPass["ComputeShader"];
				if (Filesystem::Found(basePath, computeShader, fullPath)) {

					ComPtr<IDxcBlob> computeShaderBlob;
					CompileShader(fullPath.string(), fullPath.wstring(), L"cs_6_6", computeShaderBlob, L"main");
					shaderBlobs.push_back(computeShaderBlob);
				} else {

					ASSERT(false, "Failed to find HLSL file: " + computeShader);
				}
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

		ASSERT(false , "Failed to compile HLSL file: " + std::string(errorMessage));
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
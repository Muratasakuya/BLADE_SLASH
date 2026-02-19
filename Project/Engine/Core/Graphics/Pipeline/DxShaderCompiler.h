#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
#include <dxcapi.h>
// c++
#include <vector>
// json
#include <Externals/nlohmann/json.hpp>

namespace SakuEngine {

	// namespace using
	namespace fs = std::filesystem;
	using Json = nlohmann::json;

	//============================================================================
	//	DxShaderCompiler structures
	//============================================================================

	// シェーダーデータ
	struct CompiledShaders {

		// 各ステージのシェーダ情報
		ComPtr<IDxcBlob> vs;
		ComPtr<IDxcBlob> as;
		ComPtr<IDxcBlob> ms;
		ComPtr<IDxcBlob> ps;
		ComPtr<IDxcBlob> cs;

		// ステージの有無をチェックするためのアクセサ
		bool HasVS() const { return vs.Get() != nullptr; }
		bool HasAS() const { return as.Get() != nullptr; }
		bool HasMS() const { return ms.Get() != nullptr; }
		bool HasPS() const { return ps.Get() != nullptr; }
		bool HasCS() const { return cs.Get() != nullptr; }

		// Computeシェーダのみかどうか
		bool IsComputeOnly() const {

			return HasCS() && !HasVS() && !HasAS() && !HasMS() && !HasPS();
		}
	};

	//============================================================================
	//	DxShaderCompiler class
	//	DXCでHLSLをDXILにコンパイルし、必要なシェーダBlob群を生成する。
	//============================================================================
	class DxShaderCompiler {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		DxShaderCompiler() = default;
		~DxShaderCompiler() = default;

		// DXCの初期化(コンパイラインターフェース/インクルードハンドラ等の準備)
		void Init();

		// JSON定義をもとにシェーダをコンパイルし、必要なシェーダBlob群を生成する
		void Compile(const Json& json, CompiledShaders& outShaders);

		// コンパイル処理
		void CompileShader(
			const std::string& fileName,
			const std::wstring& filePath,
			const wchar_t* profile,
			ComPtr<IDxcBlob>& shaderBlob,
			const wchar_t* entry);
		void CompileShaderLibrary(
			const std::wstring& filePath,
			const std::wstring& exports,
			ComPtr<IDxcBlob>& shaderBlob);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		ComPtr<IDxcUtils> dxcUtils_;
		ComPtr<IDxcCompiler3> dxcCompiler_;
		ComPtr<IDxcIncludeHandler> includeHandler_;
	};
}; // SakuEngine
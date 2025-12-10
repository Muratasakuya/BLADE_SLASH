#include "SceneConstBuffer.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Scene/SceneView.h>

//============================================================================
//	SceneConstBuffer classMethods
//============================================================================

void SceneConstBuffer::Create(ID3D12Device* device) {

	// buffer作成
	// camera
	viewProjectionBuffer_.CreateBuffer(device);
	cameraPosBuffer_.CreateBuffer(device);
	orthoProjectionBuffer_.CreateBuffer(device);
	// light
	lightBuffer_.CreateBuffer(device);

	// rayScene
	raySceneBuffer_.CreateBuffer(device);

	// rayScene
	RaySceneForGPU rayScene{};
	rayScene.rayMin = 0.01f;       // 飛ばす最小位置
	rayScene.rayMax = 10000000.0f; // 飛ばす距離
	raySceneBuffer_.TransferData(rayScene);

	// dither
	ditherBuffer_.CreateBuffer(device);

	// perView
	perViewBuffer_.CreateBuffer(device);
	debugScenePerViewBuffer_.CreateBuffer(device);

	// debug
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// camera
	debugSceneViewProjectionBuffer_.CreateBuffer(device);
	debugSceneCameraPosBuffer_.CreateBuffer(device);
#endif
}

void SceneConstBuffer::Update(class SceneView* sceneView) {

	// buffer更新
	// camera
	viewProjectionBuffer_.TransferData(sceneView->GetCamera()->GetViewProjectionMatrix());
	cameraPosBuffer_.TransferData(sceneView->GetCamera()->GetTransform().translation);
	orthoProjectionBuffer_.TransferData(sceneView->GetCamera2D()->GetViewProjectionMatrix());
	// light
	lightBuffer_.TransferData(sceneView->GetLight()->GetPunctualLight());

	// dither
	ditherBuffer_.TransferData(sceneView->GetDither());

	// perView
	ParticleCommon::PerViewForGPU perView{};
	perView.cameraPos = sceneView->GetCamera()->GetTransform().translation;
	perView.viewProjection = sceneView->GetCamera()->GetViewProjectionMatrix();
	perView.billboardMatrix = sceneView->GetCamera()->GetBillboardMatrix();
	perViewBuffer_.TransferData(perView);

	// debug
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// camera
	debugSceneViewProjectionBuffer_.TransferData(sceneView->GetSceneCamera()->GetViewProjectionMatrix());
	debugSceneCameraPosBuffer_.TransferData(sceneView->GetSceneCamera()->GetTransform().translation);

	perView.cameraPos = sceneView->GetSceneCamera()->GetTransform().translation;
	perView.viewProjection = sceneView->GetSceneCamera()->GetViewProjectionMatrix();
	perView.billboardMatrix = sceneView->GetSceneCamera()->GetBillboardMatrix();
	debugScenePerViewBuffer_.TransferData(perView);
#endif
}

void SceneConstBuffer::SetMainPassCommands(bool debugEnable, ID3D12GraphicsCommandList* commandList) {

	if (!debugEnable) {

		// viewProjection
		commandList->SetGraphicsRootConstantBufferView(6,
			viewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());

		// cameraPos
		commandList->SetGraphicsRootConstantBufferView(14,
			cameraPosBuffer_.GetResource()->GetGPUVirtualAddress());
	} else {

		// viewProjection
		commandList->SetGraphicsRootConstantBufferView(6,
			debugSceneViewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());

		// cameraPos
		commandList->SetGraphicsRootConstantBufferView(14,
			debugSceneCameraPosBuffer_.GetResource()->GetGPUVirtualAddress());
	}

	// light
	commandList->SetGraphicsRootConstantBufferView(13,
		lightBuffer_.GetResource()->GetGPUVirtualAddress());

	// dither
	commandList->SetGraphicsRootConstantBufferView(16,
		ditherBuffer_.GetResource()->GetGPUVirtualAddress());
}

void SceneConstBuffer::SetViewProCommand(bool debugEnable,
	ID3D12GraphicsCommandList* commandList, UINT rootIndex) {

	if (!debugEnable) {

		// viewProjection
		commandList->SetGraphicsRootConstantBufferView(rootIndex,
			viewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
	} else {

		// viewProjection
		commandList->SetGraphicsRootConstantBufferView(rootIndex,
			debugSceneViewProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
	}
}

void SceneConstBuffer::SetPerViewCommand(bool debugEnable,
	ID3D12GraphicsCommandList* commandList, UINT rootIndex) {

	if (!debugEnable) {

		// perView
		commandList->SetGraphicsRootConstantBufferView(rootIndex,
			perViewBuffer_.GetResource()->GetGPUVirtualAddress());
	} else {

		// perView
		commandList->SetGraphicsRootConstantBufferView(rootIndex,
			debugScenePerViewBuffer_.GetResource()->GetGPUVirtualAddress());
	}
}

void SceneConstBuffer::SetOrthoProCommand(ID3D12GraphicsCommandList* commandList, UINT rootIndex) {

	commandList->SetGraphicsRootConstantBufferView(rootIndex,
		orthoProjectionBuffer_.GetResource()->GetGPUVirtualAddress());
}

void SceneConstBuffer::SetRaySceneCommand(ID3D12GraphicsCommandList* commandList, UINT rootIndex) {

	commandList->SetGraphicsRootConstantBufferView(rootIndex, raySceneBuffer_.GetResource()->GetGPUVirtualAddress());
}
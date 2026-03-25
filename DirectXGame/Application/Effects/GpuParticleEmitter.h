#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <array>
#include <cstdint>
#include <vector>

#include "3d/Camera.h"
#include "math/Matrix4x4.h"
#include "math/Vector3.h"

class GpuParticleEmitter {
public:
	struct Particle {
		KamataEngine::Vector3 position;
		float scale = 0.0f;
		KamataEngine::Vector3 velocity;
		float life = 0.0f;
		float age = 0.0f;
		float startScale = 0.0f;
		float endScale = 0.0f;
		float active = 0.0f;
	};

	void Initialize(uint32_t maxParticles);
	void Update(float dt);
	void Draw(const KamataEngine::Camera* camera);
	void Emit(const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, float life, float startScale, float endScale);
	void SetColor(float r, float g, float b, float a);

private:
	void CreatePipeline_();
	void CreateVertexBuffer_();
	void CreateParticleBuffer_();
	void CreateConstantBuffer_();

private:
	struct Constants {
		KamataEngine::Matrix4x4 view;
		KamataEngine::Matrix4x4 projection;
		float color[4];
	};

	uint32_t maxParticles_ = 0;
	uint32_t nextSpawnIndex_ = 0;
	std::vector<Particle> particles_;
	std::array<float, 4> color_ = { 0.65f, 0.65f, 0.65f, 0.7f };

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffer_;
	Particle* mappedParticles_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
	Constants* mappedConstants_ = nullptr;
	uint32_t constantBufferSize_ = 0;
};
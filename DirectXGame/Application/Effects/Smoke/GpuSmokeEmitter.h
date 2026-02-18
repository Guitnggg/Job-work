#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <cstdint>
#include <vector>

#include "3d/Camera.h"
#include "math/Matrix4x4.h"
#include "math/Vector3.h"

/// <summary>
/// GPUインスタンシングによるスモーク描画クラス
/// ・CPU側でパーティクル更新
/// ・StructuredBuffer として GPU に転送
/// ・頂点シェーダー側でインスタンシング描画
/// </summary>
class GpuSmokeEmitter {
public:
	/// <summary>
	/// パーティクルデータ構造体
	/// </summary>
	struct SmokeParticle {
		KamataEngine::Vector3 position;  // ワールド座標
		float scale = 0.0f;              // 現在スケール

		KamataEngine::Vector3 velocity;  // 速度ベクトル
		float life = 0.0f;               // 寿命

		float age = 0.0f;         // 経過時間
		float startScale = 0.0f;  // 初期スケール
		float endScale = 0.0f;    // 終了スケール
		float active = 0.0f;      // 有効フラグ
	};

	GpuSmokeEmitter() = default;
	~GpuSmokeEmitter() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="maxParticles">最大パーティクル数</param>
	void Initialize(uint32_t maxParticles);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="dt">Δtime</param>
	void Update(float dt);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="camera">描画に使うカメ</param>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="position"></param>
	/// <param name="velocity"></param>
	/// <param name="life"></param>
	/// <param name="startScale"></param>
	/// <param name="endScale"></param>
	void Emit(const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity, float life, float startScale, float endScale);

private:
	/// <summary>
	/// パイプライン生成
	/// </summary>
	void CreatePipeline_();

	/// <summary>
	/// 頂点バッファ生成
	/// </summary>
	void CreateVertexBuffer_();

	/// <summary>
	/// パイプラインバッファ生成
	/// </summary>
	void CreateParticleBuffer_();

	/// <summary>
	/// 定数バッファ生成
	/// </summary>
	void CreateConstantBuffer_();

private:
	/// <summary>
	/// シェーダーへ渡す定数データ
	/// </summary>
	struct Constants {
		KamataEngine::Matrix4x4 view;
		KamataEngine::Matrix4x4 projection;
		float color[4];
	};

	uint32_t maxParticles_ = 0;    // 最大数
	uint32_t nextSpawnIndex_ = 0;  // 次に上書きするインデックス

	std::vector<SmokeParticle> particles_;  // CPU側パーティクル配列

	// ===== GPUリソース =====
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffer_;  // StructuredBuffer
	SmokeParticle* mappedParticles_ = nullptr;               // CPUマップ領域

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;  // SRVヒープ

	Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;  // 定数バッファ
	Constants* mappedConstants_ = nullptr;

	uint32_t constantBufferSize_ = 0;
};
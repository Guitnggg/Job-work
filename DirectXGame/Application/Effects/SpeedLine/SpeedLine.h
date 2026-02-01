#pragma once

#include <KamataEngine.h>
#include <random>
#include <vector>

/// <summary>
/// スピード感を演出するためのラインパーティクル群。
/// プレイヤー前方に多数のラインを配置し、
/// カメラ方向へ流すことで高速移動の視覚効果を作る。
/// </summary>
class SpeedLine {
public:
	SpeedLine();
	~SpeedLine();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="camera">描画に使用するカメラ</param>
	/// <param name="lineCount">生成するライン本数</param>
	void Initialize(KamataEngine::Camera* camera, int32_t lineCount = kDefaultLineCount);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="dt">デルタタイム（秒）</param>
	/// <param name="playerPos">プレイヤーのワールド座標</param>
	void Update(float dt, const KamataEngine::Vector3& playerPos);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private:
	/// <summary>
	/// スピードライン1本分のデータ
	/// </summary>
	struct LineParticle {
		KamataEngine::WorldTransform* worldTransform = nullptr; // 変換情報（所有）
		float speed = 0.0f;                                     // 移動速度
		float alpha = 1.0f;                                     // 透明度（将来拡張用）
	};

private:
	/// <summary>
	/// ラインを再配置する（プレイヤー前方へ）
	/// </summary>
	/// <param name="particle">対象ライン</param>
	/// <param name="basePos">基準位置（プレイヤー）</param>
	/// <param name="randomDepth">Z をランダムにするか</param>
	void Respawn_(LineParticle& particle, const KamataEngine::Vector3& basePos, bool randomDepth);

private:
	// ===== 定数 =====

	// 本数
	static constexpr int32_t kDefaultLineCount = 80;

	// 円周率関連
	static constexpr float kTwoPi = 6.28318530717958647692f;

	// 配置範囲
	static constexpr float kMaxRadius = 25.0f;
	static constexpr float kInnerRadiusRate = 0.8f; // 中央を空ける割合
	static constexpr float kYFlattenRate = 0.6f;    // Y方向のつぶし

	// Z方向スポーン範囲
	static constexpr float kSpawnZMin = 150.0f;
	static constexpr float kSpawnZMax = 250.0f;
	static constexpr float kDespawnOffsetZ = -10.0f;

	// 見た目
	static constexpr float kLineScale = 0.15f;

	// 移動速度
	static constexpr float kSpeedMin = 45.0f;
	static constexpr float kSpeedMax = 85.0f;

	// 透明度（将来用）
	static constexpr float kAlphaBase = 0.10f;
	static constexpr float kAlphaRange = 0.25f;

	// ゆらぎ
	static constexpr float kJitterX = 0.02f;
	static constexpr float kJitterY = 0.01f;

private:
	KamataEngine::Model* model_ = nullptr;   // 描画モデル
	KamataEngine::Camera* camera_ = nullptr; // 使用カメラ

	std::vector<LineParticle> lines_; // ライン群
	std::mt19937 random_;             // 乱数生成器
};

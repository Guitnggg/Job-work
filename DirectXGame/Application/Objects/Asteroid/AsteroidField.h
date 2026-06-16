#pragma once

#include <memory>
#include <random>
#include <vector>

#include "Application/Objects/Asteroid/Asteroid.h"

/// <summary>
/// 小惑星フィールドの設定構造体
/// </summary>
struct AsteroidFieldConfig {
	int count = 10;
	float spawnZMin = 0.0f;
	float spawnZMax = 140.0f;
	float recycleZ = -50.0f;
	float spawnInterval = 1.0f;
	float rangeX = 25.0f;
	float rangeY = 15.0f;
	float speedMin = -0.3f;
	float speedMax = -0.1f;
	float rotationMin = 0.01f;
	float rotationMax = 0.03f;
};

/// <summary>
/// 小惑星の背景演出をまとめて扱うクラス
/// </summary>
class AsteroidField {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const AsteroidFieldConfig& config);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(const KamataEngine::Camera& camera) const;

private:
	// 固定デルタタイム
	// 小惑星のスポーン
	void SpawnAsteroid();
	float Rand(float min, float max);

private:
	// 設定値
	AsteroidFieldConfig config_{};

	// 小惑星モデル
	std::unique_ptr<KamataEngine::Model> asteroidModel_;
	std::vector<std::unique_ptr<Asteroid>> asteroids_;

	// スポーンタイマー
	float spawnTimer_ = 0.0f;
	std::mt19937 mt_{std::random_device{}()};
};

#include "SpeedLine.h"

#include <cmath>

using namespace KamataEngine;

SpeedLine::SpeedLine() = default;

SpeedLine::~SpeedLine() = default;

void SpeedLine::Initialize(Camera* camera, int32_t lineCount) {
	camera_ = camera;

	// モデル読み込み（仮：専用モデルがあれば差し替え）
	model_.reset(Model::CreateFromOBJ("Asteroid", true));

	// 乱数初期化
	std::random_device rd;
	random_ = std::mt19937(rd());

	// ライン確保
	lines_.resize(lineCount);

	const Vector3 dummyPlayerPos{0.0f, 0.0f, 0.0f};
	for (auto& line : lines_) {
		line.worldTransform = std::make_unique<WorldTransform>();

		Respawn_(line, dummyPlayerPos, true);
	}
}

void SpeedLine::Respawn_(LineParticle& p, const Vector3& basePos, bool randomDepth) {
	if (!p.worldTransform) {
		return;
	}

	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
	std::uniform_real_distribution<float> distAngle(0.0f, kTwoPi);

	const float u = dist01(random_);
	const float angle = distAngle(random_);

	const float innerRadius = kMaxRadius * kInnerRadiusRate;
	const float outerRadius = kMaxRadius;

	// 外側寄り分布
	const float t = u * u;
	const float r = innerRadius + (outerRadius - innerRadius) * t;

	const float x = std::cos(angle) * r;
	const float y = std::sin(angle) * r * kYFlattenRate;

	float z = kSpawnZMin;
	if (randomDepth) {
		std::uniform_real_distribution<float> distZ(kSpawnZMin, kSpawnZMax);
		z = distZ(random_);
	}

	auto& wt = *p.worldTransform;
	wt.translation_ = {basePos.x + x, basePos.y + y, basePos.z + z};
	wt.scale_ = {kLineScale, kLineScale, kLineScale};
	wt.rotation_ = {0.0f, 0.0f, 0.0f};

	std::uniform_real_distribution<float> distSpeed(kSpeedMin, kSpeedMax);
	p.speed = distSpeed(random_);

	wt.UpdateMatrix();
}

void SpeedLine::Update(float dt, const Vector3& playerPos) {
	if (!camera_ || !model_) {
		return;
	}

	for (auto& line : lines_) {
		if (!line.worldTransform) {
			continue;
		}

		auto& wt = *line.worldTransform;

		// カメラ方向へ移動
		wt.translation_.z -= line.speed * dt;

		// 微小なゆらぎ
		wt.translation_.x *= (1.0f + kJitterX * dt);
		wt.translation_.y *= (1.0f + kJitterY * dt);

		// 手前に来たら再配置
		if (wt.translation_.z < playerPos.z + kDespawnOffsetZ) {
			Respawn_(line, playerPos, false);
		}

		wt.UpdateMatrix();
	}
}

void SpeedLine::Draw() {
	if (!camera_ || !model_) {
		return;
	}

	for (auto& line : lines_) {
		if (!line.worldTransform) {
			continue;
		}

		model_->Draw(*line.worldTransform, *camera_);
	}
}

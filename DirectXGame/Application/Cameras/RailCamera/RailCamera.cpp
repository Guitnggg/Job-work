#include "RailCamera.h"

#include "Application/Utility/MyMath/MyMath.h"

using namespace KamataEngine;

//
RailCamera::RailCamera() {}

//
RailCamera::~RailCamera() {}

//
void RailCamera::Initialize() {
	// ワールド変換初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = {0.0f, 0.0f, kInitialZ};

	// カメラ生成
	camera_ = std::make_unique<Camera>();
	camera_->Initialize();
}

//
void RailCamera::Update() {
	// --- レール移動（Z方向） ---
	worldTransform_.translation_.z += kMoveSpeedZ;
	worldTransform_.UpdateMatrix();

	// --- 遅延追従 ---
	const Vector3 basePos = worldTransform_.translation_;
	Vector3 posDiff = MyMath::Subtract(basePos, lagCameraPos_);
	posDiff = MyMath::Multiply(posDiff, kFollowRate);
	lagCameraPos_ = MyMath::Add(lagCameraPos_, posDiff);

	// --- カメラシェイク更新 ---
	cameraShake_.Update();

	// --- ロール補間 ---
	const float rollDiff = targetRollAngle_ - rollAngle_;
	rollAngle_ += rollDiff * kRollFollowRate;

	// --- カメラ位置合成 ---
	Vector3 finalCameraPos = MyMath::Add(lagCameraPos_, cameraShake_.GetOffset());
	finalCameraPos.x += rollAngle_ * kRollPosOffsetX;
	finalCameraPos.y += rollAngle_ * kRollPosOffsetY;
	finalCameraPos.z += cinematicZoomZ_;

	const float zoomDiff = targetCinematicZoomZ_ - cinematicZoomZ_;
	cinematicZoomZ_ += zoomDiff * kFollowRate;

	// --- Camera へ反映 ---
	camera_->translation_ = finalCameraPos;
	camera_->rotation_.z = rollAngle_;
	camera_->UpdateMatrix();
}

// 演出用ズームの目標値を設定し、Update内で補完して反映する
void RailCamera::SetCinematicZoom(float zoomZ) { targetCinematicZoomZ_ = zoomZ; }

// 外部イベントから受け取った揺れをCameraShakeに中継する
void RailCamera::AddShake(const Vector3& dir, float power) { cameraShake_.AddShake(dir, power); }

// 左右移動入力からカメラの目標ロール角を計算する
void RailCamera::SetMoveInput(float inputX) {
	// 入力値に応じて目標ロール角を設定
	targetRollAngle_ = kMaxRollAngle * -inputX;
}

#include "RailCamera.h"

#include "Application/Utility/MyMath/MyMath.h"

using namespace KamataEngine;

RailCamera::RailCamera() {}

RailCamera::~RailCamera() {}

void RailCamera::Initialize() {
    // ワールド変換データ初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = { 0,0,kInitialZ };

    // カメラ生成
	camera_ = std::make_unique<Camera>();
    camera_->Initialize();
}

void RailCamera::Update() {

	// --- レール移動 ---
	worldTransform_.translation_.z += kMoveSpeedZ;
	worldTransform_.UpdateMatrix();

	// --- 遅延追従 ---
	const Vector3 basePos = worldTransform_.translation_;

	Vector3 posDiff = MyMath::Subtract(basePos, lagCameraPos_);
	posDiff = MyMath::Multiply(posDiff, kFollowRate);
	lagCameraPos_ = MyMath::Add(lagCameraPos_, posDiff);

	// --- カメラシェイク ---
	cameraShake_.Update();

	// --- ロール補間 ---
	float rollDiff = targetRollAngle_ - rollAngle_;
	rollAngle_ += rollDiff * kRollFollowRate;

	// --- 合成 ---
	Vector3 finalCameraPos = MyMath::Add(lagCameraPos_, cameraShake_.GetOffset());

	// --- Cameraに反映（順番重要） ---
	finalCameraPos.x += rollAngle_ * 5.0f;
	finalCameraPos.y += rollAngle_ * 0.5f;

	camera_->translation_ = finalCameraPos;
	camera_->rotation_.z = rollAngle_;	
	camera_->UpdateMatrix();
}


void RailCamera::AddShake(const Vector3& dir, float power) { cameraShake_.AddShake(dir, power); }

void RailCamera::SetMoveInput(float inputX) {
	// 左右入力→傾き反映
	targetRollAngle_ = kMaxRollAngle * -inputX;
}

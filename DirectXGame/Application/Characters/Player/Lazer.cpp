#include "Lazer.h"
#include <cmath>

using namespace KamataEngine;

// ===== Razer.cpp 専用定数（実装詳細）=====
namespace {
// 固定Δt
constexpr float kFixedDeltaTime = 1.0f / 60.0f;

// 見た目スケール（細長いレーザー）
constexpr Vector3 kRazerScale{1.0f, 1.0f, 10.0f};

// コライダー半径（少し太めにして当てやすく）
constexpr float kColliderRadius = 1.0f;

// 姿勢リセット
constexpr Vector3 kZeroRot{0.0f, 0.0f, 0.0f};
} // namespace

void Lazer::Initialize() {
	// 親クラスの初期化
	CharacterBase::Initialize();

	// 見た目
	model_.reset(Model::CreateFromOBJ("Beam", true));

	// 細長く
	worldTransform_.scale_ = kRazerScale;
	worldTransform_.UpdateMatrix();

	// コライダー
	if (collider_) {
		collider_->SetRadius(kColliderRadius);
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}
}

void Lazer::FireFrom(const Vector3& worldPos, const Vector3& dir) {
	// 座標、進行方向セット
	worldTransform_.translation_ = worldPos;
	dir_ = dir;

	// 飛行距離カウントの基準座標
	startPos_ = worldPos;

	// 姿勢リセット
	worldTransform_.rotation_ = kZeroRot;
	worldTransform_.UpdateMatrix();

	// 経過時間・生存
	t_ = 0.0f;
	isDead_ = false;

	// コライダー追従
	if (collider_) {
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}
}

void Lazer::Update() {
	if (isDead_) {
		return;
	}

	const float dt = kFixedDeltaTime;
	t_ += dt;

	// 前進（Bullet よりやや速めにしたい場合は speed_ を上げる）
	worldTransform_.translation_.x += dir_.x * speed_;
	worldTransform_.translation_.y += dir_.y * speed_;
	worldTransform_.translation_.z += dir_.z * speed_;

	worldTransform_.UpdateMatrix();

	// コライダー追従
	if (collider_) {
		collider_->SetTranslate(GetWorldTranslation());
		collider_->Update();
	}

	// 発射位置からの距離を計算して一定距離を超えたら消す
	const Vector3 p = GetWorldTranslation();
	Vector3 diff = {p.x - startPos_.x, p.y - startPos_.y, p.z - startPos_.z};
	float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

	if (distanceSq > maxDistance_ * maxDistance_) {
		isDead_ = true;
	}

	// 寿命でも消す
	if (t_ >= lifeTimeSec_) {
		isDead_ = true;
	}
}

void Lazer::Draw(const Camera* camera) {
	if (!camera || isDead_) {
		return;
	}
	if (model_) {
		model_->Draw(worldTransform_, *camera, textureHandle_);
	}
}

void Lazer::OnCollision(CharacterBase* /*other*/) {
	// レーザーの挙動は好みで切替：
	// 1) 1発で消えるレーザー：isDead_ = true;
	// 2) 貫通レーザー：何もしない（デフォルト）
	//
	// まずは「貫通」だと強すぎる場合があるので、必要なら下を有効化してください。
	// isDead_ = true;
}

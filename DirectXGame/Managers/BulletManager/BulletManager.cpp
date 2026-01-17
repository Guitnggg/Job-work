#include "BulletManager.h"
#include <algorithm>

using namespace KamataEngine;

namespace {
// 弾の基本前方（+Z）
Vector3 kForward{0.0f, 0.0f, 1.0f};

// 固定Δt（Bullet/Razer と同じ想定）
constexpr float kFixedDeltaTime = 1.0f / 60.0f;
} // namespace

void BulletManager::Initialize() {
	bullets_.clear();
	razers_.clear();

	fireCooldownFrames_ = 0;

	isCharging_ = false;
	chargeFrames_ = 0;
	prevShootDown_ = false;
}

void BulletManager::HandleShooting(Input* input, Player* player, const CountDown& countDown) {
	// カウントダウン中は入力無効
	if (!input || !player || countDown.IsInputLocked()) {
		// 入力がロックされている間はチャージ状態もリセットしておく
		isCharging_ = false;
		chargeFrames_ = 0;
		prevShootDown_ = false;
		return;
	}

	// クールダウン経過
	if (fireCooldownFrames_ > 0) {
		--fireCooldownFrames_;
	}

	// ここでは「SPACE：押し続けるとチャージ、離すと発射」という動作にする
	const bool down = input->PushKey(DIK_SPACE);

	// ReleaseKey が無いので前フレームとの差分で離し判定を作る
	const bool released = (prevShootDown_ && !down);
	prevShootDown_ = down;

	// 押し始め
	if (down && !isCharging_) {
		// クールダウン中はチャージ開始できない（誤射防止）
		if (fireCooldownFrames_ != 0) {
			return;
		}
		isCharging_ = true;
		chargeFrames_ = 0;
	}

	// チャージ中
	if (isCharging_ && down) {
		++chargeFrames_;
		if (chargeFrames_ > kChargeLaserMaxFrames) {
			chargeFrames_ = kChargeLaserMaxFrames;
		}
	}

	// 離した瞬間に発射
	if (released && isCharging_) {
		const Vector3 muzzle = player->GetWorldTranslation();

		if (chargeFrames_ >= kChargeLaserMinFrames) {
			// ===== レーザー（Razer）発射 =====
			auto r = std::make_unique<Lazer>();
			r->Initialize();

			// チャージ量に応じて「速度」「寿命」を少しだけ伸ばす（演出用）
			const float t = static_cast<float>(chargeFrames_ - kChargeLaserMinFrames) / static_cast<float>(kChargeLaserMaxFrames - kChargeLaserMinFrames);
			const float clampT = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);

			r->SetSpeed(3.2f + 1.6f * clampT);      // 3.2～4.8
			r->SetLifeTime(0.35f + 0.25f * clampT); // 0.35～0.60（短め）
			r->FireFrom(muzzle, kForward);

			razers_.push_back(std::move(r));

			// レーザー後はクールダウン長め
			fireCooldownFrames_ = kLaserCooldownMax;
		} else {
			// ===== 通常弾発射 =====
			auto b = std::make_unique<Bullet>();
			b->Initialize();
			b->FireFrom(muzzle, kForward);
			bullets_.push_back(std::move(b));

			fireCooldownFrames_ = kFireCooldownMax;
		}

		// リセット
		isCharging_ = false;
		chargeFrames_ = 0;
	}
}

void BulletManager::UpdateBullets() {
	// 通常弾更新
	for (auto& b : bullets_) {
		if (b) {
			b->Update();
		}
	}

	// レーザー更新
	for (auto& r : razers_) {
		if (r) {
			r->Update();
		}
	}

	// 死亡した弾を削除
	RemoveDeadBullets();
}

void BulletManager::RemoveDeadBullets() {
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const std::unique_ptr<Bullet>& b) { return !b || b->IsDead(); }), bullets_.end());

	razers_.erase(std::remove_if(razers_.begin(), razers_.end(), [](const std::unique_ptr<Lazer>& r) { return !r || r->IsDead(); }), razers_.end());
}

void BulletManager::Update(Input* input, Player* player, const CountDown& countDown) {
	HandleShooting(input, player, countDown);
	UpdateBullets();
}

void BulletManager::Draw(Camera* camera) {
	// 通常弾
	for (auto& b : bullets_) {
		b->Draw(camera);
	}
	// レーザー
	for (auto& r : razers_) {
		r->Draw(camera);
	}
}

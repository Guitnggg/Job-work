#include "BulletManager.h"

#include <algorithm>

#include "Managers/EnemyManager/EnemyManager.h"

using namespace KamataEngine;

namespace {
constexpr Vector3 kForward{0.0f, 0.0f, 1.0f};
} // namespace

void BulletManager::Initialize() {
	bullets_.clear();
	lasers_.clear();
	homingMissiles_.clear();
	lockedTargets_.clear();

	fireCooldownFrames_ = 0;
	isHomingLocking_ = false;
	homingLockFrame_ = 0;
	homingCooldownFrames_ = 0;
	wasHomingPressing_ = false;
}

float BulletManager::GetHomingCooldownRate() const {
	const float rate = 1.0f - static_cast<float>(homingCooldownFrames_) / static_cast<float>(kHomingCooldownMaxFrame);
	return std::clamp(rate, 0.0f, 1.0f);
}

void BulletManager::HandleShooting_(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager) {
	if (!input || !player || countDown.IsInputLocked()) {
		return;
	}

	// クールダウン経過
	if (fireCooldownFrames_ > 0) {
		--fireCooldownFrames_;
	}

	if (!input->IsTriggerMouse(0) || fireCooldownFrames_ > 0) {
		HandleHomingMissile_(input, player, countDown, enemyManager);
		return;
	}

	Vector3 dir = shootDir;
	const float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
	if (lenSq <= 0.000001f) {
		dir = kForward;
	}

	auto b = std::make_unique<Bullet>();
	b->Initialize();
	b->FireFrom(player->GetWorldTranslation(), dir);
	bullets_.push_back(std::move(b));

	fireCooldownFrames_ = kFireCooldownMax;

	HandleHomingMissile_(input, player, countDown, enemyManager);
}

void BulletManager::HandleHomingMissile_(KamataEngine::Input* input, Player* player, const CountDown& countDown, EnemyManager* enemyManager) {
	if (!input || !player || !enemyManager || countDown.IsInputLocked()) {
		isHomingLocking_ = false;
		lockedTargets_.clear();
		homingLockFrame_ = 0;
		wasHomingPressing_ = false;
		return;
	}

	if (homingCooldownFrames_ > 0) {
		--homingCooldownFrames_;
	}

	// 右クリック押下でロック開始
	if (input->IsTriggerMouse(1) && homingCooldownFrames_ <= 0) {
		isHomingLocking_ = true;
		homingLockFrame_ = 0;
		lockedTargets_.clear();
	}

	if (!isHomingLocking_) {
		return;
	}

	++homingLockFrame_;
	if (homingLockFrame_ >= kHomingLockStartFrame) {
		lockedTargets_ = enemyManager->GetNearestEnemies(player->GetWorldTranslation(), kHomingMaxLockCount);
	}

	const bool isPressing = input->IsPressMouse(1);
	const bool released = wasHomingPressing_ && !isPressing;
	wasHomingPressing_ = isPressing;
	const bool lockCompleted = homingLockFrame_ >= kHomingLockMaxFrame;
	if (!released && !lockCompleted) {
		return;
	}

	for (CharacterBase* target : lockedTargets_) {
		if (!target || target->IsDead()) {
			continue;
		}

		auto missile = std::make_unique<HomingMissile>();
		missile->Initialize();
		missile->FireFrom(player->GetWorldTranslation(), target);
		homingMissiles_.push_back(std::move(missile));
	}

	homingCooldownFrames_ = kHomingCooldownMaxFrame;
	isHomingLocking_ = false;
	homingLockFrame_ = 0;
	lockedTargets_.clear();
}

void BulletManager::ValidateHomingTargets_(EnemyManager* enemyManager){
	if (!enemyManager) {
		for (auto& m : homingMissiles_) {
			if (m) {
				m->ClearTarget();
			}
		}
		return;
	}

	auto& enemies = enemyManager->GetEnemies();
	for (auto& m : homingMissiles_) {
		if (!m || m->IsDead()) {
			continue;
		}

		CharacterBase* target = m->GetTarget();
		if (!target) {
			continue;
		}

		const bool exists = std::any_of(enemies.begin(), enemies.end(), [target](const std::unique_ptr<CharacterBase>& e) {
			return e && e.get() == target && !e->IsDead();
			});

		if (!exists) {
			m->ClearTarget();
		}
	}
}

void BulletManager::UpdateBullets_(EnemyManager*enemyManager) {
	// 通常弾更新
	for (auto& b : bullets_) {
		if (b) {
			b->Update();
		}
	}

	// レーザー更新
	for (auto& r : lasers_) {
		if (r) {
			r->Update();
		}
	}

	// ホーミング対象の安全性を事前確認
	ValidateHomingTargets_(enemyManager);

	// ホーミングミサイル更新
	for (auto& m : homingMissiles_) {
		if (m) {
			m->Update();
		}
	}

	// 死亡した弾を削除
	RemoveDeadBullets_();
}


void BulletManager::RemoveDeadBullets_() {
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const std::unique_ptr<Bullet>& b) { return !b || b->IsDead(); }), bullets_.end());

	lasers_.erase(std::remove_if(lasers_.begin(), lasers_.end(), [](const std::unique_ptr<Laser>& r) { return !r || r->IsDead(); }), lasers_.end());

	homingMissiles_.erase(std::remove_if(homingMissiles_.begin(), homingMissiles_.end(), [](const std::unique_ptr<HomingMissile>& m) { return !m || m->IsDead(); }), homingMissiles_.end());
}

void BulletManager::Update(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager) {
	HandleShooting_(input, player, countDown, shootDir,enemyManager);
	UpdateBullets_(enemyManager);
}

void BulletManager::Draw(const Camera* camera) {
	// 通常弾
	for (auto& b : bullets_) {
		b->Draw(camera);
	}

	// レーザー
	for (auto& r : lasers_) {
		r->Draw(camera);
	}

	// ホーミングミサイル
	for (auto& m : homingMissiles_) {
		m->Draw(camera);
	}
}

#include "BossAttackController.h"

void BossAttackController::Initialize(Boss* boss) {
	boss_ = boss;
	timer_ = 0.0f;
}

void BossAttackController::Update(float dt) {
	if (!boss_) {
		return;
	}

	// 攻撃パターンは後続実装で追加する。
	timer_ += dt;
}

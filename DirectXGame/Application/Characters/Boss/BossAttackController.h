#pragma once

class Boss;

/// <summary>
/// ボス攻撃パターン追加用の差し込み口。
/// 現仕様では攻撃は未実装のため、状態更新のみ受け取る。
/// </summary>
class BossAttackController {
public:
	void Initialize(Boss* boss);
	void Update(float dt);

private:
	Boss* boss_ = nullptr;
	float timer_ = 0.0f;
};

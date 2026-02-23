#pragma once

#include <memory>
#include <vector>

#include "Application/Characters/Collider.h"
#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Player/Bullet.h"
#include "Application/Characters/Player/Player.h"
#include "Application/Characters/Player/Laser.h"
#include "Application/Characters/Player/HomingMissile.h"

#include "UI/CountDown/CountDown.h"

/// <summary>
/// 当たり判定の管理クラス。
/// ・プレイヤーと敵との衝突
/// ・弾と敵との衝突
/// を判定し、それぞれの OnCollision を呼び出す役割を持つ。
/// 本クラスは「衝突の検知・通知のみ」を行い、
/// HPの減少・被弾エフェクト・スコア処理などは各オブジェクトへ委譲する。
/// </summary>
class CollisionManager {
public:
	/// <summary>
	/// プレイヤーと複数の敵の衝突判定を行う。
	/// 衝突が発生した場合、両者の OnCollision を呼び出す。
	/// </summary>
	/// <param name="player">衝突対象のプレイヤー</param>
	/// <param name="enemies">衝突対象の敵リスト</param>
	/// <param name="countDown">カウントダウン中は衝突を無効にするため使用</param>
	static void ResolvePlayerEnemyCollisions(Player* player, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// 弾と複数の敵との衝突判定を行う。
	/// 衝突が発生した場合、敵に OnCollision、弾に OnCollision を通知し、
	/// 1発の弾につき1体の敵へ命中することを想定して処理を中断する。
	/// </summary>
	/// <param name="bullets">衝突対象の弾のリスト</param>
	/// <param name="enemies">衝突対象の敵のリスト</param>
	/// <param name="countDown">カウントダウン中は衝突を無効にするため使用</param>
	static void ResolveBulletEnemyCollisions(std::vector<std::unique_ptr<Bullet>>& bullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// レーザー(Razer)と複数の敵との衝突判定を行う。
	/// 衝突が発生した場合、敵に OnCollision、レーザーに OnCollision を通知する。
	/// </summary>
	/// <param name="razers">衝突対象のレーザーのリスト</param>
	/// <param name="enemies">衝突対象の敵のリスト</param>
	/// <param name="countDown">カウントダウン中は衝突を無効にするため使用</param>
	static void ResolveLaserEnemyCollisions(std::vector<std::unique_ptr<Laser>>& lasers, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="missiles"></param>
	/// <param name="enemies"></param>
	/// <param name="countDown"></param>
	static void ResolveHomingMissileEnemyCollisions(std::vector<std::unique_ptr<HomingMissile>>& missiles, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);
};

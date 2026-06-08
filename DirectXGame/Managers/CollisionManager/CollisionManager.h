#pragma once

#include <memory>
#include <vector>

#include "Application/Characters/Collider.h"
#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Enemy/TurretEnemy.h"
#include "Application/Characters/Player/Bullet.h"
#include "Application/Characters/Player/HomingMissile.h"
#include "Application/Characters/Player/Player.h"

#include "UI/CountDown/CountDown.h"

/// <summary>
/// プレイヤー、敵、弾、ミサイルの当たり判定をまとめる。
/// 各オブジェクトへの通知だけを行い、HPやスコアの処理は個別クラスへ任せる。
/// </summary>
class CollisionManager {
public:
	/// <summary>
	/// プレイヤーと複数の敵の衝突判定を行う
	/// 衝突が発生した場合、双方の OnCollision を呼び出す
	/// </summary>
	/// <param name="player">衝突対象のプレイヤー</param>
	/// <param name="enemies">衝突対象の敵リスト</param>
	/// <param name="countDown">カウントダウン中は衝突を無効化するために使用する</param>
	static void ResolvePlayerEnemyCollisions(Player* player, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// 通常弾と複数の敵との衝突判定を行う。
	/// 衝突が発生した場合、敵に OnCollision、弾に OnCollision を通知する。
	/// 1発の弾が複数の敵へ同時に命中しないよう、命中後は処理を中断する。
	/// </summary>
	/// <param name="bullets">衝突対象の通常弾リスト。</param>
	/// <param name="enemies">衝突対象の敵リスト。</param>
	/// <param name="countDown">カウントダウン中は衝突を無効化するために使用する。</param>
	static void ResolveBulletEnemyCollisions(std::vector<std::unique_ptr<Bullet>>& bullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// ホーミングミサイルと複数の敵との衝突判定を行う
	/// </summary>
	/// <param name="missiles">衝突対象のホーミングミサイルリスト</param>
	/// <param name="enemies">衝突対象の敵リスト</param>
	/// <param name="countDown">カウントダウン中は衝突を無効化するために使用する</param>
	static void ResolveHomingMissileEnemyCollisions(std::vector<std::unique_ptr<HomingMissile>>& missiles, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// プレイヤーとターレット弾の衝突判定を行う
	/// </summary>
	/// <param name="player">衝突対象のプレイヤー</param>
	/// <param name="enemies">ターレットを含む敵リスト</param>
	/// <param name="countDown">カウントダウン中は衝突を無効化するために使用する</param>
	static void ResolvePlayerTurretBulletCollisions(Player* player, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);

	/// <summary>
	/// プレイヤー弾とターレット弾の衝突判定を行う
	/// </summary>
	/// <param name="playerBullets">プレイヤーの通常弾リスト</param>
	/// <param name="enemies">ターレットを含む敵リスト</param>
	/// <param name="countDown">カウントダウン中は衝突を無効化するために使用する</param>
	static void ResolvePlayerBulletTurretBulletCollisions(std::vector<std::unique_ptr<Bullet>>& playerBullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown);
};

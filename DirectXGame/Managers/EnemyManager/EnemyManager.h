#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <KamataEngine.h>

#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Enemy/TurretEnemy.h"
#include "Application/Effects/Damage/GpuDamageEmitter.h"

/// <summary>
/// 敵キャラクター全体を管理するクラス。
///
/// ・JSON から敵出現データを読み込み
/// ・時間経過に応じて敵を生成
/// ・全敵の Update / Draw を一括管理
/// ・死亡した敵の削除
/// ・撃破時の爆発エフェクト生成
///
/// EnemyManager は「敵のライフサイクル管理」に専念し、
/// 個々の敵の挙動（移動・攻撃・被弾処理）は
/// 各 Enemy クラスへ委譲する。
/// </summary>
class EnemyManager {
public:
	/// <summary>
	/// 敵出現データ（JSON 1件分）
	/// </summary>
	struct EnemySpawnData {
		float time = 0.0f;
		KamataEngine::Vector3 pos{0.0f, 0.0f, 0.0f};

		// 敵タイプ（"seeker" / "turret"）
		std::string type = "seeker";

		// ===== Seeker 用 =====
		float speed = kDefaultSeekerSpeed;
		float turnRate = kDefaultSeekerTurnRate;

		// ===== 共通 =====
		int32_t hp = kDefaultHp;
		float radius = kDefaultColliderRadius;
		float lifeTime = kDefaultLifeTime;

		// ===== Turret 用 =====
		int32_t shootIntervalFrames = kDefaultShootIntervalFrames;
		float bulletSpeed = kDefaultBulletSpeed;
		float bulletLifeTime = kDefaultBulletLifeTime;
	};

public:
	/// <summary>初期化処理</summary>
	void Initialize();

	/// <summary>
	/// JSON 形式の敵出現データ読み込み
	/// </summary>
	void LoadEnemyCsv(const std::string& path);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="dt">デルタタイム（秒）</param>
	/// <param name="playerPos">プレイヤーのワールド座標</param>
	void Update(float dt, const KamataEngine::Vector3& playerPos);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 生存中の敵リスト取得（CollisionManager 用）
	/// </summary>
	std::vector<std::unique_ptr<CharacterBase>>& GetEnemies() { return enemies_; }

	/// <summary>
	/// プレイヤーから近い順の敵ポインタを最大数まで取得する
	/// </summary>
	std::vector<CharacterBase*> GetNearestEnemies(const KamataEngine::Vector3& from, int32_t maxCount) const;

	/// <summary>
	/// 死亡した敵を削除する
	/// </summary>
	void RemoveDeadEnemies();

private:
	/// <summary>
	/// 出現時間に達した敵を生成する
	/// </summary>
	void SpawnEnemiesByCsv(const KamataEngine::Vector3& playerPos);

	/// <summary>
	/// 指定位置に爆発エフェクトを生成する
	/// </summary>
	void SpawnExplosionAt(const KamataEngine::Vector3& pos);

private:
	// ===== 定数（デフォルト値）=====
	static constexpr float kDefaultSeekerSpeed = 0.2f;
	static constexpr float kDefaultSeekerTurnRate = 0.15f;
	static constexpr int32_t kDefaultHp = 1;
	static constexpr float kDefaultColliderRadius = 1.0f;
	static constexpr float kDefaultLifeTime = 30.0f;

	static constexpr int32_t kDefaultShootIntervalFrames = 60;
	static constexpr float kDefaultBulletSpeed = 2.8f;
	static constexpr float kDefaultBulletLifeTime = 3.0f;

	// ===== 爆発エフェクト =====
	static constexpr int32_t kExplosionParticleCount = 16;
	static constexpr float kExplosionSpeed = 3.0f;
	static constexpr float kExplosionLifeTime = 0.6f;
	static constexpr float kExplosionStartScale = 0.25f;
	static constexpr float kExplosionEndScale = 0.0f;

private:
	// 生存中の敵
	std::vector<std::unique_ptr<CharacterBase>> enemies_;

	// 出現待ち敵データ
	std::vector<EnemySpawnData> enemySpawnList_;

	// 経過時間
	float enemySpawnTimer_ = 0.0f;

	// 爆発エフェクト
	std::unique_ptr<GpuDamageEmitter> explosionEmitter_;
};

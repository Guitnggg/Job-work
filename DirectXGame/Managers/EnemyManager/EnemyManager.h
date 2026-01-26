#pragma once

#include <KamataEngine.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Enemy/TurretEnemy.h" 
#include "Application/Effects/Damage/DamageParticle.h"

/// <summary>
/// 敵キャラクター全体を管理するクラス。
/// ・CSV(JSON)から敵出現データを読み込み
/// ・時間経過に応じて敵を生成
/// ・全敵の Update / Draw を一括管理
/// ・死亡した敵の削除
/// ・爆発エフェクト（DamageParticle）の生成
/// 
/// EnemyManager は「敵のライフサイクル管理」に専念し、
/// 個々の敵の挙動（移動・攻撃・被弾処理）は
/// 各 Enemy クラスへ委譲する。
/// </summary>
class EnemyManager {
public:
	/// <summary>
	/// 敵出現データ（Json１件分）
	/// </summary>
	struct EnemySpawnData {
		float time = 0.0f;
		KamataEngine::Vector3 pos{0, 0, 0};

		// 敵タイプ（"seeker" / "turret"）
		std::string type = "seeker";

		// ===== Seeker用 =====
		float speed = 0.2f;
		float turnRate = 0.15f;

		// ===== 共通 =====
		int hp = 1;
		float radius = 1.0f;
		float lifeTime = 30.0f;

		// ===== Turret用 =====
		int shootIntervalFrames = 60; // 1秒@60fps
		float bulletSpeed = 2.8f;
		float bulletLifeTime = 3.0f;
	};

public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// Json形式の敵出現データ読み込み
	/// </summary>
	/// <param name="path"></param>
	void LoadEnemyScv(const std::string& path);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="dt"></param>
	/// <param name="playerPos"></param>
	void Update(float dt, const KamataEngine::Vector3& playerPos);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="camera"></param>
	void Draw(const KamataEngine::Camera* camera);

	/// <summary>
	/// 現在生存中の敵一覧を取得
	/// </summary>
	/// <returns></returns>
	std::vector<std::unique_ptr<CharacterBase>>& GetEnemies() { return enemies_; }

	/// <summary>
	/// 死亡した敵をリストから削除
	/// </summary>
	void RemoveDeadEnemies();

private:
	/// <summary>
	/// 出現時間に達したら敵を生成する
	/// </summary>
	/// <param name="playerPos"></param>
	void SpawnEnemiesByScv(const KamataEngine::Vector3& playerPos);

	/// <summary>
	/// 指定位置に爆発エフェクトを生成
	/// </summary>
	/// <param name="pos"></param>
	void SpawnExplosionAt(const KamataEngine::Vector3& pos);

private:
	std::vector<std::unique_ptr<CharacterBase>> enemies_;  // 生存中の敵
	std::vector<EnemySpawnData> enemySpawnList_;           // 出現待ちのデータ
	float enemySpawnTimer_ = 0.0f;                         // 経過時間

	KamataEngine::Model* explosionModel_ = nullptr;        // 爆発用モデル
	std::vector<std::unique_ptr<DamageParticle>> explosionParticles_;
};

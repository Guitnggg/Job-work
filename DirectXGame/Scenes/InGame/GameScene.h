#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>
#include <memory>
#include <algorithm>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Cameras/RailCamera/RailCamera.h"
#include "Application/Charactors/Player/Player.h"
#include "Application/Effects/SpeedLine/SpeedLine.h"
#include "Application/Effects/Smoke/Smoke.h"
#include "Application/Effects/Damage/DamageParticle.h"
#include "Application/Objects/Asteroid/Asteroid.h"

#include "UI/CountDown/CountDown.h"
#include "UI/HPBar/Graph.h"
#include "UI/Score/Score.h"

#include "Managers/EnemyManager/EnemyManager.h"
#include "Managers/BulletManager/BulletManager.h"
#include "Managers/CollisionManager/CollisionManager.h"
#include "Managers/UIManager/UIManager.h"

#include "Scenes/IScene.h"
class FinishScene;
class ClearScene;

enum class GameResult {
    Clear,  // クリアに成功
    Fail,   // プレイヤーが倒された
    None    // 判定前
};

/// <summary>
/// ゲーム本編シーン。  
/// カウントダウン → 戦闘 → 判定（クリア／失敗） → 演出 → シーン遷移  
/// までの流れを一括で制御する。  
/// 弾・敵・HP・スコア・パーティクル・カメラを統合する中枢クラス。
/// </summary>
class GameScene : public IScene {
public:
    GameScene();
    ~GameScene();

    /// <summary>
    /// 初期化処理
    /// 全てのゲーム要素を生成・初期化する
    /// プレイヤー・天球・カメラ・敵・弾・UI・演出・各モデルを生成する。
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画処理
    /// 背景→3Dオブジェクト→UIの順に描画
    /// </summary>
    void Draw() override;

public:
    bool IsEnd() const override { return isEnd_; }
    IScene* NextScene() const override;

    SceneName GetSceneName() const override { return SceneName::InGame; }

private:
    // ========== 基本 ==========
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    KamataEngine::WorldTransform* worldTransform_ = nullptr;
    KamataEngine::Camera camera_;
    KamataEngine::Model* model_ = nullptr;

    // ========== ３カウントUI ==========
    CountDown countDown_;

    // ========== レールカメラ ==========
    bool isRailCameraActive_ = true;
    RailCamera* railCamera_ = nullptr;

    // ========== 天球 ==========
    Skydome* skydome_ = nullptr;

    // ========== プレイヤー ==========
    Player* player_ = nullptr;

    // ========== 弾・敵管理 ==========
    EnemyManager enemyManager_;
    BulletManager bulletManager_;

    // ========== スピード演出 ==========
    SpeedLine speedLine_;

    // ========== ダメージ演出 ==========
    std::vector<std::unique_ptr<DamageParticle>>damageParticles_;
    KamataEngine::Model* damageParticleModel_ = nullptr;

    // ========== エンジンスモーク ==========
    KamataEngine::Model* smokeModel_ = nullptr;
    std::vector<std::unique_ptr<Smoke>> engineSmokes_;
    float smokeEmitTimer_ = 0.0f;

    float emitInterval = 0.08f;  // 通常時：0.08秒ごと（1秒に12〜13個くらい）
    float lifeTime = 0.15f;      // すぐ消える
    float startScale = 0.14f;    // 小さめ
    int   burstCount = 1;        // 一度に出す個数
    float baseZSpeed = -0.8f;    // 後ろへの速度

    // ========== UI（HPバー／スコアなど） ==========
    UIManager uiManager_;
    int clearScore_ = 0;

    // ========== クリア後演出用 ==========
    bool  isClearAnimating_ = false;
    float clearAnimTimer_ = 0.0f;

    // ========== シーン制御 ==========
    bool isEnd_ = false;
    GameResult result_ = GameResult::None;
};

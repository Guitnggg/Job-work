#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>

class SpeedLine {
public:
    SpeedLine();
    ~SpeedLine();

    /// <summary>
    /// 初期化。使用するカメラと、スピードラインの本数を指定
    /// </summary>
    void Initialize(KamataEngine::Camera* camera, int lineCount = 80);

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="dt">経過時間(秒)</param>
    /// <param name="playerPos">プレイヤーのワールド座標</param>
    void Update(float dt, const KamataEngine::Vector3& playerPos);

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

private:
    struct LineParticle {
        KamataEngine::WorldTransform* worldTransform = nullptr;  // ★ ポインタで管理
        float speed = 0.0f;                                      // カメラに向かって流れる速度
        float alpha = 1.0f;                                      // 透明度
    };

private:
    /// <summary>
    /// 1本を再配置する（プレイヤー前方にスポーン）
    /// </summary>
    /// <param name="particle">対象パーティクル</param>
    /// <param name="playerPos">プレイヤー座標</param>
    /// <param name="randomDepth">true: Z をランダム、false: 最小付近に配置</param>
    void Respawn(LineParticle& particle, const KamataEngine::Vector3& basePos, bool randomDepth);

private:
    KamataEngine::Model* model_ = nullptr;
    KamataEngine::Camera* camera_ = nullptr;

    std::vector<LineParticle> lines_;
    std::mt19937 random_;

    // 画面中心付近を濃くするための最大半径
    float maxRadius_ = 25.0f;

    // プレイヤーよりどのくらい前に沸かせるか（Z 方向）
    float spawnZMin_ = 150.0f;
    float spawnZMax_ = 250.0f;

    // カメラ手前に抜けたらどのくらいで再配置するか
    float despawnOffsetZ_ = -10.0f;
};

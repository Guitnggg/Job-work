#include "SpeedLine.h"

using namespace KamataEngine;

// ===== SpeedLine.cpp 専用定数 =====
namespace {
    // 円周率
    constexpr float kTwoPi = 6.28318530717958647692f;

    // 分布
    constexpr float kInnerRadiusRate = 0.8f;   // 中央を空ける割合
    constexpr float kYFlattenRate = 0.6f;      // Y方向のつぶし

    // 見た目サイズ
    constexpr float kLineScale = 0.15f;

    // スピード範囲
    constexpr float kSpeedMin = 45.0f;
    constexpr float kSpeedMax = 85.0f;

    // 透明度
    constexpr float kAlphaBase = 0.10f;
    constexpr float kAlphaRange = 0.25f;

    // ゆらぎ
    constexpr float kJitterX = 0.02f;
    constexpr float kJitterY = 0.01f;
}

SpeedLine::SpeedLine() {}

SpeedLine::~SpeedLine() {
    // WorldTransform の破棄
    for (auto& line : lines_) {
        delete line.worldTransform;
        line.worldTransform = nullptr;
    }

    delete model_;
    model_ = nullptr;
}

void SpeedLine::Initialize(Camera* camera, int lineCount) {
    camera_ = camera;

    // 必要に応じて専用モデルの読み込みに差し替えてください
    model_ = Model::CreateFromOBJ("Asteroid", true);

    // 乱数初期化
    std::random_device rd;
    random_ = std::mt19937(rd());

    // 指定本数ぶん確保
    lines_.resize(lineCount);

    Vector3 dummyPlayerPos{ 0.0f, 0.0f, 0.0f };
    for (auto& line : lines_) {
        line.worldTransform = new WorldTransform();
        line.worldTransform->Initialize();

        Respawn(line, dummyPlayerPos, /*randomDepth=*/true);
    }
}

void SpeedLine::Respawn(LineParticle& p, const Vector3& basePos, bool randomDepth) {
    if (!p.worldTransform) {
        return;
    }

    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> distAngle(0.0f,kTwoPi);

    float u = dist01(random_);        // 0〜1
    float angle = distAngle(random_);     // 0〜2π

    // ★ さらに外側だけ：内側 80% は完全に空けるイメージ
    float innerRadius = maxRadius_ * kInnerRadiusRate;
    float outerRadius = maxRadius_;

    // u^2 で外側寄りの分布に
    float t = u * u;
    float r = innerRadius + (outerRadius - innerRadius) * t;

    float x = std::cos(angle) * r;
    float y = std::sin(angle) * r * kYFlattenRate;

    float z = spawnZMin_;
    if (randomDepth) {
        std::uniform_real_distribution<float> distZ(spawnZMin_, spawnZMax_);
        z = distZ(random_);

        float depthFactor = (spawnZMax_ - z) / (spawnZMax_ - spawnZMin_); // 0〜1 : 奥→手前
        p.alpha *= depthFactor;   // 奥ほど alpha が弱くなる
    }

    auto& wt = *p.worldTransform;

    // 基準位置（いまは RailCamera の位置を渡している）
    wt.translation_.x = basePos.x + x;
    wt.translation_.y = basePos.y + y;
    wt.translation_.z = basePos.z + z;

    // size
    wt.scale_ = { kLineScale, kLineScale, kLineScale };

    std::uniform_real_distribution<float> distSpeed(kSpeedMin,kSpeedMax);
    p.speed = distSpeed(random_);

    // 外側ほど少し明るく（でも全体は薄め）
    float edgeFactor = (r - innerRadius) / (outerRadius - innerRadius); // 0〜1
    p.alpha = kAlphaBase + edgeFactor * kAlphaRange;

    wt.rotation_ = { 0.0f, 0.0f, 0.0f };
    wt.UpdateMatrix();
}

void SpeedLine::Update(float dt, const Vector3& playerPos) {
    if (!camera_ || !model_) {
        return;
    }

    for (auto& line : lines_) {
        if (!line.worldTransform) {
            continue;
        }

        auto& wt = *line.worldTransform;

        // Z方向にプレイヤーへ近づける（カメラは後ろにいる想定なので -Z）
        wt.translation_.z -= line.speed * dt;

        // ほんの少しだけゆらぎを入れるとブラー感UP
        wt.translation_.x *= (1.0f + kJitterX * dt);
        wt.translation_.y *= (1.0f + kJitterY * dt);

        // プレイヤー位置より手前まで来たら再配置
        if (wt.translation_.z < playerPos.z + despawnOffsetZ_) {
            Respawn(line, playerPos, /*randomDepth=*/false);
        }

        wt.UpdateMatrix();
    }
}

void SpeedLine::Draw() {
    if (!camera_ || !model_) {
        return;
    }

    for (auto& line : lines_) {
        if (!line.worldTransform) {
            continue;
        }

        auto& wt = *line.worldTransform;

        // ★ エンジン側に色/アルファ設定APIがあるならここで line.alpha を使うと
        //    「薄くて残像っぽいスピードライン」が表現できます。
        //    例:
        //    model_->SetColor({1.0f, 1.0f, 1.0f, line.alpha});

        model_->Draw(wt, *camera_);
    }
}

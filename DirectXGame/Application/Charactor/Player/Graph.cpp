#include "Graph.h"
#include <algorithm>

using namespace KamataEngine;

Graph::Graph() {}
Graph::~Graph() = default;

void Graph::Initialize(uint32_t whiteTexHandle) {
    whiteTexHandle_ = whiteTexHandle;

    // Sprite は private コンストラクタなので Create() で生成
    backBar_.reset(Sprite::Create(
        /*textureHandle*/ whiteTexHandle_,
        /*position*/      { 0.0f, 0.0f },
        /*color*/         { 1, 1, 1, 1 },
        /*anchorpoint*/   { 0.0f, 0.5f }, // 左中央基準
        /*flipX*/         false,
        /*flipY*/         false
    ));
    frontBar_.reset(Sprite::Create(
        whiteTexHandle_,
        { 0.0f, 0.0f },
        { 1, 1, 1, 1 },
        { 0.0f, 0.5f }, // 左中央基準
        false,
        false
    ));

    // 念のためハンドルの再設定が可能なら適用
    backBar_->SetTextureHandle(whiteTexHandle_);
    frontBar_->SetTextureHandle(whiteTexHandle_);

    // 初期配置を反映
    ApplyRect_(backBar_.get(), origin_, size_, backColor_);
    ApplyRect_(frontBar_.get(), origin_, size_, frontColor_);

    targetValue_  = 1.0f;
    displayValue_ = 1.0f;
}

void Graph::SetLayout(const Vector2& origin, const Vector2& size) {
    origin_ = origin;
    size_   = size;

    // 即時反映（前面バーは現在の displayValue_ に合わせる）
    ApplyRect_(backBar_.get(), origin_, size_, backColor_);
    ApplyRect_(frontBar_.get(), origin_, { size_.x * displayValue_, size_.y }, frontColor_);
}

void Graph::SetColors(const Vector4& backRGBA, const Vector4& frontRGBA) {
    backColor_  = backRGBA;
    frontColor_ = frontRGBA;

    // 即時反映
    ApplyRect_(backBar_.get(), origin_, size_, backColor_);
    ApplyRect_(frontBar_.get(), origin_, { size_.x * displayValue_, size_.y }, frontColor_);
}

void Graph::SetValue(float rate) {
    // Windows.h の min/max マクロ影響を避けるため (std::clamp) を丸括弧で呼ぶのも手ですが、
    // ここでは 0..1 を手動でクランプしておきます。
    if (rate < 0.0f) rate = 0.0f;
    if (rate > 1.0f) rate = 1.0f;
    targetValue_ = rate;
}

void Graph::Update() {
    // スムージング（目標へ指数的に追従）
    displayValue_ += (targetValue_ - displayValue_) * followRate_;

    // 横幅だけ可変（左端基準で伸縮）
    ApplyRect_(frontBar_.get(), origin_, { size_.x * displayValue_, size_.y }, frontColor_);
}

void Graph::Draw() {
    if (backBar_)  backBar_->Draw();
    if (frontBar_) frontBar_->Draw();
}

void Graph::ApplyRect_(Sprite* s, const Vector2& pos, const Vector2& size, const Vector4& rgba) {
    if (!s) return;
    s->SetPosition(pos);
    s->SetSize(size);
    s->SetColor(rgba);
    // アンカーは Initialize 時に左中央 {0,0.5} を指定済み。
    // 別方向にしたい場合は SetAnchorPoint({0.5f, 1.0f}) 等に変更してください。
}

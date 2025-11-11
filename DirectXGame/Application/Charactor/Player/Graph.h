#pragma once
#include <2d/Sprite.h>
#include <memory>

class Graph {
public:
    Graph();
    ~Graph();

    // コピー禁止（スマートポインタ保護）
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    /// <summary>
    /// 初期化。描画に使う 1x1 白テクスチャのハンドルを指定してください。
    /// </summary>
    /// <param name="whiteTexHandle">1x1白テクスチャのハンドル</param>
    void Initialize(uint32_t whiteTexHandle);

    /// <summary>
    /// 表示位置とサイズ（左上座標, 幅×高さ）を設定します。
    /// </summary>
    void SetLayout(const KamataEngine::Vector2& origin, const KamataEngine::Vector2& size);

    /// <summary>
    /// 背景色と手前バー色を設定します（RGBA）。
    /// </summary>
    void SetColors(const KamataEngine::Vector4& backRGBA, const KamataEngine::Vector4& frontRGBA);

    /// <summary>
    /// 0.0～1.0 の目標値を設定します（HP/MaxHP など）。
    /// 実描画は内部のスムージングで滑らかに追従します。
    /// </summary>
    void SetValue(float rate);

    float GetValue() const { return targetValue_; }

    /// <summary>
    /// 値追従などの内部更新。
    /// </summary>
    void Update();

    /// <summary>
    /// 描画（背景→前面バーの順）。
    /// </summary>
    void Draw();

private:
    // 矩形スプライトに共通設定を適用
    void ApplyRect_(KamataEngine::Sprite* s, const KamataEngine::Vector2& pos,
        const KamataEngine::Vector2& size, const KamataEngine::Vector4& rgba);

private:
    // 使用する 1x1 白テクスチャ
    uint32_t whiteTexHandle_ = 0;

    // スプライト
    std::unique_ptr<KamataEngine::Sprite> backBar_;
    std::unique_ptr<KamataEngine::Sprite> frontBar_;

    // レイアウト
    KamataEngine::Vector2 origin_{ 100.0f, 50.0f };
    KamataEngine::Vector2 size_{ 200.0f, 40.0f };

    // 色
    KamataEngine::Vector4 backColor_{ 0.15f, 0.15f, 0.15f, 0.80f };
    KamataEngine::Vector4 frontColor_{ 0.20f, 0.85f, 0.35f, 1.00f };

    // 値（目標値と表示値を分けてスムージング）
    float targetValue_ = 1.0f;
    float displayValue_ = 1.0f;
    float followRate_ = 0.20f; // 追従率（大きいほど素早く追従）
};

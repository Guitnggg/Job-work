#pragma once

#include "math/Vector3.h"

/// <summary>
/// カメラシェイク制御クラス
/// カメラに対して揺れ演出（シェイク）を付加する
/// </summary>
class CameraShake {
public:
    /// <summary>
    /// カメラに揺れを追加する
    /// </summary>
    /// <param name="direction">揺れの方向</param>
    /// <param name="power">揺れの強さ</param>
    void AddShake(const KamataEngine::Vector3& direction, float power);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 揺れ状態をリセットする
    /// </summary>
    void Reset();

    /// <summary>
    /// 現在のカメラオフセットを取得する
    /// </summary>
    /// <returns></returns>
    const KamataEngine::Vector3& GetOffset() const { return offset_; }

private:
    // ===== 定数 =====
    static constexpr float kDecayRate = 0.85f;            // 減衰率
    static constexpr float kMinDirectionLength = 0.0001f; // 有効な方向とみなす最小長

    static constexpr KamataEngine::Vector3 kZeroVector{ 0.0f, 0.0f, 0.0f };

private:
    // ===== メンバ変数 =====
    KamataEngine::Vector3 offset_{};   // カメラオフセット
    KamataEngine::Vector3 velocity_{}; // 揺れ用速度ベクトル
};
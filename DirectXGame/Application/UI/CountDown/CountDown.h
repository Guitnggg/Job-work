#pragma once

#include <algorithm>
#include <memory>

#include "2d/Sprite.h"
#include "3d/Camera.h"
#include "3d/Model.h"
#include "3d/WorldTransform.h"
#include "audio/Audio.h"
#include "base/TextureManager.h"
#include "math/Vector2.h"

/// <summary>
/// ゲーム開始時のカウントダウン演出
/// </summary>
class CountDown {
public:
	enum class Phase {
		Inactive,   // 待機（何も表示しない）
		ReadyDelay, // 開始時のウェイト
		Count3,     // ３
		Count2,     // ２
		Count1,     // １
		Go,         // GO
		Done        // 完了
	};

	CountDown() = default;
	~CountDown() = default;

	/// <summary>
	/// 画像パスからテクスチャを読み込み、カウント用スプライトを生成して初期化する
	/// </summary>
	/// <param name="tex3">「3」に使用するテクスチャのパス</param>
	/// <param name="tex2">「2」に使用するテクスチャのパス</param>
	/// <param name="tex1">「1」に使用するテクスチャのパス</param>
	/// <param name="texGo">「GO」に使用するテクスチャのパス</param>
	/// <param name="centerPos">表示の中心座標</param>
	/// <param name="baseSizeCount">3/2/1 表示時の基準サイズ</param>
	/// <param name="baseGoSize">GO 表示時の基準サイズ</param>
	void InitializeFromPaths(
	    const char* tex3, const char* tex2, const char* tex1, const char* texGo, const KamataEngine::Vector2& centerPos = {640.0f, 360.0f},
	    const KamataEngine::Vector2& baseSizeCount = {256.0f, 256.0f}, const KamataEngine::Vector2& baseGoSize = {320.0f, 160.0f});

	/// <summary>
	/// テクスチャハンドルから直接スプライトを生成し初期化する
	/// 事前に別処理で読み込んだテクスチャを使い回したい場合に利用する
	/// </summary>
	/// <param name="tex3">「3」に使用するテクスチャハンドル</param>
	/// <param name="tex2">「2」に使用するテクスチャハンドル</param>
	/// <param name="tex1">「1」に使用するテクスチャハンドル</param>
	/// <param name="texGo">「GO」に使用するテクスチャハンドル</param>
	/// <param name="centerPos">表示の中心座標</param>
	/// <param name="baseSizeCount">3/2/1 表示時の基準サイズ</param>
	/// <param name="baseSizeGo">GO 表示時の基準サイズ</param>
	void InitializeFromHandles(
	    uint32_t tex3, uint32_t tex2, uint32_t tex1, uint32_t texGo, const KamataEngine::Vector2& centerPos = {640.0f, 360.0f}, const KamataEngine::Vector2& baseSizeCount = {256.0f, 256.0f},
	    const KamataEngine::Vector2& baseSizeGo = {320.0f, 160.0f});

	/// <summary>
	/// 各フェーズの表示時間を設定する
	/// ReadyDelay → 3 → 2 → 1 → GO の順
	/// </summary>
	/// <param name="readyDelay">カウント開始前の待機時間</param>
	/// <param name="countUnit">3/2/1 各数字の表示時間</param>
	/// <param name="goDuration">GO 表示時間</param>
	void SetTimings(float readyDelay, float countUnit, float goDuration);

	/// <summary>
	/// 拡大アニメーションの開始スケールと終了スケールを設定する
	/// </summary>
	/// <param name="startScale">表示開始時のスケール</param>
	/// <param name="endScale">表示終了時のスケール</param>
	void SetScaleRange(float startScale, float endScale);

	/// <summary>
	/// BackEase 用の「跳ね返り量」を設定する
	/// 数値が大きいほど表示時の弾みが強くなる
	/// </summary>
	/// <param name="s">BackEase のオーバーシュート量</param>
	void SetBackOvershoot(float s);

	/// <summary>
	/// カウントダウン時に再生する効果音を設定する。
	/// </summary>
	/// <param name="seBeep">3/2/1 表示開始時に鳴らすビープ音</param>
	/// <param name="seGo">GO 表示開始時に鳴らす効果音</param>
	void SetAudio(uint32_t seBeep, uint32_t seGo);

public:
	/// <summary>
	/// カウントダウン開始
	/// </summary>
	void Start();

	/// <summary>
	/// カウントダウン状態のリセット
	/// </summary>
	void Reset();

	/// <summary>
	/// カウントダウンの更新
	/// </summary>
	/// <param name="dt">前フレーム空の経過時間</param>
	void Update(float dt);

	/// <summary>
	/// カウントダウンの描画
	/// </summary>
	void Draw();

public:
	/// <summary>
	/// カウントダウンが有効かどうか返す
	/// </summary>
	bool IsActive() const { return phase_ != Phase::Inactive; }

	/// <summary>
	/// カウントダウンが完了かどうか返す
	/// </summary>
	bool IsDone() const { return phase_ == Phase::Done; }

	/// <summary>
	/// 入力がロックされているかどうか返す
	/// </summary>
	bool IsInputLocked() const { return inputLocked_; }

	/// <summary>
	/// 現在のフェーズを取得する
	/// </summary>
	Phase GetPhase() const { return phase_; }

private:
	/// <summary>
	/// BackEaseのイージング関数
	/// </summary>
	float EaseOutBack(float t, float s) const;

	/// <summary>
	/// 現在のフェーズに対する表示時間を返す
	/// </summary>
	float CurrentPhaseDuration() const;

	/// <summary>
	/// 現在のフェーズで表示すべきスプライトを返す
	/// 該当しないフェーズの場合は nullptr を返す
	/// </summary>
	KamataEngine::Sprite* CurrentPhaseSprite() const;

	/// <summary>
	/// 現在のフェーズにおけるアルファ値を返す
	/// </summary>
	float CurrentPhaseAlpha(float t01) const;

	/// <summary>
	/// 現在のフェーズにおけるスケール値を返す
	/// </summary>
	float CurrentPhaseScale(float t01) const;

	/// <summary>
	/// タイマーのフェーズが持ち時間を超えたら次のフェーズに進める
	/// </summary>
	void AdvancePhase();

private:
	// スプライト
	std::unique_ptr<KamataEngine::Sprite> count3Sprite_ = nullptr;
	std::unique_ptr<KamataEngine::Sprite> count2Sprite_ = nullptr;
	std::unique_ptr<KamataEngine::Sprite> count1Sprite_ = nullptr;
	std::unique_ptr<KamataEngine::Sprite> goSprite_ = nullptr;

	// 表示パラメータ
	KamataEngine::Vector2 center_ = {640.0f, 360.0f};
	KamataEngine::Vector2 baseSizeCount_ = {256.0f, 256.0f};
	KamataEngine::Vector2 baseSizeGo_ = {320.0f, 160.0f};
	float scaleStart_ = 1.20f;
	float scaleEnd_ = 1.00f;
	float backS_ = 1.70f;

	// タイミング
	float readyDelay_ = 0.10f;
	float countUnit_ = 0.50f;
	float goDuration_ = 0.40f;

	// SE
	uint32_t seBeep_ = 0;
	uint32_t seGo_ = 0;
	bool goPlayed_ = false;

	// 進行
	Phase phase_ = Phase::Inactive;
	float phaseTimer_ = 0.0f;
	bool inputLocked_ = true;
};

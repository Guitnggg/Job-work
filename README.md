# Job-work

3Dレールシューティングゲームの就職活動提出用リポジトリです。
プレイヤーはレールカメラに沿って進行しながら敵を撃破し、ステージごとの目標スコア到達を目指します。

[![DebugBuild](https://github.com/Guitnggg/Job-work/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/Guitnggg/Job-work/actions/workflows/DebugBuild.yml)
[![ReleaseBuild](https://github.com/Guitnggg/Job-work/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/Guitnggg/Job-work/actions/workflows/ReleaseBuild.yml)

## 概要

- ジャンル: 3Dレールシューティング
- 開発環境: Visual Studio 2022 / C++ / DirectX
- ソリューション: `DirectXGame/DirectXGame.sln`

## 操作方法

- WASD: 移動
- マウス: 照準操作
- 左クリック: 通常弾
- 右クリック: ホーミングミサイル
- Esc: ポーズ

## 主な実装機能

- レールカメラによる自動進行
- 敵の出現管理
- 通常弾・ホーミングミサイル管理
- ロックオン UI
- HP / スコア UI
- GPU パーティクルによる煙・演出表現
- タイトル、イントロ、ゲーム本編、クリア、失敗などのシーン遷移
- GitHub Actions による Debug / Release ビルド確認

## 提出版でのボス戦の扱い

ボス戦関連のクラスや管理処理は実装途中ですが、現時点では完成度と安定性を優先し、提出版ではボス戦へ遷移しない方針です。
そのため、`DirectXGame/Scenes/InGame/GameScene.cpp` の `kEnableBossStage` を `false` にして、通常ステージのクリアフローを使用しています。

## ビルド方法

Visual Studio 2022 で `DirectXGame/DirectXGame.sln` を開き、`x64` の `Debug` または `Release` 構成でビルドしてください。

GitHub Actions でも `master` ブランチへの push 時に Debug / Release ビルドを確認する構成にしています。
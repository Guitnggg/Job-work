#include <Windows.h>
#include <memory>
#include <KamataEngine.h>

#include "Scenes/SceneManager.h"
#include "Scenes/Title/TitleScene.h"

// Windowsアプリのエントリーポイント
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    // KamataEngineの初期化、ゲームループ開始
    KamataEngine::Initialize(L"LE4C_06_タカキ_ケンゴ_AstroWing");
    {
        // ゲームプレイ中はOSカーソルを非表示にする
        //while (ShowCursor(FALSE) >= 0) {}

        // 描画開始/終了を行うDirectX共通クラス
        KamataEngine::DirectXCommon* dxCommon = KamataEngine::DirectXCommon::GetInstance();

        // 最初に表示するsceneをタイトルに設定
        SceneManager sceneManager;
        sceneManager.SetInitialScene(std::make_unique<TitleScene>());

        // Mainループ　更新→描画を毎フレーム実行
        while (true) {

            // OSイベント処理とエンジン側更新。trueで終了要求
            if (KamataEngine::Update()) {
                break;
            }

            // ゲームロジック更新
            sceneManager.Update();

            // 描画処理開始
            dxCommon->PreDraw();
            sceneManager.Draw();

            // 描画処理終了
            dxCommon->PostDraw();
        }
    }

    // KamataEngineの終了 
    KamataEngine::Finalize();

    // アプリ終了時にOSカーソルを元に戻す
    //while (ShowCursor(TRUE) < 0) {}

    return 0;
}
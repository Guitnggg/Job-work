#include <Windows.h>
#include <KamataEngine.h>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    // KamataEngineの初期化
    KamataEngine::Initialize(L"LE3C_14_タカキ_ケンゴ_タイトル");

    // DirectXCommonのインスタンスの取得
    DirectXCommon* dxCommon = DirectXCommon::GetInstance();

    //===============
    // Mainループ
    //===============
    while (true) {

        // エンジンの更新
        if (KamataEngine::Update()) {
            break;
        }

        //=============
        // 更新処理
        //=============

        

        //=============
        /// 描画処理 
        //=============

        // 描画処理開始       
        dxCommon->PreDraw();



        //描画処理終了       
        dxCommon->PostDraw();


    }
       

    // KamataEngineの終了 
    KamataEngine::Finalize();

    return 0;
}

#include <Novice.h>
#include <math.h>

const char kWindowTitle[] = "LD2B_04_ナガサワタカユキ_射影とビューポート";

//==================================================
// 構造体定義
//==================================================

// 4x4 行列構造体
struct Matrix4x4 {
	float m[4][4];
};

//==================================================
// 射影・ビューポート変換行列の作成関数群
//==================================================

/// <summary>
/// 正射影行列の作成
/// </summary>
/// <param name="left">左端</param>
/// <param name="top">上端</param>
/// <param name="right">右端</param>
/// <param name="bottom">下端</param>
/// <param name="nearClip">近クリップ面</param>
/// <param name="farClip">遠クリップ面</param>
/// <returns>正射影行列</returns>
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result{};
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;
	return result;
}

/// <summary>
/// 透視投影行列の作成
/// </summary>
/// <param name="fovY">Y軸方向の視野角（ラジアン）</param>
/// <param name="aspectRatio">アスペクト比</param>
/// <param name="nearClip">近クリップ面</param>
/// <param name="farClip">遠クリップ面</param>
/// <returns>透視投影行列</returns>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result{};
	float f = 1.0f / tanf(fovY / 2.0f);
	result.m[0][0] = f / aspectRatio;
	result.m[1][1] = f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return result;
}

/// <summary>
/// ビューポート変換行列の作成
/// </summary>
/// <param name="left">ビューポートの左端</param>
/// <param name="top">ビューポートの上端</param>
/// <param name="width">ビューポートの幅</param>
/// <param name="height">ビューポートの高さ</param>
/// <param name="minDepth">最小深度</param>
/// <param name="maxDepth">最大深度</param>
/// <returns>ビューポート変換行列</returns>
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result{};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

//==================================================
// 行列の表示関数
//==================================================

/// <summary>
/// 行列を画面に表示する
/// </summary>
/// <param name="x">表示開始位置X</param>
/// <param name="y">表示開始位置Y</param>
/// <param name="matrix">表示する行列</param>
/// <param name="label">ラベル文字列</param>
void MatrixScreenPrint(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int i = 0; i < 4; ++i) {
		Novice::ScreenPrintf(x, y + 20 * (i + 1),
			"%6.2f %6.2f %6.2f %6.2f",
			matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
	}
}

//==================================================
// メイン関数
//==================================================

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	//===============================
	// 射影・ビューポート行列の作成
	//===============================
	Matrix4x4 orthographicMatrix = MakeOrthographicMatrix(-160.0f, 160.0f, 200.0f, 300.0f, 0.0f, 1000.0f);
	Matrix4x4 perspectiveFovMatrix = MakePerspectiveFovMatrix(0.63f, 1.33f, 0.1f, 1000.0f);
	Matrix4x4 viewportMatrix = MakeViewportMatrix(100.0f, 200.0f, 600.0f, 300.0f, 0.0f, 1.0f);

	//===============================
	// メインループ
	//===============================
	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// 行列の描画（20ピクセルごとに段差）
		int kRowHeight = 20;
		MatrixScreenPrint(0, 0, orthographicMatrix, "orthographicMatrix");
		MatrixScreenPrint(0, kRowHeight * 5, perspectiveFovMatrix, "perspectiveFovMatrix");
		MatrixScreenPrint(0, kRowHeight * 10, viewportMatrix, "viewportMatrix");

		Novice::EndFrame();

		// ESCキーで終了
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
	}

	Novice::Finalize();
	return 0;
}

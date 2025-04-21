#include <Novice.h>
#include <stdio.h>
#include <math.h>

const char kWindowTitle[] = "LD2B_04_ナガサワタカユキ_回転行列";

//==================================================
// 構造体定義
//==================================================

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[4][4];
};

//==================================================
// 行列生成関数群
//==================================================

// X軸回転行列の作成
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 matrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosf(radian), sinf(radian), 0.0f,
		0.0f, -sinf(radian), cosf(radian), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return matrix;
}

// Y軸回転行列の作成
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 matrix = {
		cosf(radian), 0.0f, -sinf(radian), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sinf(radian), 0.0f, cosf(radian), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return matrix;
}

// Z軸回転行列の作成
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 matrix = {
		cosf(radian), sinf(radian), 0.0f, 0.0f,
		-sinf(radian), cosf(radian), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return matrix;
}

// 平行移動行列の作成
Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 matrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		translate.x, translate.y, translate.z, 1.0f
	};
	return matrix;
}

// 拡大縮小行列の作成
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
	Matrix4x4 matrix = {
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return matrix;
}

//==================================================
// 行列演算
//==================================================

// 4x4行列同士の乗算
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

//==================================================
// アフィン変換行列の合成
//==================================================

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateX = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateY = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZ = MakeRotateZMatrix(rotate.z);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	// 合成順：S → Rx → Ry → Rz → T
	Matrix4x4 result = scaleMatrix;
	result = Multiply(result, rotateX);
	result = Multiply(result, rotateY);
	result = Multiply(result, rotateZ);
	result = Multiply(result, translateMatrix);

	return result;
}

//==================================================
// 行列の画面表示
//==================================================

void MatrixScreenPrint(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int i = 0; i < 4; ++i) {
		Novice::ScreenPrintf(x, y + (i + 1) * 20,
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

	// 拡大・回転・移動ベクトルの初期化
	Vector3 scale = { 1.2f, 0.79f, -2.1f };
	Vector3 rotate = { 0.4f, 1.43f, -0.8f };
	Vector3 translate = { 2.7f, -4.15f, 1.57f };

	// アフィン変換行列の作成
	Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// 描画処理
		MatrixScreenPrint(30, 40, worldMatrix, "worldMatrix");

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}

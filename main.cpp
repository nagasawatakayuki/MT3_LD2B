#include <Novice.h>
#include <math.h>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_クロス積と3D三角形描画";

//====================================
// ウィンドウサイズ
//====================================
const int kWindowWidth = 1280;
const int kWindowHeight = 720;

//====================================
// 構造体定義
//====================================
struct Vector3 {
    float x, y, z;
};

struct Matrix4x4 {
    float m[4][4];
};

//====================================
// 演算・行列ユーティリティ
//====================================
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
    return {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

Vector3 operator+(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Matrix4x4 MakeIdentityMatrix() {
    return {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
}

Matrix4x4 MakeRotateYMatrix(float rad) {
    return {
        cosf(rad), 0, -sinf(rad), 0,
        0, 1, 0, 0,
        sinf(rad), 0, cosf(rad), 0,
        0, 0, 0, 1
    };
}

Matrix4x4 MakeTranslateMatrix(Vector3 t) {
    Matrix4x4 m = MakeIdentityMatrix();
    m.m[3][0] = t.x;
    m.m[3][1] = t.y;
    m.m[3][2] = t.z;
    return m;
}

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
    Matrix4x4 result{};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                result.m[i][j] += m1.m[i][k] * m2.m[k][j];
    return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ) {
    Matrix4x4 m{};
    float f = 1.0f / tanf(fovY / 2);
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = farZ / (farZ - nearZ);
    m.m[2][3] = 1.0f;
    m.m[3][2] = -nearZ * farZ / (farZ - nearZ);
    return m;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
    Matrix4x4 m{};
    m.m[0][0] = width / 2;
    m.m[1][1] = -height / 2;
    m.m[2][2] = maxDepth - minDepth;
    m.m[3][0] = left + width / 2;
    m.m[3][1] = top + height / 2;
    m.m[3][2] = minDepth;
    m.m[3][3] = 1;
    return m;
}

Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
    float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
    float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
    float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
    float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];

    return { x / w, y / w, z / w };
}

void VectorScreenPrintf(int x, int y, const Vector3& v, const char* label) {
    Novice::ScreenPrintf(x, y, "%s: %.2f %.2f %.2f", label, v.x, v.y, v.z);
}

//====================================
// メイン関数
//====================================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

    char keys[256] = {};
    char preKeys[256] = {};

    // クロス積確認用
    Vector3 v1{ 1.2f, -3.9f, 2.5f };
    Vector3 v2{ 2.8f, 0.4f, -1.3f };
    Vector3 cross = Cross(v1, v2);

    // 三角形ローカル座標
    Vector3 localVertices[3] = {
        {-0.5f, -0.5f, 0.0f},
        { 0.0f,  0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f}
    };

    Vector3 translate = { 0.0f, 0.0f, 5.0f };
    Vector3 rotate{};
    float angle = 0.0f;

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // 入力に応じた移動ベクトル（毎フレームリセット）
        Vector3 deltaTranslate{};
        if (keys[DIK_W]) deltaTranslate.z -= 0.1f;
        if (keys[DIK_S]) deltaTranslate.z += 0.1f;
        if (keys[DIK_A]) deltaTranslate.x -= 0.1f;
        if (keys[DIK_D]) deltaTranslate.x += 0.1f;

        translate = translate + deltaTranslate;

        // Y軸自動回転
        angle += 0.02f;
        rotate.y = angle;

        // 各行列
        Matrix4x4 worldMatrix = Multiply(MakeRotateYMatrix(rotate.y), MakeTranslateMatrix(translate));
        Matrix4x4 viewMatrix = MakeTranslateMatrix({ 0, 0, -10 });  // カメラ後方に固定
        Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
        Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

        // 最終行列合成
        Matrix4x4 wvp = Multiply(Multiply(worldMatrix, viewMatrix), projectionMatrix);

        // 頂点変換 → スクリーン座標へ
        Vector3 screenVertices[3];
        for (int i = 0; i < 3; i++) {
            Vector3 ndc = Transform(localVertices[i], wvp);
            screenVertices[i] = Transform(ndc, viewportMatrix);
        }

        // 描画
        Novice::DrawTriangle(
            int(screenVertices[0].x), int(screenVertices[0].y),
            int(screenVertices[1].x), int(screenVertices[1].y),
            int(screenVertices[2].x), int(screenVertices[2].y),
            RED, kFillModeSolid
        );

        // クロス積確認
        VectorScreenPrintf(0, 0, cross, "Cross");

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

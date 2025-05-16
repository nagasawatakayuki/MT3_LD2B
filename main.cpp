#include <Novice.h>
#include <imgui.h>
#include <string>
#include <math.h>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_演算子オーバーロード";

//------------------------------------------------
// Vector3 構造体と演算子オーバーロード
//------------------------------------------------
struct Vector3 {
    float x, y, z;

    // 複合代入演算子（メンバ関数）
    Vector3& operator+=(const Vector3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    Vector3& operator-=(const Vector3& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
    Vector3& operator*=(float s) {
        x *= s; y *= s; z *= s;
        return *this;
    }
    Vector3& operator/=(float s) {
        x /= s; y /= s; z /= s;
        return *this;
    }
};

// 二項演算子（グローバル関数）
Vector3 operator+(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
Vector3 operator-(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vector3 operator*(const Vector3& v, float s) {
    return { v.x * s, v.y * s, v.z * s };
}
Vector3 operator*(float s, const Vector3& v) {
    return v * s;
}
Vector3 operator/(const Vector3& v, float s) {
    return { v.x / s, v.y / s, v.z / s };
}
Vector3 operator-(const Vector3& v) {
    return { -v.x, -v.y, -v.z };
}
Vector3 operator+(const Vector3& v) {
    return v;
}

//------------------------------------------------
// Matrix4x4構造体（最低限）と演算子（加算のみ）
//------------------------------------------------
struct Matrix4x4 {
    float m[4][4];
};

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
    Matrix4x4 result{};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                result.m[i][j] += m1.m[i][k] * m2.m[k][j];
    return result;
}

//------------------------------------------------
// メイン関数
//------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256]{}, preKeys[256]{};

    Vector3 a{ 0.2f, 1.0f, 0.0f };
    Vector3 b{ 2.4f, 3.1f, 1.2f };
    Vector3 c = a + b;
    Vector3 d = a - b;
    Vector3 e = a * 2.4f;

    Vector3 rotate{ 0.4f, 1.43f, -0.8f };

    Matrix4x4 rotateXMatrix = {
        1, 0, 0, 0,
        0, cosf(rotate.x), sinf(rotate.x), 0,
        0, -sinf(rotate.x), cosf(rotate.x), 0,
        0, 0, 0, 1
    };
    Matrix4x4 rotateYMatrix = {
        cosf(rotate.y), 0, -sinf(rotate.y), 0,
        0, 1, 0, 0,
        sinf(rotate.y), 0, cosf(rotate.y), 0,
        0, 0, 0, 1
    };
    Matrix4x4 rotateZMatrix = {
        cosf(rotate.z), sinf(rotate.z), 0, 0,
        -sinf(rotate.z), cosf(rotate.z), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    Matrix4x4 rotateMatrix = Multiply(rotateZMatrix, Multiply(rotateYMatrix, rotateXMatrix));

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ImGui::Begin("Window");
        ImGui::Text("c:%f, %f, %f", c.x, c.y, c.z);
        ImGui::Text("d:%f, %f, %f", d.x, d.y, d.z);
        ImGui::Text("e:%f, %f, %f", e.x, e.y, e.z);

        ImGui::Text("matrix:\n"
            "%f, %f, %f, %f\n"
            "%f, %f, %f, %f\n"
            "%f, %f, %f, %f\n"
            "%f, %f, %f, %f",
            rotateMatrix.m[0][0], rotateMatrix.m[0][1], rotateMatrix.m[0][2], rotateMatrix.m[0][3],
            rotateMatrix.m[1][0], rotateMatrix.m[1][1], rotateMatrix.m[1][2], rotateMatrix.m[1][3],
            rotateMatrix.m[2][0], rotateMatrix.m[2][1], rotateMatrix.m[2][2], rotateMatrix.m[2][3],
            rotateMatrix.m[3][0], rotateMatrix.m[3][1], rotateMatrix.m[3][2], rotateMatrix.m[3][3]);
        ImGui::End();

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

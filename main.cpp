#define NOMINMAX
#include <Novice.h>
#include <imgui.h>
#include <math.h>
#include <algorithm>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_ベジエ曲線";

//------------------------------------------------
// 構造体定義
//------------------------------------------------
struct Vector3 {
    float x, y, z;
};

struct Matrix4x4 {
    float m[4][4];
};

//------------------------------------------------
// ベクトル関連関数
//------------------------------------------------
Vector3 Add(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}
Vector3 Subtract(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}
Vector3 Multiply(float s, const Vector3& v) {
    return { s * v.x, s * v.y, s * v.z };
}
float Dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
float Length(const Vector3& v) {
    return sqrtf(Dot(v, v));
}

//------------------------------------------------
// 線形補間
//------------------------------------------------
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
    return Add(Multiply(1.0f - t, v1), Multiply(t, v2));
}

//------------------------------------------------
// 行列関連関数
//------------------------------------------------
Matrix4x4 MakeIdentityMatrix() {
    return { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
}
Matrix4x4 MakeTranslateMatrix(Vector3 t) {
    Matrix4x4 m = MakeIdentityMatrix();
    m.m[3][0] = t.x; m.m[3][1] = t.y; m.m[3][2] = t.z;
    return m;
}
Matrix4x4 MakeRotateXMatrix(float rad) {
    return { 1,0,0,0, 0,cosf(rad),sinf(rad),0, 0,-sinf(rad),cosf(rad),0, 0,0,0,1 };
}
Matrix4x4 MakeRotateYMatrix(float rad) {
    return { cosf(rad),0,-sinf(rad),0, 0,1,0,0, sinf(rad),0,cosf(rad),0, 0,0,0,1 };
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
    m.m[0][0] = width / 2; m.m[1][1] = height / 2;
    m.m[2][2] = maxDepth - minDepth;
    m.m[3][0] = left + width / 2; m.m[3][1] = top + height / 2; m.m[3][2] = minDepth;
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

//------------------------------------------------
// ベジエ曲線描画関数
//------------------------------------------------
void DrawBezier(const Vector3& p0, const Vector3& p1, const Vector3& p2,
    const Matrix4x4& vp, const Matrix4x4& viewport, unsigned int color) {
    const int kSubdiv = 30;
    for (int i = 0; i < kSubdiv; ++i) {
        float t0 = (float)i / kSubdiv;
        float t1 = (float)(i + 1) / kSubdiv;

        Vector3 a1 = Lerp(p0, p1, t0);
        Vector3 b1 = Lerp(p1, p2, t0);
        Vector3 pA = Lerp(a1, b1, t0);

        Vector3 a2 = Lerp(p0, p1, t1);
        Vector3 b2 = Lerp(p1, p2, t1);
        Vector3 pB = Lerp(a2, b2, t1);

        Vector3 spA = Transform(Transform(pA, vp), viewport);
        Vector3 spB = Transform(Transform(pB, vp), viewport);

        Novice::DrawLine((int)spA.x, (int)spA.y, (int)spB.x, (int)spB.y, color);
    }
}

//------------------------------------------------
// グリッド描画
//------------------------------------------------
void DrawGrid(const Matrix4x4& vp, const Matrix4x4& viewport) {
    const float size = 2.0f;
    const int div = 10;
    const float y = -2.0f;
    for (int i = 0; i <= div; ++i) {
        float p = -size + (2 * size) * i / div;
        Vector3 s1 = Transform(Transform({ p, y, -size }, vp), viewport);
        Vector3 e1 = Transform(Transform({ p, y, size }, vp), viewport);
        Vector3 s2 = Transform(Transform({ -size, y, p }, vp), viewport);
        Vector3 e2 = Transform(Transform({ size, y, p }, vp), viewport);
        unsigned int color = fabsf(p) < 0.001f ? 0x000000FF : 0xAAAAAAFF;
        Novice::DrawLine((int)s1.x, (int)s1.y, (int)e1.x, (int)e1.y, color);
        Novice::DrawLine((int)s2.x, (int)s2.y, (int)e2.x, (int)e2.y, color);
    }
}

//------------------------------------------------
// メイン関数
//------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256]{}, preKeys[256]{};

    Vector3 controlPoints[3] = {
        {-0.8f, -1.6f, 1.0f},
        {1.3f, -1.5f, -0.3f},
        {0.94f, -1.2f, 2.3f},
    };

    Vector3 cameraTranslate = { 0.0f, 1.5f, -6.0f };
    Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };
    int mouseX, mouseY, prevMouseX = 0, prevMouseY = 0;

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);
        Novice::GetMousePosition(&mouseX, &mouseY);

        if (Novice::IsPressMouse(2)) {
            cameraRotate.y += (mouseX - prevMouseX) * 0.01f;
            cameraRotate.x += (mouseY - prevMouseY) * 0.01f;
        }
        prevMouseX = mouseX;
        prevMouseY = mouseY;

        ImGui::Begin("Bezier Control Points");
        ImGui::DragFloat3("P0", &controlPoints[0].x, 0.01f);
        ImGui::DragFloat3("P1", &controlPoints[1].x, 0.01f);
        ImGui::DragFloat3("P2", &controlPoints[2].x, 0.01f);
        ImGui::End();

        Matrix4x4 view = Multiply(MakeRotateXMatrix(cameraRotate.x), Multiply(MakeRotateYMatrix(cameraRotate.y), MakeTranslateMatrix(cameraTranslate)));
        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

        DrawGrid(vp, viewport);
        DrawBezier(controlPoints[0], controlPoints[1], controlPoints[2], vp, viewport, 0xFFFFFFFF);

        // Control points に球体を描画（黒）
        for (int i = 0; i < 3; ++i) {
            Vector3 screen = Transform(Transform(controlPoints[i], vp), viewport);
            Novice::DrawEllipse((int)screen.x, (int)screen.y, 5, 5, 0.0f, 0x000000FF, kFillModeSolid);
        }

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

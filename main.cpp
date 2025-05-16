#define NOMINMAX
#include <Novice.h>
#include <imgui.h>
#include <math.h>
#include <string>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_階層構造の構築";

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
// ベクトル・行列関連関数
//------------------------------------------------
Vector3 MakeVector3(float x, float y, float z) {
    return { x, y, z };
}

Matrix4x4 MakeIdentityMatrix() {
    return { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
}

Matrix4x4 MakeTranslateMatrix(const Vector3& t) {
    Matrix4x4 m = MakeIdentityMatrix();
    m.m[3][0] = t.x;
    m.m[3][1] = t.y;
    m.m[3][2] = t.z;
    return m;
}

Matrix4x4 MakeRotateXMatrix(float rad) {
    return { 1,0,0,0, 0,cosf(rad),sinf(rad),0, 0,-sinf(rad),cosf(rad),0, 0,0,0,1 };
}

Matrix4x4 MakeRotateYMatrix(float rad) {
    return { cosf(rad),0,-sinf(rad),0, 0,1,0,0, sinf(rad),0,cosf(rad),0, 0,0,0,1 };
}

Matrix4x4 MakeRotateZMatrix(float rad) {
    return { cosf(rad),sinf(rad),0,0, -sinf(rad),cosf(rad),0,0, 0,0,1,0, 0,0,0,1 };
}

Matrix4x4 MakeScaleMatrix(const Vector3& s) {
    Matrix4x4 m = MakeIdentityMatrix();
    m.m[0][0] = s.x;
    m.m[1][1] = s.y;
    m.m[2][2] = s.z;
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
    m.m[1][1] = height / 2;
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

//------------------------------------------------
// グリッド描画関数
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

    Vector3 translates[3] = { {0.2f, -1.7f, 0.0f}, {0.4f, -0.7f, 0.0f}, {0.3f, -0.7f, 0.0f} };
    Vector3 rotates[3] = { {0.0f, 0.0f, -6.8f}, {0.0f, 0.0f, -1.4f}, {0.0f, 0.0f, 0.0f} };
    Vector3 scales[3] = { {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} };

    Vector3 cameraTranslate = { 0.0f, 1.5f, -6.0f };
    Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };
    int mouseX, mouseY, prevMouseX = 0, prevMouseY = 0;

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // マウス操作でカメラ回転
        Novice::GetMousePosition(&mouseX, &mouseY);
        if (Novice::IsPressMouse(2)) {
            cameraRotate.y += (mouseX - prevMouseX) * 0.01f;
            cameraRotate.x += (mouseY - prevMouseY) * 0.01f;
        }
        prevMouseX = mouseX;
        prevMouseY = mouseY;

        ImGui::Begin("Joint Parameters");
        for (int i = 0; i < 3; ++i) {
            ImGui::DragFloat3(("Translate[" + std::to_string(i) + "]").c_str(), &translates[i].x, 0.01f);
            ImGui::DragFloat3(("Rotate[" + std::to_string(i) + "]").c_str(), &rotates[i].x, 0.01f);
            ImGui::DragFloat3(("Scale[" + std::to_string(i) + "]").c_str(), &scales[i].x, 0.01f);
        }
        ImGui::End();

        Matrix4x4 view = Multiply(MakeRotateXMatrix(cameraRotate.x),
            Multiply(MakeRotateYMatrix(cameraRotate.y),
                MakeTranslateMatrix(cameraTranslate)));
        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0.0f, 1.0f);

        DrawGrid(vp, viewport);

        Matrix4x4 world[3];
        for (int i = 0; i < 3; ++i) {
            Matrix4x4 S = MakeScaleMatrix(scales[i]);
            Matrix4x4 Rx = MakeRotateXMatrix(rotates[i].x);
            Matrix4x4 Ry = MakeRotateYMatrix(rotates[i].y);
            Matrix4x4 Rz = MakeRotateZMatrix(rotates[i].z);
            Matrix4x4 T = MakeTranslateMatrix(translates[i]);
            Matrix4x4 local = Multiply(S, Multiply(Rz, Multiply(Ry, Multiply(Rx, T))));
            world[i] = (i == 0) ? local : Multiply(world[i - 1], local);
        }

        Vector3 pos[3];
        for (int i = 0; i < 3; ++i) {
            pos[i] = Transform(Transform({ 0, 0, 0 }, world[i]), vp);
            pos[i] = Transform(pos[i], viewport);
        }

        // 肩→肘：赤
        Novice::DrawLine((int)pos[0].x, (int)pos[0].y, (int)pos[1].x, (int)pos[1].y, 0xFF4444FF);
        // 肘→手：青
        Novice::DrawLine((int)pos[1].x, (int)pos[1].y, (int)pos[2].x, (int)pos[2].y, 0x4444FFFF);

        Novice::DrawEllipse((int)pos[0].x, (int)pos[0].y, 8, 8, 0.0f, 0xFF0000FF, kFillModeSolid); // 肩：赤
        Novice::DrawEllipse((int)pos[1].x, (int)pos[1].y, 8, 8, 0.0f, 0x00FF00FF, kFillModeSolid); // 肘：緑
        Novice::DrawEllipse((int)pos[2].x, (int)pos[2].y, 8, 8, 0.0f, 0x0000FFFF, kFillModeSolid); // 手：青

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

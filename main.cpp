#include <Novice.h>
#include <imgui.h>
#include <stdint.h>
#include <math.h>

const char kWindowTitle[] = "球と球の衝突判定";

struct Vector3 {
    float x, y, z;
};

struct Matrix4x4 {
    float m[4][4];
};

struct Sphere {
    Vector3 center;
    float radius;
};

Vector3 Subtract(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

float Dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Length(const Vector3& v) {
    return sqrtf(Dot(v, v));
}

Matrix4x4 MakeIdentityMatrix() {
    return { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
}

Matrix4x4 MakeTranslateMatrix(Vector3 t) {
    Matrix4x4 m = MakeIdentityMatrix();
    m.m[3][0] = t.x; m.m[3][1] = t.y; m.m[3][2] = t.z;
    return m;
}

Matrix4x4 MakeRotateXMatrix(float rad) {
    return {
        1, 0, 0, 0,
        0, cosf(rad), sinf(rad), 0,
        0, -sinf(rad), cosf(rad), 0,
        0, 0, 0, 1
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

bool IsCollision(const Sphere& s1, const Sphere& s2) {
    float distance = Length(Subtract(s1.center, s2.center));
    return distance <= (s1.radius + s2.radius);
}

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
    const uint32_t kSubdivision = 16;
    const float kLonEvery = 2.0f * 3.14159f / kSubdivision;
    const float kLatEvery = 3.14159f / kSubdivision;

    for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        float lat = -3.14159f / 2.0f + kLatEvery * latIndex;
        for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
            float lon = lonIndex * kLonEvery;

            Vector3 a = {
                sphere.center.x + sphere.radius * cosf(lat) * cosf(lon),
                sphere.center.y + sphere.radius * sinf(lat),
                sphere.center.z + sphere.radius * cosf(lat) * sinf(lon)
            };
            Vector3 b = {
                sphere.center.x + sphere.radius * cosf(lat + kLatEvery) * cosf(lon),
                sphere.center.y + sphere.radius * sinf(lat + kLatEvery),
                sphere.center.z + sphere.radius * cosf(lat + kLatEvery) * sinf(lon)
            };
            Vector3 c = {
                sphere.center.x + sphere.radius * cosf(lat) * cosf(lon + kLonEvery),
                sphere.center.y + sphere.radius * sinf(lat),
                sphere.center.z + sphere.radius * cosf(lat) * sinf(lon + kLonEvery)
            };

            Vector3 aScreen = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
            Vector3 bScreen = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
            Vector3 cScreen = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

            Novice::DrawLine((int)aScreen.x, (int)aScreen.y, (int)bScreen.x, (int)bScreen.y, color);
            Novice::DrawLine((int)aScreen.x, (int)aScreen.y, (int)cScreen.x, (int)cScreen.y, color);
        }
    }
}

void DrawGrid(const Matrix4x4& vp, const Matrix4x4& viewport) {
    const float size = 2.0f;
    const int div = 10;
    const float y = -2.0f; // ▼ グリッドのY位置を下に変更
    for (int i = 0; i <= div; ++i) {
        float p = -size + (2 * size) * i / div;
        Vector3 s1 = Transform(Transform({ p, y, -size }, vp), viewport);
        Vector3 e1 = Transform(Transform({ p, y, size }, vp), viewport);
        Vector3 s2 = Transform(Transform({ -size, y, p }, vp), viewport);
        Vector3 e2 = Transform(Transform({ size, y, p }, vp), viewport);
        uint32_t color = fabsf(p) < 0.001f ? 0x000000FF : 0xAAAAAAFF;
        Novice::DrawLine((int)s1.x, (int)s1.y, (int)e1.x, (int)e1.y, color);
        Novice::DrawLine((int)s2.x, (int)s2.y, (int)e2.x, (int)e2.y, color);
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    Sphere s1 = { {0.0f, -0.5f, 1.5f}, 0.6f };  // ▼ 球1のYを下へ
    Sphere s2 = { {0.8f, -0.5f, 1.0f}, 0.4f };  // ▼ 球2のYを下へ

    Vector3 cameraTranslate = { 0.0f, 1.5f, -6.49f }; // ▼ 視点位置もYを下げて合わせる
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

        ImGui::Begin("Window");
        ImGui::DragFloat3("Sphere[0].Center", &s1.center.x, 0.01f);
        ImGui::DragFloat("Sphere[0].Radius", &s1.radius, 0.01f);
        ImGui::DragFloat3("Sphere[1].Center", &s2.center.x, 0.01f);
        ImGui::DragFloat("Sphere[1].Radius", &s2.radius, 0.01f);
        ImGui::End();

        Matrix4x4 view = Multiply(MakeRotateXMatrix(cameraRotate.x), Multiply(MakeRotateYMatrix(cameraRotate.y), MakeTranslateMatrix(cameraTranslate)));
        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

        DrawGrid(vp, viewport);
        uint32_t color1 = IsCollision(s1, s2) ? 0xFF0000FF : 0xFFFFFFFF;
        DrawSphere(s1, vp, viewport, color1);
        DrawSphere(s2, vp, viewport, 0xFFFFFFFF);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

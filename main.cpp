#include <Novice.h>
#include <stdint.h>
#include <imgui.h>
#include <math.h>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_グリッドと球の描画";

//===========================
// 構造体
//===========================
struct Vector3 {
    float x, y, z;
};

struct Sphere {
    Vector3 center;
    float radius;
};

struct Matrix4x4 {
    float m[4][4];
};

//===========================
// 行列ユーティリティ
//===========================
Matrix4x4 MakeIdentityMatrix() {
    return {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
}

Matrix4x4 MakeTranslateMatrix(Vector3 t) {
    Matrix4x4 m = MakeIdentityMatrix();
    m.m[3][0] = t.x;
    m.m[3][1] = t.y;
    m.m[3][2] = t.z;
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
    m.m[1][1] = height / 2; // Y反転しないように正のまま
    m.m[2][2] = maxDepth - minDepth;
    m.m[3][0] = left + width / 2;
    m.m[3][1] = top + height / 2;
    m.m[3][2] = minDepth;
    m.m[3][3] = 1;
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

Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
    float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
    float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
    float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
    float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
    return { x / w, y / w, z / w };
}

//===========================
// 球を描画（緯度経度グリッド）
//===========================
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
    const uint32_t kSubdivision = 16;
    const float PI = 3.1415926535f;
    const float kLonEvery = 2.0f * PI / float(kSubdivision);
    const float kLatEvery = PI / float(kSubdivision);

    for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        float lat = -PI / 2.0f + kLatEvery * latIndex;
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

            Vector3 ab = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
            Vector3 bb = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
            Vector3 cb = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

            Novice::DrawLine(int(ab.x), int(ab.y), int(bb.x), int(bb.y), color);
            Novice::DrawLine(int(ab.x), int(ab.y), int(cb.x), int(cb.y), color);
        }
    }
}

//===========================
// グリッドを描画
//===========================
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
    const float kGridHalfWidth = 2.0f;
    const uint32_t kSubdivision = 10;
    const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);

    for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
        float x = -kGridHalfWidth + kGridEvery * xIndex;
        Vector3 start = { x, 0.0f, -kGridHalfWidth };
        Vector3 end = { x, 0.0f, kGridHalfWidth };
        Vector3 s = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
        Vector3 e = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
        Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), 0xAAAAAAFF);
    }

    for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
        float z = -kGridHalfWidth + kGridEvery * zIndex;
        Vector3 start = { -kGridHalfWidth, 0.0f, z };
        Vector3 end = { kGridHalfWidth, 0.0f, z };
        Vector3 s = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
        Vector3 e = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);
        Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), 0xAAAAAAFF);
    }
}

//===========================
// メイン関数
//===========================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256] = {}, preKeys[256] = {};

    Vector3 cameraTranslate = { 0.0f, 1.9f, -10.0f };
    Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };
    Sphere sphere = { {0.0f, 1.0f, 0.0f}, 1.0f };

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ImGui::SetNextWindowSize(ImVec2(300, 200));
        ImGui::Begin("Window");
        ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
        ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
        ImGui::DragFloat3("SphereCenter", &sphere.center.x, 0.01f);
        ImGui::DragFloat("SphereRadius", &sphere.radius, 0.01f);
        ImGui::End();

        Matrix4x4 cameraMatrix = Multiply(MakeRotateXMatrix(cameraRotate.x), MakeRotateYMatrix(cameraRotate.y));
        cameraMatrix = Multiply(cameraMatrix, MakeTranslateMatrix(cameraTranslate));
        Matrix4x4 viewMatrix = cameraMatrix;  // ※本来は逆行列
        Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
        Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280, 720, 0.0f, 1.0f);

        DrawGrid(viewProjectionMatrix, viewportMatrix);
        DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

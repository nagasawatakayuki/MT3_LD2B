#include <Novice.h>
#include <imgui.h>
#include <stdint.h>
#include <math.h>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_正射影ベクトルと最近接点";

//========================================
// 構造体定義
//========================================
struct Vector3 {
    float x, y, z;
};

struct Sphere {
    Vector3 center;
    float radius;
};

struct Segment {
    Vector3 origin;  // 線分の支点
    Vector3 diff;    // 線分のベクトル（方向と長さ）
};

struct Matrix4x4 {
    float m[4][4];
};

//========================================
// ベクトル演算ユーティリティ関数
//========================================
Vector3 Add(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector3 Subtract(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

float Dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 Multiply(float scalar, const Vector3& v) {
    return { v.x * scalar, v.y * scalar, v.z * scalar };
}

//========================================
// 正射影ベクトルの計算
//========================================
Vector3 Project(const Vector3& v1, const Vector3& v2) {
    float dotVV = Dot(v2, v2);
    if (dotVV == 0) return { 0, 0, 0 };
    float scalar = Dot(v1, v2) / dotVV;
    return Multiply(scalar, v2);
}

//========================================
// 線分上の最近接点の計算
//========================================
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
    Vector3 diff = Subtract(point, segment.origin);
    Vector3 project = Project(diff, segment.diff);
    return Add(segment.origin, project);
}

//========================================
// 行列の生成と変換処理
//========================================
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

//========================================
// ベクトルの座標変換（行列適用）
//========================================
Vector3 Transform(const Vector3& v, const Matrix4x4& m) {
    float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
    float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
    float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
    float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
    return { x / w, y / w, z / w };
}

//========================================
// 描画関数とメイン関数（続き）
//========================================

// 点を円で描画（2D的表現）
void DrawSpherePoint(const Vector3& pos, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
    Sphere s = { pos, 0.01f };
    const int div = 24;
    for (int i = 0; i < div; ++i) {
        float t1 = 2 * 3.14159f * i / div;
        float t2 = 2 * 3.14159f * (i + 1) / div;
        Vector3 p1 = { s.center.x + s.radius * cosf(t1), s.center.y, s.center.z + s.radius * sinf(t1) };
        Vector3 p2 = { s.center.x + s.radius * cosf(t2), s.center.y, s.center.z + s.radius * sinf(t2) };
        Vector3 s1 = Transform(Transform(p1, viewProjectionMatrix), viewportMatrix);
        Vector3 s2 = Transform(Transform(p2, viewProjectionMatrix), viewportMatrix);
        Novice::DrawLine(int(s1.x), int(s1.y), int(s2.x), int(s2.y), color);
    }
}

// グリッドの描画（XZ平面、中心線は黒、それ以外は灰色）
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
    const float size = 2.0f;
    const int div = 10;
    const float y = -1.0f;

    for (int i = 0; i <= div; ++i) {
        float p = -size + (2 * size) * i / div;
        Vector3 s1 = Transform(Transform({ p, y, -size }, viewProjectionMatrix), viewportMatrix);
        Vector3 e1 = Transform(Transform({ p, y, size }, viewProjectionMatrix), viewportMatrix);
        Vector3 s2 = Transform(Transform({ -size, y, p }, viewProjectionMatrix), viewportMatrix);
        Vector3 e2 = Transform(Transform({ size, y, p }, viewProjectionMatrix), viewportMatrix);

        uint32_t colorX = fabsf(p) < 0.001f ? 0x000000FF : 0xAAAAAAFF;
        uint32_t colorZ = fabsf(p) < 0.001f ? 0x000000FF : 0xAAAAAAFF;

        Novice::DrawLine(int(s1.x), int(s1.y), int(e1.x), int(e1.y), colorX);
        Novice::DrawLine(int(s2.x), int(s2.y), int(e2.x), int(e2.y), colorZ);
    }
}

// メイン処理
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256] = {}, preKeys[256] = {};

    Vector3 point = { -1.5f, 0.6f, 0.6f };
    Segment segment = { {-2.0f, -1.0f, 0.0f}, {5.0f, 2.0f, 0.0f} };

    Vector3 project = Project(Subtract(point, segment.origin), segment.diff);
    Vector3 closest = ClosestPoint(point, segment);

    Vector3 cameraTranslate = { 0.0f, 1.0f, -6.0f };
    Vector3 cameraRotate = { 0.35f, 0.0f, 0.0f };

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // ImGuiウィンドウ表示（点・線分の編集）
        ImGui::Begin("Window");
        ImGui::DragFloat3("Point", &point.x, 0.01f);
        ImGui::DragFloat3("Segment Origin", &segment.origin.x, 0.01f);
        ImGui::DragFloat3("Segment Diff", &segment.diff.x, 0.01f);
        ImGui::InputFloat3("Project", &project.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::End();

        // 再計算
        project = Project(Subtract(point, segment.origin), segment.diff);
        closest = ClosestPoint(point, segment);

        // 行列計算（ビュー → 射影 → ビューポート）
        Matrix4x4 viewMatrix = Multiply(MakeRotateXMatrix(cameraRotate.x), MakeTranslateMatrix(cameraTranslate));
        Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 viewProjection = Multiply(viewMatrix, projectionMatrix);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0.0f, 1.0f);

        // グリッド描画
        DrawGrid(viewProjection, viewport);

        // 線分描画
        Vector3 segEnd = Add(segment.origin, segment.diff);
        Vector3 s = Transform(Transform(segment.origin, viewProjection), viewport);
        Vector3 e = Transform(Transform(segEnd, viewProjection), viewport);
        Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), 0xFFFFFFFF);

        // 点描画（赤：元点、黒：最近接点）
        DrawSpherePoint(point, viewProjection, viewport, RED);
        DrawSpherePoint(closest, viewProjection, viewport, BLACK);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

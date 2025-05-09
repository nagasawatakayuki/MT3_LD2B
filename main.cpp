#include <Novice.h>
#include <imgui.h>
#include <math.h>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_三角形と線分の衝突判定";

//========================================
// 基本構造体定義
//========================================
struct Vector3 {
    float x, y, z;
};

struct Matrix4x4 {
    float m[4][4];
};

struct Segment {
    Vector3 origin;
    Vector3 diff;
};

struct Triangle {
    Vector3 vertices[3];
};

//========================================
// ベクトル関数
//========================================
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
Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
float Length(const Vector3& v) {
    return sqrtf(Dot(v, v));
}
Vector3 Normalize(const Vector3& v) {
    float len = Length(v);
    return len == 0 ? Vector3{ 0, 0, 0 } : Multiply(1.0f / len, v);
}

//========================================
// 行列関係
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
        1,0,0,0,
        0,cosf(rad),sinf(rad),0,
        0,-sinf(rad),cosf(rad),0,
        0,0,0,1
    };
}
Matrix4x4 MakeRotateYMatrix(float rad) {
    return {
        cosf(rad),0,-sinf(rad),0,
        0,1,0,0,
        sinf(rad),0,cosf(rad),0,
        0,0,0,1
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

//========================================
// 衝突判定（三角形と線分）
// モーラー・トランバーグ法ベース
//========================================
bool IsCollision(const Triangle& tri, const Segment& seg) {
    Vector3 p0 = tri.vertices[0];
    Vector3 p1 = tri.vertices[1];
    Vector3 p2 = tri.vertices[2];
    Vector3 dir = seg.diff;
    Vector3 edge1 = Subtract(p1, p0);
    Vector3 edge2 = Subtract(p2, p0);
    Vector3 h = Cross(dir, edge2);
    float det = Dot(edge1, h);
    if (fabs(det) < 1e-5f) return false;

    float invDet = 1.0f / det;
    Vector3 s = Subtract(seg.origin, p0);
    float u = Dot(s, h) * invDet;
    if (u < 0.0f || u > 1.0f) return false;

    Vector3 q = Cross(s, edge1);
    float v = Dot(dir, q) * invDet;
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = Dot(edge2, q) * invDet;
    return (t >= 0.0f && t <= 1.0f); // 線分の範囲内なら衝突
}

//========================================
// グリッド描画
//========================================
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

//========================================
// 三角形描画
//========================================
void DrawTriangle(const Triangle& tri, const Matrix4x4& vp, const Matrix4x4& viewport, unsigned int color) {
    for (int i = 0; i < 3; ++i) {
        Vector3 a = Transform(Transform(tri.vertices[i], vp), viewport);
        Vector3 b = Transform(Transform(tri.vertices[(i + 1) % 3], vp), viewport);
        Novice::DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
    }
}

//========================================
// メイン関数
//========================================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256] = {}, preKeys[256] = {};

    Triangle tri = { {
        {-1.0f, -2.0f, 0.0f},
        {1.0f, -2.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}
    } };

    Segment seg = { {0.6f, -1.5f, -2.0f}, {0.0f, -2.0f, 1.0f} };

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

        // ImGui操作
        ImGui::Begin("Window");
        ImGui::DragFloat3("Triangle.v0", &tri.vertices[0].x, 0.01f);
        ImGui::DragFloat3("Triangle.v1", &tri.vertices[1].x, 0.01f);
        ImGui::DragFloat3("Triangle.v2", &tri.vertices[2].x, 0.01f);
        ImGui::DragFloat3("Segment.Origin", &seg.origin.x, 0.01f);
        ImGui::DragFloat3("Segment.Diff", &seg.diff.x, 0.01f);
        ImGui::End();

        Matrix4x4 view = Multiply(MakeRotateXMatrix(cameraRotate.x), Multiply(MakeRotateYMatrix(cameraRotate.y), MakeTranslateMatrix(cameraTranslate)));
        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

        DrawGrid(vp, viewport);
        DrawTriangle(tri, vp, viewport, 0xFFFFFFFF);

        Vector3 s0 = Transform(Transform(seg.origin, vp), viewport);
        Vector3 s1 = Transform(Transform(Add(seg.origin, seg.diff), vp), viewport);
        unsigned int color = IsCollision(tri, seg) ? 0xFF0000FF : 0xFFFFFFFF;
        Novice::DrawLine((int)s0.x, (int)s0.y, (int)s1.x, (int)s1.y, color);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

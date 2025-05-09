#include <Novice.h>
#include <imgui.h>
#include <math.h>
#include <algorithm>

const char kWindowTitle[] = "LD2B_04_ナガサワ_タカユキ_AABBとSphereの衝突判定";

struct Vector3 {
    float x, y, z;
};

struct Matrix4x4 {
    float m[4][4];
};

struct AABB {
    Vector3 min;
    Vector3 max;
};

struct Sphere {
    Vector3 center;
    float radius;
};

//==================================================
// ベクトル計算
//==================================================
Vector3 Add(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector3 Subtract(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

float Dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Length(const Vector3& v) {
    return sqrtf(Dot(v, v));
}

float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}



Vector3 ClosestPoint(const Vector3& point, const AABB& aabb) {
    return {
        Clamp(point.x, aabb.min.x, aabb.max.x),
        Clamp(point.y, aabb.min.y, aabb.max.y),
        Clamp(point.z, aabb.min.z, aabb.max.z)
    };
}

//==================================================
// 行列処理
//==================================================
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

//==================================================
// 衝突判定（Sphere vs AABB）
//==================================================
bool IsCollision(const AABB& aabb, const Sphere& sphere) {
    Vector3 closest = ClosestPoint(sphere.center, aabb);
    Vector3 diff = Subtract(closest, sphere.center);
    float distanceSq = Dot(diff, diff);
    return distanceSq <= sphere.radius * sphere.radius;
}

//==================================================
// 描画処理
//==================================================
void DrawAABB(const AABB& aabb, const Matrix4x4& vp, const Matrix4x4& viewport, unsigned int color) {
    Vector3 v[8] = {
        {aabb.min.x, aabb.min.y, aabb.min.z},
        {aabb.max.x, aabb.min.y, aabb.min.z},
        {aabb.min.x, aabb.max.y, aabb.min.z},
        {aabb.max.x, aabb.max.y, aabb.min.z},
        {aabb.min.x, aabb.min.y, aabb.max.z},
        {aabb.max.x, aabb.min.y, aabb.max.z},
        {aabb.min.x, aabb.max.y, aabb.max.z},
        {aabb.max.x, aabb.max.y, aabb.max.z},
    };
    int edges[12][2] = {
        {0,1},{1,3},{3,2},{2,0}, {4,5},{5,7},{7,6},{6,4}, {0,4},{1,5},{2,6},{3,7}
    };
    for (int i = 0; i < 12; ++i) {
        Vector3 a = Transform(Transform(v[edges[i][0]], vp), viewport);
        Vector3 b = Transform(Transform(v[edges[i][1]], vp), viewport);
        Novice::DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
    }
}

// 球体を緯度・経度のラインで描画
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
    const uint32_t kSubdivision = 16;
    const float kLonEvery = 2.0f * 3.141592f / kSubdivision;
    const float kLatEvery = 3.141592f / kSubdivision;

    for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        float lat = -3.141592f / 2.0f + kLatEvery * latIndex;

        for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
            float lon = lonIndex * kLonEvery;

            // 各点のワールド座標を求める
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

            // 座標変換（ワールド → スクリーン）
            Vector3 aScreen = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
            Vector3 bScreen = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
            Vector3 cScreen = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

            // 線で描画
            Novice::DrawLine((int)aScreen.x, (int)aScreen.y, (int)bScreen.x, (int)bScreen.y, color);
            Novice::DrawLine((int)aScreen.x, (int)aScreen.y, (int)cScreen.x, (int)cScreen.y, color);
        }
    }
}


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

//==================================================
// メイン処理
//==================================================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256]{}, preKeys[256]{};

    AABB aabb = { {-1.5f, -1.5f, -0.5f}, {-0.5f, -0.5f, 0.5f} };
    Sphere sphere = { {0.4f, -0.9f, 0.0f}, 0.5f };

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

        ImGui::Begin("Window");
        ImGui::DragFloat3("AABB Min", &aabb.min.x, 0.01f);
        ImGui::DragFloat3("AABB Max", &aabb.max.x, 0.01f);
        ImGui::DragFloat3("Sphere Center", &sphere.center.x, 0.01f);
        ImGui::DragFloat("Sphere Radius", &sphere.radius, 0.01f);
        ImGui::End();

        for (int i = 0; i < 3; ++i) {
            float& minVal = ((float*)&aabb.min)[i];
            float& maxVal = ((float*)&aabb.max)[i];
            if (minVal > maxVal) std::swap(minVal, maxVal);
        }

        Matrix4x4 view = Multiply(MakeRotateXMatrix(cameraRotate.x), Multiply(MakeRotateYMatrix(cameraRotate.y), MakeTranslateMatrix(cameraTranslate)));
        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

        DrawGrid(vp, viewport);

        bool hit = IsCollision(aabb, sphere);
        DrawAABB(aabb, vp, viewport, hit ? 0xFF0000FF : 0xFFFFFFFF);
        DrawSphere(sphere, vp, viewport, hit ? 0xFF0000FF : 0xFFFFFFFF);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

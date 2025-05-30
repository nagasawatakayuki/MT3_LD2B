#include <Novice.h>
#include <imgui.h>
#include <math.h>

const char kWindowTitle[] = "振り子シミュレーション";

//==================================================
// 基本構造体
//==================================================
struct Vector3 {
    float x, y, z;
};

struct Matrix4x4 {
    float m[4][4];
};

//==================================================
// 振り子構造体
//==================================================
struct Pendulum {
    Vector3 anchor;             // 固定点
    float length;               // 紐の長さ
    float angle;                // 現在の角度（rad）
    float angularVelocity;      // 角速度
    float angularAcceleration;  // 角加速度
};

//==================================================
// 行列関連関数
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

//==================================================
// グリッド描画
//==================================================
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
// メイン関数
//==================================================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256]{}, preKeys[256]{};

    // 振り子初期値（資料準拠）
    Pendulum pendulum;
    pendulum.anchor = { 0.0f, -1.0f, 0.0f };
    pendulum.length = 0.8f;
    pendulum.angle = 0.7f;
    pendulum.angularVelocity = 0.0f;
    pendulum.angularAcceleration = 0.0f;

    const float g = 9.8f;
    const float deltaTime = 1.0f / 60.0f;
    bool isRunning = false;

    // カメラ設定
    Vector3 cameraTranslate = { 0.0f, 1.5f, -6.0f };
    Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // ImGui UI
        ImGui::Begin("Window");
        if (ImGui::Button("Start")) {
            isRunning = true;
        }
        ImGui::End();

        // シミュレーション開始後の更新
        if (isRunning) {
            pendulum.angularAcceleration = -(g / pendulum.length) * sinf(pendulum.angle);
            pendulum.angularVelocity += pendulum.angularAcceleration * deltaTime;
            pendulum.angle += pendulum.angularVelocity * deltaTime;
        }

        // 位置計算（円周の下向き）
        Vector3 tip{};
        tip.x = pendulum.anchor.x + sinf(pendulum.angle) * pendulum.length;
        tip.y = pendulum.anchor.y - cosf(pendulum.angle) * pendulum.length;
        tip.z = pendulum.anchor.z;

        // 行列計算
        Matrix4x4 view = Multiply(
            MakeRotateXMatrix(cameraRotate.x),
            Multiply(MakeRotateYMatrix(cameraRotate.y),
                MakeTranslateMatrix(cameraTranslate))
        );
        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

        // 描画
        DrawGrid(vp, viewport);
        Vector3 a = Transform(Transform(pendulum.anchor, vp), viewport);
        Vector3 b = Transform(Transform(tip, vp), viewport);
        Novice::DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, 0xFFFFFFFF);
        Novice::DrawEllipse((int)b.x, (int)b.y, 8, 8, 0.0f, 0xFFFFFFFF, kFillModeSolid);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

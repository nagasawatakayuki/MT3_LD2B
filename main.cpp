#define NOMINMAX
#include <Novice.h>
#include <imgui.h>
#include <math.h>
#include <string>

const char kWindowTitle[] = "LD2B_04_ナガサワ_ばね構造体のシミュレーション";

//================================================
// Vector3型と基本演算
//================================================
struct Vector3 {
    float x, y, z;

    Vector3 operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vector3 operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }
    Vector3 operator/(float s) const { return { x / s, y / s, z / s }; }

    Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
};

float Length(const Vector3& v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 Normalize(const Vector3& v) {
    float len = Length(v);
    return len != 0.0f ? v / len : Vector3{ 0,0,0 };
}

//================================================
// Ball構造体
//================================================
struct Ball {
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    float radius;
    unsigned int color;
};

//================================================
// Spring構造体
//================================================
struct Spring {
    Vector3 anchor;
    float naturalLength;
    float stiffness;
    float dampingCoefficient;
};

//================================================
// Matrix4x4 (ビュープロジェクション用)
//================================================
struct Matrix4x4 {
    float m[4][4];
};

Matrix4x4 MakeIdentityMatrix() {
    return { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
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

Matrix4x4 MakeTranslateMatrix(Vector3 t) {
    Matrix4x4 m = MakeIdentityMatrix();
    m.m[3][0] = t.x;
    m.m[3][1] = t.y;
    m.m[3][2] = t.z;
    return m;
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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256]{}, preKeys[256]{};

    Spring spring{ {0, -2.0, 0}, 0.5f, 100.0f, 2.0f };
    Ball ball{ {1.2f, -2.0, 0}, {}, {}, 2.0f, 0.05f, 0x0000FFFF };
    float deltaTime = 1.0f / 60.0f;

    bool useSpring = false;
    bool justEnabled = false;

    // 固定カメラ
    Vector3 cameraTranslate = { 0.0f, 1.5f, -6.0f };
    Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        if (preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE] != 0) {
            useSpring = !useSpring;
            justEnabled = useSpring;
        }

        // 物理演算
        if (useSpring) {
            if (justEnabled) {
                ball.velocity = {};
                ball.acceleration = {};
                justEnabled = false;
            }
            Vector3 diff = ball.position - spring.anchor;
            float length = Length(diff);
            if (length != 0.0f) {
                Vector3 direction = Normalize(diff);
                Vector3 restPos = spring.anchor + direction * spring.naturalLength;
                Vector3 displacement = ball.position - restPos;
                Vector3 restoringForce = displacement * -spring.stiffness;
                Vector3 dampingForce = ball.velocity * -spring.dampingCoefficient;
                Vector3 force = restoringForce + dampingForce;
                ball.acceleration = force / ball.mass;
            }
            ball.velocity += ball.acceleration * deltaTime;
            ball.position += ball.velocity * deltaTime;
        }

        // ビュー行列（三角形コードと同様）
        Matrix4x4 view = Multiply(
            MakeRotateXMatrix(cameraRotate.x),
            Multiply(MakeRotateYMatrix(cameraRotate.y),
                MakeTranslateMatrix(cameraTranslate))
        );

        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

        DrawGrid(vp, viewport);

        // 表示
        Vector3 anchorScreen = Transform(Transform(spring.anchor, vp), viewport);
        Vector3 ballScreen = Transform(Transform(ball.position, vp), viewport);
        Novice::DrawLine((int)anchorScreen.x, (int)anchorScreen.y, (int)ballScreen.x, (int)ballScreen.y, 0xFF0000FF);
        Novice::DrawEllipse((int)ballScreen.x, (int)ballScreen.y, 8, 8, 0.0f, ball.color, kFillModeSolid);

        ImGui::Begin("Spring Settings");
        ImGui::DragFloat3("Ball Pos", &ball.position.x, 0.01f);
        ImGui::DragFloat3("Ball Vel", &ball.velocity.x, 0.01f);
        ImGui::DragFloat("Mass", &ball.mass, 0.01f);
        ImGui::DragFloat("Stiffness", &spring.stiffness, 1.0f);
        ImGui::DragFloat("Damping", &spring.dampingCoefficient, 0.1f);
        ImGui::DragFloat("Natural Length", &spring.naturalLength, 0.01f);
        ImGui::Text("Press SPACE to toggle spring force [%s]", useSpring ? "ON" : "OFF");
        ImGui::End();

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

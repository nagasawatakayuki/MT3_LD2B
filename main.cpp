#include <Novice.h>
#include <imgui.h>
#include <math.h>

const char kWindowTitle[] = "Ball Plane Collision Adjusted";

struct Vector3 {
    float x, y, z;
};

struct Plane {
    Vector3 normal;
    float distance;
};

struct Ball {
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    float radius;
    unsigned int color;
};

struct Matrix4x4 {
    float m[4][4];
};

Vector3 Normalize(const Vector3& v) {
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return { v.x / len, v.y / len, v.z / len };
}

float Dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 Add(const Vector3& a, const Vector3& b) {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector3 Subtract(const Vector3& a, const Vector3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vector3 Multiply(const Vector3& v, float scalar) {
    return { v.x * scalar, v.y * scalar, v.z * scalar };
}

Vector3 Project(const Vector3& v, const Vector3& n) {
    float d = Dot(v, n);
    return Multiply(n, d);
}

Vector3 Reflect(const Vector3& input, const Vector3& normal) {
    return Subtract(input, Multiply(normal, 2.0f * Dot(input, normal)));
}

bool IsCollision(const Vector3& sphereCenter, float radius, const Plane& plane) {
    float dist = Dot(sphereCenter, plane.normal) - plane.distance;
    return fabsf(dist) < radius;
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
    Matrix4x4 r{};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                r.m[i][j] += m1.m[i][k] * m2.m[k][j];
    return r;
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

void DrawGrid(const Matrix4x4& vp, const Matrix4x4& viewport) {
    const float size = 2.0f;
    const int div = 10;
    const float y = -2.0f; // ← ここを円錐振り子と揃える
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

    Plane plane;
    plane.normal = Normalize({ 0.0f, 1.0f, 0.0f });
    plane.distance = 2.0f; // y = -2.0f の面

    Ball ball{};
    ball.position = { 0.8f, 1.2f, 0.3f };
    ball.mass = 2.0f;
    ball.radius = 0.05f;
    ball.color = 0xFFFFFFFF;
    ball.velocity = { 0,0,0 };
    ball.acceleration = { 0.0f, -9.8f, 0.0f };

    Vector3 cameraTranslate = { 0.0f, 1.5f, -6.0f };
    Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };

    bool isRunning = false;
    const float deltaTime = 1.0f / 60.0f;

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ImGui::Begin("Window");
        if (ImGui::Button("Start")) isRunning = true;
        ImGui::End();

        if (isRunning) {
            ball.velocity = Add(ball.velocity, Multiply(ball.acceleration, deltaTime));
            ball.position = Add(ball.position, Multiply(ball.velocity, deltaTime));

            if (IsCollision(ball.position, ball.radius, plane)) {
                Vector3 reflected = Reflect(ball.velocity, plane.normal);
                Vector3 projectToNormal = Project(reflected, plane.normal);
                Vector3 movingDirection = Subtract(reflected, projectToNormal);
                ball.velocity = Add(Multiply(projectToNormal, -1.0f), movingDirection);
            }
        }

        Matrix4x4 view = Multiply(
            MakeRotateXMatrix(cameraRotate.x),
            Multiply(MakeRotateYMatrix(cameraRotate.y), MakeTranslateMatrix(cameraTranslate)));
        Matrix4x4 proj = MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
        Matrix4x4 vp = Multiply(view, proj);
        Matrix4x4 viewport = MakeViewportMatrix(0, 0, 1280, 720, 0, 1);

        DrawGrid(vp, viewport);

        Vector3 screen = Transform(Transform(ball.position, vp), viewport);
        Novice::DrawEllipse((int)screen.x, (int)screen.y, (int)(ball.radius * 100), (int)(ball.radius * 100), 0.0f, ball.color, kFillModeSolid);

        Novice::EndFrame();
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) break;
    }

    Novice::Finalize();
    return 0;
}

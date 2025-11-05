#include "raylib.h"
#include "raymath.h"

struct Player {
    Vector2 position;
    float rotation;
    float speed;
    float size;
    float sensitivity;

    Player(Vector2 pos) {
        position = pos;
        rotation = 0;
        speed = 3;
        size = 20;
        sensitivity = 0.25f;
    }

    void Update() {
        rotation += GetMouseDelta().x * sensitivity; // Rotate with mouse

        Vector2 forward = Vector2Rotate({ 1, 0 }, DEG2RAD * rotation);
        Vector2 right = Vector2Rotate({ 0, 1 }, DEG2RAD * rotation);

        if (IsKeyDown(KEY_W)) position = Vector2Add(position, Vector2Scale(forward, speed));
        if (IsKeyDown(KEY_S)) position = Vector2Subtract(position, Vector2Scale(forward, speed));
        if (IsKeyDown(KEY_A)) position = Vector2Subtract(position, Vector2Scale(right, speed));
        if (IsKeyDown(KEY_D)) position = Vector2Add(position, Vector2Scale(right, speed));
    }

    void Draw() {
        Vector2 forward = Vector2Rotate({ 1, 0 }, DEG2RAD * rotation);
        Vector2 right = Vector2Rotate({ 0, 1 }, DEG2RAD * rotation);

        Vector2 v1 = Vector2Add(position, Vector2Scale(forward, size));
        Vector2 v2 = Vector2Subtract(position, Vector2Add(Vector2Scale(forward, size * 0.5f), Vector2Scale(right, size * 0.5f)));
        Vector2 v3 = Vector2Subtract(position, Vector2Add(Vector2Scale(forward, size * 0.5f), Vector2Scale(right, -size * 0.5f)));

        DrawTriangle(v1, v2, v3, WHITE);
    }
};

void SetSoundPosition2D(Vector2 listenerPos, float listenerRot, Sound sound, Vector2 sourcePos, float maxDist) {
    Vector2 direction = Vector2Subtract(sourcePos, listenerPos);
    float distance = Vector2Length(direction);

    float attenuation = 1.0f / (1.0f + (distance / maxDist));
    attenuation = Clamp(attenuation, 0.0f, 1.0f);

    Vector2 forward = Vector2Rotate({ 1, 0 }, DEG2RAD * listenerRot);
    Vector2 right = Vector2Rotate({ 0, -1 }, DEG2RAD * listenerRot);

    Vector2 normalizedDir = Vector2Normalize(direction);
    float dotFront = Vector2DotProduct(forward, normalizedDir);
    float dotRight = Vector2DotProduct(right, normalizedDir);

    if (dotFront < 0) attenuation *= (1.0f + dotFront * 0.5f);
    float pan = 0.5f + 0.5f * dotRight;

    SetSoundVolume(sound, attenuation);
    SetSoundPan(sound, pan);
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "WAAAAAAGH!!!");
    InitAudioDevice();
    SetTargetFPS(60);
    DisableCursor();

    Player player = { { screenWidth / 2.0f, screenHeight / 2.0f }};
    Vector2 ballPos = { screenWidth / 2.0f, screenHeight / 2.0f - 150.0f };

    Sound sound = LoadSound("resources/coin.wav");

    PlaySound(sound);
    SetSoundVolume(sound, 1.0f);

    while (!WindowShouldClose()) {
        player.Update();

        if (!IsSoundPlaying(sound)) PlaySound(sound);

        SetSoundPosition2D(player.position, player.rotation, sound, ballPos, 100.0f);

        BeginDrawing();
        ClearBackground(BLACK);

        player.Draw();
        DrawCircleV(ballPos, 10, RED);
        DrawText("WASD to move, mouse to rotate", 20, 20, 20, GRAY);
        DrawText("Walk around and listen!", 20, 50, 20, GRAY);

        EndDrawing();
    }

    UnloadSound(sound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

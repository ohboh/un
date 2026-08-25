#include "raylib.h"
#include "raymath.h"
#include <vector>

using namespace std;

// Forward declarations
void SetSoundPosition2D(Vector2 listenerPos, float listenerRot, Sound sound, Vector2 sourcePos, float maxDist);
void DrawCompass(Texture2D compass, float rotation);

// ============================================================================
// PLAYER
// ============================================================================
struct Player {
    Vector2 position;
    float rotation;
    float speed;
    float size;
    float sensitivity;
    bool tankControls;

    Player(Vector2 pos, bool useTankControls = false) {
        position = pos;
        rotation = 0.0f;
        speed = 100.0f;
        size = 20.0f;
        sensitivity = 0.25f;
        tankControls = useTankControls;
    }

    void Update(float dt) {
        Vector2 forward = Vector2Rotate({ 1, 0 }, DEG2RAD * rotation);
        Vector2 right = Vector2Rotate({ 0, 1 }, DEG2RAD * rotation);

        float rotSpeed = 120.0f; // degrees per second
        Vector2 move = { 0, 0 };

        if (tankControls) {
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) rotation -= rotSpeed * dt;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) rotation += rotSpeed * dt;

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move = Vector2Add(move, forward);
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move = Vector2Subtract(move, forward);
            if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_DELETE)) move = Vector2Subtract(move, right);
            if (IsKeyDown(KEY_E) || IsKeyDown(KEY_PAGE_DOWN)) move = Vector2Add(move, right);
        }
        else {
            rotation += GetMouseDelta().x * sensitivity;

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move = Vector2Add(move, forward);
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move = Vector2Subtract(move, forward);
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move = Vector2Subtract(move, right);
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move = Vector2Add(move, right);
        }

        if (Vector2Length(move) > 0) {
            move = Vector2Normalize(move);
            position = Vector2Add(position, Vector2Scale(move, speed * dt));
        }
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

// ============================================================================
// SOUND MANAGER
// ============================================================================
struct SoundManager {
    Sound orcSpawn, orcWalk, orcWindup, orcAttack, orcDeath;
    Sound skeletonSpawn, skeletonWalk, skeletonWindup, skeletonAttack, skeletonDeath;

    void Load() {
        orcSpawn  = LoadSound("resources/orc.wav");
        orcWalk   = LoadSound("resources/orc_walk.wav");
        orcWindup = LoadSound("resources/orc_windup.wav");
        orcAttack = LoadSound("resources/orc_attack.wav");
        orcDeath  = LoadSound("resources/orc.wav");

        skeletonSpawn  = LoadSound("resources/bones.wav");
        skeletonWalk   = LoadSound("resources/skeleton_walk.wav");
        skeletonWindup = LoadSound("resources/skeleton_windup.wav");
        skeletonAttack = LoadSound("resources/skeleton_attack.wav");
        skeletonDeath  = LoadSound("resources/bones.wav");

        // Report exactly which files didn't load, instead of finding out
        // via a segfault three systems away.
        CheckLoaded(orcSpawn, "orc.wav");
        CheckLoaded(orcWalk, "orc_walk.wav");
        CheckLoaded(orcWindup, "orc_windup.wav");
        CheckLoaded(orcAttack, "orc_attack.wav");
        CheckLoaded(orcDeath, "orc.wav");
        CheckLoaded(skeletonSpawn, "bones.wav");
        CheckLoaded(skeletonWalk, "skeleton_walk.wav");
        CheckLoaded(skeletonWindup, "skeleton_windup.wav");
        CheckLoaded(skeletonAttack, "skeleton_attack.wav");
        CheckLoaded(skeletonDeath, "bones.wav");
    }

    void CheckLoaded(Sound s, const char *name) {
        if (!IsSoundValid(s)) {
            TraceLog(LOG_WARNING, "SOUNDMANAGER: '%s' failed to load - check that resources/ is in the working directory", name);
        }
    }

    void Unload() {
        UnloadSound(orcSpawn);
        UnloadSound(orcWalk);
        UnloadSound(orcWindup);
        UnloadSound(orcAttack);
        UnloadSound(orcDeath);

        UnloadSound(skeletonSpawn);
        UnloadSound(skeletonWalk);
        UnloadSound(skeletonWindup);
        UnloadSound(skeletonAttack);
        UnloadSound(skeletonDeath);
    }
};

// ============================================================================
// ENEMY
// ============================================================================
struct Enemy {
    enum EnemyType { ORC, SKELETON };
    enum State { SPAWNING, WALKING, WINDUP, ATTACKING, DYING } state;

    Vector2 position;
    Color color;
    float speed;
    float attackRange;

    float stateTimer;
    float spawnTime;
    float windupTime;
    float attackTime;
    Vector2 attackDir;

    Sound spawnSound;
    Sound walkSound;
    Sound windupSound;
    Sound attackSound;
    Sound deathSound;
    Sound currentSound;

    static Sound SafeAlias(Sound source) {
        if (IsSoundValid(source)) {
            return LoadSoundAlias(source);
        }
        return Sound{ 0 };
    }

    Enemy(Vector2 pos, EnemyType type, SoundManager &sm) {
        position = pos;
        switch (type) {
            case ORC:
                spawnSound  = SafeAlias(sm.orcSpawn);
                walkSound   = SafeAlias(sm.orcWalk);
                windupSound = SafeAlias(sm.orcWindup);
                attackSound = SafeAlias(sm.orcAttack);
                deathSound  = SafeAlias(sm.orcDeath);

                color = GREEN;
                speed = 50.0f;
                spawnTime = 1.0f;
                attackRange = 40.0f;
                windupTime = 0.7f;
                attackTime = 0.4f;
                break;

            case SKELETON:
                spawnSound  = SafeAlias(sm.skeletonSpawn);
                walkSound   = SafeAlias(sm.skeletonWalk);
                windupSound = SafeAlias(sm.skeletonWindup);
                attackSound = SafeAlias(sm.skeletonAttack);
                deathSound  = SafeAlias(sm.skeletonDeath);

                color = GRAY;
                speed = 80.0f;
                spawnTime = 0.5f;
                attackRange = 35.0f;
                windupTime = 0.4f;
                attackTime = 0.2f;
                break;
        }
        state = SPAWNING;
        stateTimer = 0.0f;
        currentSound = walkSound;
    }

    void UnloadSounds() {
        if (IsSoundValid(spawnSound))  UnloadSoundAlias(spawnSound);
        if (IsSoundValid(walkSound))   UnloadSoundAlias(walkSound);
        if (IsSoundValid(windupSound)) UnloadSoundAlias(windupSound);
        if (IsSoundValid(attackSound)) UnloadSoundAlias(attackSound);
        if (IsSoundValid(deathSound))  UnloadSoundAlias(deathSound);
    }

    void Update(Vector2 playerPos, float listenerRot, float dt) {
        Vector2 toPlayer = Vector2Subtract(playerPos, position);
        float distance = Vector2Length(toPlayer);
        Vector2 dir = Vector2Normalize(toPlayer);

        stateTimer += dt;

        switch (state) {
            case SPAWNING:
                currentSound = spawnSound;
                if (stateTimer >= spawnTime) {
                    state = WALKING;
                    stateTimer = 0.0f;
                }
                break;
            case WALKING:
                currentSound = walkSound;
                if (distance <= attackRange) {
                    state = WINDUP;
                    stateTimer = 0.0f;
                    attackDir = dir;
                } else {
                    position = Vector2Add(position, Vector2Scale(dir, speed * dt));
                }
                break;
            case WINDUP:
                currentSound = windupSound;
                if (stateTimer >= windupTime) {
                    state = ATTACKING;
                    stateTimer = 0.0f;
                }
                break;
            case ATTACKING:
                currentSound = attackSound;
                if (stateTimer >= attackTime) {
                    state = WALKING;
                    stateTimer = 0.0f;
                }
                break;
            case DYING:
                break;
        }

        if (IsSoundValid(currentSound)) {
            if (!IsSoundPlaying(currentSound)) PlaySound(currentSound);
            SetSoundPosition2D(playerPos, listenerRot, currentSound, position, 50.0f);
        }
    }

    void Draw() {
        DrawCircleV(position, 10, color);

        if (state == WINDUP || state == ATTACKING) {
            Vector2 boxCenter = Vector2Add(position, Vector2Scale(attackDir, 25));
            float boxSize = 20;

            Color hitboxColor = (state == WINDUP) ? (Color){255, 255, 0, 100}
                                                  : (Color){255, 0, 0, 120};

            DrawRectangleV(
                Vector2Subtract(boxCenter, {boxSize / 2, boxSize / 2}),
                { boxSize, boxSize },
                hitboxColor
            );
        }
    }
};

// ============================================================================
// MAIN LOOP
// ============================================================================
int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "WAAAAAAGH!!!");
    InitAudioDevice();
    SetTargetFPS(60);
    DisableCursor();

    Player player({ screenWidth / 2.0f, screenHeight / 2.0f });
    SoundManager sm;
    sm.Load();

    Texture2D compass = LoadTexture("resources/compass2.png");
    if (!IsTextureValid(compass)) {
        TraceLog(LOG_WARNING, "TEXTURE: 'compass2.png' failed to load - check that resources/ is in the working directory");
    }

    vector<Enemy> enemies;

    // 2D Camera centered on player
    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Update logic
        player.Update(dt);
        camera.target = player.position;

        for (auto &e : enemies) {
            e.Update(player.position, player.rotation, dt);
        }

         if (IsKeyPressed(KEY_SPACE)) { // Debug spawn
            float spawnAngle = (float)GetRandomValue(0, 359);
            float spawnDist = (float)GetRandomValue(200, 400);
            Vector2 offset = Vector2Scale(Vector2Rotate({ 1, 0 }, DEG2RAD * spawnAngle), spawnDist);
            Vector2 spawnPos = Vector2Add(player.position, offset);
 
            Enemy::EnemyType type = (GetRandomValue(0, 1) == 0) ? Enemy::ORC : Enemy::SKELETON;
            enemies.emplace_back(spawnPos, type, sm);
        }

        // Draw pipeline
        BeginDrawing();
            ClearBackground(BLACK);

            // World space rendering
            BeginMode2D(camera);
                player.Draw();
                for (auto &e : enemies) {
                    e.Draw();
                }
            EndMode2D();

            // Screen/HUD space rendering
            if (IsTextureValid(compass)) {
                DrawCompass(compass, -player.rotation);
            }
            DrawText("WS/Arrow keys to move, Mouse/AD to rotate, Space to spawn enemy", 20, 20, 20, GRAY);

        EndDrawing();
    }

    for (auto &e : enemies) {
        e.UnloadSounds();
    }

    UnloadTexture(compass);
    sm.Unload();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
void SetSoundPosition2D(Vector2 listenerPos, float listenerRot, Sound sound, Vector2 sourcePos, float maxDist) {
    Vector2 direction = Vector2Subtract(sourcePos, listenerPos);
    float distance = Vector2Length(direction);

    float attenuation = 1.0f / (1.0f + (distance / maxDist));
    attenuation = Clamp(attenuation, 0.0f, 1.0f);

    Vector2 forward = Vector2Rotate({ 1, 0 }, DEG2RAD * listenerRot);
    Vector2 right = Vector2Rotate({ 0, 1 }, DEG2RAD * listenerRot);

    Vector2 normalizedDir = Vector2Normalize(direction);
    float dotFront = Vector2DotProduct(forward, normalizedDir);
    float dotRight = Vector2DotProduct(right, normalizedDir);

    if (dotFront < 0) attenuation *= (1.0f + dotFront * 0.5f);
    float pan = 0.5f + 0.5f * dotRight;

    SetSoundVolume(sound, attenuation);
    SetSoundPan(sound, pan);
}

void DrawCompass(Texture2D compass, float rotation) {
    float size = 500;
    Vector2 pos = { size / 2.5f, size / 2 };
    Rectangle src = { 0, 0, (float)compass.width, (float)compass.height };
    Rectangle dest = { pos.x, pos.y, size, size };
    Vector2 origin = { size / 2, size / 2 };

    DrawTexturePro(
        compass,
        src,
        dest,
        origin,
        rotation,
        WHITE
    );
}

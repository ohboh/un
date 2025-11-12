#include "raylib.h"
#include "raymath.h"

void SetSoundPosition2D(Vector2 listenerPos, float listenerRot, Sound sound, Vector2 sourcePos, float maxDist);

struct Player {
	Vector2 position;
	float rotation;
	float speed;
	float size;
	float sensitivity;
	bool tankControls;

	Player(Vector2 pos, bool useTankControls = true) {
		position = pos;
		rotation = 0;
		speed = 100;
		size = 20;
		sensitivity = 0.25f;
		tankControls = useTankControls;
	}

	void Update(float dt) {
		Vector2 forward = Vector2Rotate({ 1, 0 }, DEG2RAD * rotation);
		Vector2 right = Vector2Rotate({ 0, 1 }, DEG2RAD * rotation);

		float rotSpeed = 120.0f; // degrees per second

		if (tankControls) {
			// 🕹️ Tank-style movement
			if (IsKeyDown(KEY_A)) rotation -= rotSpeed * dt; // Turn left
			if (IsKeyDown(KEY_D)) rotation += rotSpeed * dt; // Turn right

			if (IsKeyDown(KEY_W)) position = Vector2Add(position, Vector2Scale(forward, speed * dt));
			if (IsKeyDown(KEY_S)) position = Vector2Subtract(position, Vector2Scale(forward, speed * dt));
			if (IsKeyDown(KEY_Q)) position = Vector2Subtract(position, Vector2Scale(right, speed * dt));
			if (IsKeyDown(KEY_E)) position = Vector2Add(position, Vector2Scale(right, speed * dt));
		}
		else {
			// 🎯 Free-movement (mouse aim) mode
			rotation += GetMouseDelta().x * sensitivity;

			if (IsKeyDown(KEY_W)) position = Vector2Add(position, Vector2Scale(forward, speed * dt));
			if (IsKeyDown(KEY_S)) position = Vector2Subtract(position, Vector2Scale(forward, speed * dt));
			if (IsKeyDown(KEY_A)) position = Vector2Subtract(position, Vector2Scale(right, speed * dt));
			if (IsKeyDown(KEY_D)) position = Vector2Add(position, Vector2Scale(right, speed * dt));
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

enum EnemyType { ORC, SKELETON };

struct Enemy {
	Vector2 position;
	Sound sound;
	Color color;
	float speed;
	float attackRange;

	enum State { WALKING, WINDUP, ATTACKING } state;
	float stateTimer;
	float windupTime;
	float attackTime;
	Vector2 attackDir;

	Enemy(Vector2 pos, EnemyType type = ORC) {
		position = pos;
		switch (type) {
			case ORC:
				sound = LoadSound("resources/orc.wav");
				color = GREEN;
				speed = 50.0f;
				attackRange = 40.0f;
				windupTime = 0.7f;
				attackTime = 0.4f;
				break;
			case SKELETON:
				sound = LoadSound("resources/bones.wav");
				color = GRAY;
				speed = 80.0f;
				attackRange = 35.0f;
				windupTime = 0.4f;
				attackTime = 0.2f;
				break;
		}
		state = WALKING;
		stateTimer = 0.0f;
	}

	void Update(Vector2 playerPos, float listenerRot, float dt) {
		Vector2 toPlayer = Vector2Subtract(playerPos, position);
		float distance = Vector2Length(toPlayer);
		Vector2 dir = Vector2Normalize(toPlayer);

		stateTimer += dt;

		switch (state) {
			case WALKING:
				if (distance > attackRange) {
					position = Vector2Add(position, Vector2Scale(dir, speed * dt));
				} else {
					state = WINDUP;
					stateTimer = 0.0f;
					attackDir = dir;
				}
				break;

			case WINDUP:
				if (stateTimer >= windupTime) {
					state = ATTACKING;
					stateTimer = 0.0f;
				}
				break;

			case ATTACKING:
				if (stateTimer >= attackTime) {
					state = WALKING;
					stateTimer = 0.0f;
				}
				break;
		}

		if (!IsSoundPlaying(sound)) PlaySound(sound);
		SetSoundPosition2D(playerPos, listenerRot, sound, position, 100.0f);
	}

	void Draw() {
		DrawCircleV(position, 10, color);

		if (state == WINDUP || state == ATTACKING) {
			Vector2 boxCenter = Vector2Add(position, Vector2Scale(attackDir, 25));
			float boxSize = 20;

			Color hitboxColor = (state == WINDUP) ? (Color){255, 255, 0, 100}   // yellow
												  : (Color){255, 0, 0, 120};    // red

			DrawRectangleV(
				Vector2Subtract(boxCenter, {boxSize / 2, boxSize / 2}),
				{ boxSize, boxSize },
				hitboxColor
			);
		}
	}

	~Enemy() {
		UnloadSound(sound);
	}
};


int main() {
	const int screenWidth = 1280;
	const int screenHeight = 720;

	InitWindow(screenWidth, screenHeight, "WAAAAAAGH!!!");
	InitAudioDevice();
	SetTargetFPS(60);
	DisableCursor();

	Player player = { { screenWidth / 2.0f, screenHeight / 2.0f } };

	// Different enemy types
	Enemy orc({ 600, 400 }, ORC);
	Enemy skeleton({ 800, 500 }, SKELETON);

	while (!WindowShouldClose()) {
		float dt = GetFrameTime();

		player.Update(dt);

		orc.Update(player.position, player.rotation, dt);
		skeleton.Update(player.position, player.rotation, dt);

		BeginDrawing();
		ClearBackground(BLACK);

		player.Draw();
		orc.Draw();
		skeleton.Draw();

		DrawText("WS to move, AD to rotate", 20, 20, 20, GRAY);
		EndDrawing();
	}

	CloseAudioDevice();
	CloseWindow();
	return 0;
}

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

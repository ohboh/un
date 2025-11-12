#include "raylib.h"
#include "raymath.h"
#include <vector>
using namespace std;

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
		Vector2 move = { 0, 0 };

		if (tankControls) {
			if (IsKeyDown(KEY_A)) rotation -= rotSpeed * dt;
			if (IsKeyDown(KEY_D)) rotation += rotSpeed * dt; 

			if (IsKeyDown(KEY_W)) move = Vector2Add(move, forward);
			if (IsKeyDown(KEY_S)) move = Vector2Subtract(move, forward);
			if (IsKeyDown(KEY_Q)) move = Vector2Subtract(move, right);
			if (IsKeyDown(KEY_E)) move = Vector2Add(move, right);
		}
		else {
			rotation += GetMouseDelta().x * sensitivity;

			if (IsKeyDown(KEY_W)) move = Vector2Add(move, forward);
			if (IsKeyDown(KEY_S)) move = Vector2Subtract(move, forward);
			if (IsKeyDown(KEY_A)) move = Vector2Subtract(move, right);
			if (IsKeyDown(KEY_D)) move = Vector2Add(move, right);
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

struct SoundManager {
	Sound orcWalk, orcWindup, orcAttack;
	Sound skeletonWalk, skeletonWindup, skeletonAttack;

	void Load() {
		orcWalk  = LoadSound("resources/orc_walk.wav");
		orcWindup = LoadSound("resources/orc_windup.wav");
		orcAttack = LoadSound("resources/orc_attack.wav");

		skeletonWalk  = LoadSound("resources/skeleton_walk.wav");
		skeletonWindup = LoadSound("resources/skeleton_windup.wav");
		skeletonAttack = LoadSound("resources/skeleton_attack.wav");
	}

	void Unload() {
		UnloadSound(orcWalk);
		UnloadSound(orcWindup);
		UnloadSound(orcAttack);
		UnloadSound(skeletonWalk);
		UnloadSound(skeletonWindup);	
		UnloadSound(skeletonAttack);
	}
};

struct Enemy {
	enum EnemyType { ORC, SKELETON };
	enum State { WALKING, WINDUP, ATTACKING } state;

	Vector2 position;
	Color color;
	float speed;
	float attackRange;

	float stateTimer;
	float windupTime;
	float attackTime;
	Vector2 attackDir;

	Sound walkSound;
	Sound windupSound;
	Sound attackSound;

	Sound currentSound;


	Enemy(Vector2 pos, EnemyType type, SoundManager &sm) {
		position = pos;
		switch (type) {
			case ORC:
				walkSound = LoadSoundAlias(sm.orcWalk);
				windupSound = LoadSoundAlias(sm.orcWindup);
				attackSound = LoadSoundAlias(sm.orcAttack);
				color = GREEN;
				speed = 50.0f;
				attackRange = 40.0f;
				windupTime = 0.7f;
				attackTime = 0.4f;
				break;
			case SKELETON:
				walkSound = LoadSoundAlias(sm.skeletonWalk);
				windupSound = LoadSoundAlias(sm.skeletonWindup);
				attackSound = LoadSoundAlias(sm.skeletonAttack);
				color = GRAY;
				speed = 80.0f;
				attackRange = 35.0f;
				windupTime = 0.4f;
				attackTime = 0.2f;
				break;
		}
		state = WALKING;
		stateTimer = 0.0f;
		currentSound = walkSound;
	}


	void Update(Vector2 playerPos, float listenerRot, float dt) {
		Vector2 toPlayer = Vector2Subtract(playerPos, position);
		float distance = Vector2Length(toPlayer);
		Vector2 dir = Vector2Normalize(toPlayer);

		stateTimer += dt;

		// Handle state
		switch (state) {
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
		}

		// Play the sound if it's not playing
		if (!IsSoundPlaying(currentSound)) PlaySound(currentSound);

		SetSoundPosition2D(playerPos, listenerRot, currentSound, position, 50.0f);
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
};

int main() {
	const int screenWidth = 1280;
	const int screenHeight = 720;

	InitWindow(screenWidth, screenHeight, "WAAAAAAGH!!!");
	InitAudioDevice();
	SetTargetFPS(60);
	DisableCursor();

	Player player = { { screenWidth / 2.0f, screenHeight / 2.0f } };
	SoundManager sm;
	sm.Load();

	vector<Enemy> enemies;

	enemies.emplace_back(Vector2{600, 400}, Enemy::ORC, sm);
	enemies.emplace_back(Vector2{800, 500}, Enemy::SKELETON, sm);

	while (!WindowShouldClose()) {
		float dt = GetFrameTime();

		player.Update(dt);

		for (auto &e : enemies) {
    		e.Update(player.position, player.rotation, dt);
		}

		if (IsKeyPressed(KEY_SPACE)) { // debug: spawn enemy
    		Vector2 spawnPos = { (float)GetRandomValue(100, screenWidth-100), (float)GetRandomValue(100, screenHeight-100) };
    		Enemy::EnemyType type = (GetRandomValue(0, 1) == 0) ? Enemy::ORC : Enemy::SKELETON;
    		enemies.emplace_back(spawnPos, type, sm);
		}


		BeginDrawing();
		ClearBackground(BLACK);

		player.Draw();
		for (auto &e : enemies) {
    		e.Draw();
		}


		DrawText("WS to move, AD to rotate", 20, 20, 20, GRAY);
		EndDrawing();
	}
	sm.Unload();
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

#include <raylib.h>

class Player{
public:
    float x, y;
    int speed;
    int radius;

    void Draw()
    {
        DrawCircle(x, y, radius, WHITE);
    }

    void Update()
    {        
        if (IsKeyDown(KEY_UP))
        {
            y -= speed;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            y += speed;
        }
        if (IsKeyDown(KEY_LEFT))
        {
            x -= speed;
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            x += speed;
        }
    }
};

Player player;

int main(void) 
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;
        
    InitWindow(screenWidth, screenHeight, "un");
    SetTargetFPS(60);

    player.radius = 20;
    player.speed = 5;
    player.x = screenWidth/2;
    player.y = screenHeight/2;
    
    while (!WindowShouldClose())
    {
        BeginDrawing();
        player.Update();
        player.Draw();
        ClearBackground(BLACK);
        EndDrawing();
    }
    
    CloseWindow();
}
#include "raylib.h"
#include "raymath.h"


#include "types.h"
#include "ball.h"
#include "level.h"
#include "input.h"
#include "putter.h"
#include "render.h"


#define STROKE_LIMIT_OVER_PAR 5
#define HOLE_DONE_PAUSE 1.60f


typedef enum {   
    SCREEN_MAIN_MENU,
    SCREEN_GAME,
    SCREEN_TUTORIAL,
    SCREEN_ABOUT
} ScreenState;


typedef struct {
    GameStateId state;
    Course course;
    Ball ball;
    Putter putter;
    InputSystem input_sys;
    Edtior editor;
    RenderState render;
    int scores[MAX_HOLES];
    int best[MAX_HOLES];
    float hole_done_t;
    float prev_speed;
    
    //Menu variables
    ScreenState screen;
    Texture2D menuBackground;
    Texture2D tutorialImage;
    Texture2D aboutImage;
    Texture2D windSprite;
    Music menuMusic;
    Music gameMusic;
    bool muted;
} GameApp;


void DrawFullscreenTexture(Texture2D texture)
{
    Rectangle source = {0,0,(float)texture.width,(float)texture.height};
    Rectangle destination = {0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};
    Vector2 origin = {0,0};
    DrawTexturePro(texture,source,destination,origin,0.0f,WHITE);
}


bool DrawMenuButton(Rectangle rect,const char *text)
{
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse,rect);

    Color buttonColor;
    if (hovered) buttonColor = (Color){70,120,70,230};
    else buttonColor = (Color){30,50,30,210};

    DrawRectangleRounded(rect,0.20f,10,buttonColor);
    DrawRectangleRoundedLines(rect,0.20f,10,(Color){220,220,180,255});

    int fontSize = 30;
    int textWidth = MeasureText(text,fontSize);
    DrawText(text,(int)(rect.x + rect.width/2 - textWidth/2),(int)(rect.y + rect.height/2 - fontSize/2),fontSize,WHITE);
    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return true;
    return false;
}


void StartHole(GameApp *g)
{
    BallInit(&g->ball,course_current(&g->course)->tee_pos);
    putter_init(&g->putter);
    g->render.hole_time = 0.0f;
    g->render.cam.target = g->ball.pos;
    g->state = GS_PLAYING;
}


void StartGameMusic(GameApp *g)
{
    StopMusicStream(g->menuMusic);
    if (!g->muted) PlayMusicStream(g->gameMusic);
}


void StartMenuMusic(GameApp *g)
{
    StopMusicStream(g->gameMusic);
    if (!g->muted) PlayMusicStream(g->menuMusic);
}


void ToggleMute(GameApp *g)
{
    g->muted = !g->muted;

    if (g->muted) {
    SetMusicVolume(g->menuMusic,0.0f);
    SetMusicVolume(g->gameMusic,0.0f);
    } 
    else {
    SetMusicVolume(g->menuMusic,0.45f);
    SetMusicVolume(g->gameMusic,0.35f);
    }
}


void GameInit(GameApp *g)
{
    *g = (GameApp){0};

    course_load(&g->course,"levels");

    for (int i = 0;i < MAX_HOLES;i++) g->scores[i] = 0;

    InputInit(&g->input_sys);
    EditorInit(&g->editor);
    BallInit(&g->ball,course_current(&g->course)->tee_pos);
    putter_init(&g->putter);
    RenderInit(&g->render,g->ball.pos);
    g->state = GS_PLAYING;
    g->screen = SCREEN_MAIN_MENU;
    g->muted = false;

    g->menuBackground = LoadTexture("menu.png");
    g->tutorialImage = LoadTexture("tutorials.png");
    g->aboutImage = LoadTexture("GolfGameCredit.png");
    g->windSprite = LoadTexture("wind_sprite.png");
    SetTextureFilter(g->windSprite,TEXTURE_FILTER_BILINEAR);
    g->menuMusic = LoadMusicStream("golfmenu.mp3");
    g->gameMusic = LoadMusicStream("gamemusic.mp3");
    g->menuMusic.looping = true;
    g->gameMusic.looping = true;
    SetMusicVolume(g->menuMusic,0.45f);
    SetMusicVolume(g->gameMusic,0.35f);
    PlayMusicStream(g->menuMusic);
}


bool DrawMainMenu(GameApp *g)
{
    DrawFullscreenTexture(g->menuBackground);
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),(Color){0,0,0,70});
    const char *title = "MINI GOLF";
    int titleFontSize = 70;
    int titleWidth = MeasureText(title,titleFontSize);
    DrawText(title,GetScreenWidth()/2 - titleWidth/2,80,titleFontSize,WHITE);
    float buttonWidth = 260.0f;
    float buttonHeight = 60.0f;
    float centerX = GetScreenWidth()/2.0f - buttonWidth/2.0f;
    Rectangle playButton = {centerX,220,buttonWidth,buttonHeight};
    Rectangle tutorialButton = {centerX,300,buttonWidth,buttonHeight};
    Rectangle aboutButton = {centerX,380,buttonWidth,buttonHeight};
    Rectangle soundButton = {centerX,460,buttonWidth,buttonHeight};
    Rectangle exitButton = {centerX,540,buttonWidth,buttonHeight};
    if (DrawMenuButton(playButton,"PLAY")) {
    g->screen = SCREEN_GAME;
    StartGameMusic(g);
    }
    if (DrawMenuButton(tutorialButton,"TUTORIAL")) g->screen = SCREEN_TUTORIAL;
    if (DrawMenuButton(aboutButton,"ABOUT US")) g->screen = SCREEN_ABOUT;
    const char *soundText;
    if (g->muted) soundText = "SOUND: OFF";
    else soundText = "SOUND: ON";
    if (DrawMenuButton(soundButton,soundText)) ToggleMute(g);
    if (DrawMenuButton(exitButton,"EXIT")) return true;
    return false;
}


void DrawTutorialScreen(GameApp *g)
{
    DrawFullscreenTexture(g->tutorialImage);
    Rectangle backButton = {30,30,160,55};
    if (DrawMenuButton(backButton,"BACK")) g->screen = SCREEN_MAIN_MENU;
}


void DrawAboutScreen(GameApp *g)
{
    DrawFullscreenTexture(g->aboutImage);
    Rectangle backButton = {30,30,160,55};
    if (DrawMenuButton(backButton,"BACK")) g->screen = SCREEN_MAIN_MENU;
}


int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);


    InitWindow(1280,720,"Mini Golf By Akif Rafin");
    InitAudioDevice();
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    GameApp game;
    GameInit(&game);
    bool shouldQuit = false;


    while (!WindowShouldClose() && !shouldQuit) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;
        UpdateMusicStream(game.menuMusic);
        UpdateMusicStream(game.gameMusic);
        if (game.screen == SCREEN_MAIN_MENU) {
        BeginDrawing();
        ClearBackground(BLACK);
        shouldQuit = DrawMainMenu(&game);
        EndDrawing();
        continue;
        }
        if (game.screen == SCREEN_TUTORIAL) {
        if (IsKeyPressed(KEY_ESCAPE)) game.screen = SCREEN_MAIN_MENU;
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTutorialScreen(&game);
        EndDrawing();
        continue;
        }
        if (game.screen == SCREEN_ABOUT) {
        if (IsKeyPressed(KEY_ESCAPE)) game.screen = SCREEN_MAIN_MENU;
        BeginDrawing();
        ClearBackground(BLACK);
        DrawAboutScreen(&game);
        EndDrawing();
        continue;
        }
        Hole *hole = course_current(&game.course);
        InputState in = InputPoll(&game.input_sys,game.ball.pos,game.render.cam,dt);
        if (IsKeyPressed(KEY_ESCAPE)) {
        game.screen = SCREEN_MAIN_MENU;
        StartMenuMusic(&game);
        continue;
        }
        switch (game.state) {
        case GS_PLAYING:
            if (IsKeyPressed(KEY_R)) {
            game.ball.pos = hole->tee_pos;
            game.ball.vel = Vector2Zero();
            game.ball.state = BALL_AIM;
            }
            putter_update(&game.putter,&in,&game.ball,dt);
            BallUpdate(&game.ball,hole,dt);
            if (game.ball.state == BALL_SUNK) {
            game.scores[game.course.current] = game.ball.strokes;
            game.hole_done_t = 0.0f;
            game.state = GS_HOLE_DONE;
            int idx = game.course.current;
            if (game.best[idx] == 0 || game.ball.strokes < game.best[idx]) game.best[idx] = game.ball.strokes;
            TraceLog(LOG_INFO,"hole %d: %d strokes (par %d) -- %s",hole->number,game.ball.strokes,hole->par,score_name(game.ball.strokes,hole->par));
            } else if (game.ball.state == BALL_AIM && game.ball.strokes >= hole->par + STROKE_LIMIT_OVER_PAR) {
            game.scores[game.course.current] = hole->par + STROKE_LIMIT_OVER_PAR;
            game.hole_done_t = 0.0f;
            game.state = GS_HOLE_DONE;
            }
            break;
        case GS_HOLE_DONE:
            game.hole_done_t += dt;
            if (game.hole_done_t >= HOLE_DONE_PAUSE || in.confirm) {
            if (course_advance(&game.course)) StartHole(&game);
            else game.state = GS_SCOREBOARD;
            }
            break;
        case GS_SCOREBOARD:
            if (in.confirm) {
            game.course.current = 0;
            for (int i = 0;i < MAX_HOLES;i++) game.scores[i] = 0;
            StartHole(&game);
            }
            break;
        default:
            break;
        }

        RenderUpdateCamera(&game.render,&game.ball,hole,&game.putter,dt);


        BeginDrawing();
        ClearBackground((Color){92,92,56,255});
        BeginMode2D(game.render.cam);
        DrawCourse(hole);
        DrawWindZones(hole,game.windSprite,(float)GetTime());
        DrawBall(&game.ball);
        DrawRails(hole);
        if (game.state == GS_PLAYING && game.ball.state == BALL_AIM) DrawAimGuide(&game.ball,hole,in.aim_angle,game.putter.power);
        putter_draw(&game.putter,&game.ball);
        EndMode2D();
        DrawHUD(hole,&game.ball,&game.putter,game.course.current,game.course.hole_count,Course_total(game.scores,game.course.hole_count));
        if (game.state == GS_HOLE_DONE) {
        const char *msg = score_name(game.scores[game.course.current],hole->par);
        int tw = MeasureText(msg,54);
        DrawText(msg,GetScreenWidth()/2 - tw/2,GetScreenHeight()/2 - 40,54,(Color){255,214,102,255});
        }
        EndDrawing();
    }

    StopMusicStream(game.menuMusic);
    StopMusicStream(game.gameMusic);
    UnloadMusicStream(game.menuMusic);
    UnloadMusicStream(game.gameMusic);
    UnloadTexture(game.menuBackground);
    UnloadTexture(game.tutorialImage);
    UnloadTexture(game.aboutImage);
    UnloadTexture(game.windSprite);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
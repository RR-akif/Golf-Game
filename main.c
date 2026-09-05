/* ===========================================================================
   main.c -- the window, the loop and the top-level state machine.  [SHARED]

   This file owns nothing except the ORDER of operations. Both people may
   edit it, but only to add a line to one of the numbered sections below,
   and you tell your partner when you do.
   ======================================================================== */
#include "raylib.h"
#include "raymath.h"

#include "types.h"
#include "ball.h"
#include "level.h"
#include "input.h"
#include "putter.h"
#include "render.h"

#define STROKE_LIMIT_OVER_PAR 5
#define HOLE_DONE_PAUSE       1.6f

typedef struct {
    GameStateId state;
    Course      course;
    Ball        ball;
    Putter      putter;
    InputSystem input_sys;
    Edtior      editor;
    RenderState render;
    int         scores[MAX_HOLES];
    int         best[MAX_HOLES];      /* best score per hole, persisted */
    float       hole_done_t;
    float       prev_speed;           /* to detect an impact this frame */
} GameApp;

static void StartHole(GameApp *g)
{
    BallInit(&g->ball, course_current(&g->course)->tee_pos);
    putter_init(&g->putter);
    g->render.hole_time = 0.0f;
    g->render.cam.target = g->ball.pos;
    g->state = GS_PLAYING;
}

static void GameInit(GameApp *g)
{
    *g = (GameApp){ 0 };
    course_load(&g->course, "levels");
    for (int i = 0; i < MAX_HOLES; i++) g->scores[i] = 0;

    InputInit(&g->input_sys);
    EditorInit(&g->editor);
    BallInit(&g->ball, course_current(&g->course)->tee_pos);
    putter_init(&g->putter);
    RenderInit(&g->render, g->ball.pos);
    g->state = GS_PLAYING;
}

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Mini Golf");
    SetTargetFPS(60);

    GameApp game;
    GameInit(&game);

    while (!WindowShouldClose())
    {
        /* ---- 1. time, clamped so one slow frame cannot teleport the ball */
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;

        Hole *hole = course_current(&game.course);

        /* ---- 2. input (the ONLY place hardware is read) ---------------- */
        InputState in = InputPoll(&game.input_sys, game.ball.pos,
                                  game.render.cam, dt);

        /* ---- 3. update, by state --------------------------------------- */
        switch (game.state)
        {
        case GS_PLAYING:
            if (IsKeyPressed(KEY_R)) {          /* reset, keeping the score */
                game.ball.pos = hole->tee_pos;
                game.ball.vel = Vector2Zero();
                game.ball.state = BALL_AIM;
            }

            putter_update(&game.putter, &in, &game.ball, dt);
            BallUpdate(&game.ball, hole, dt);

            if (game.ball.state == BALL_SUNK) {
                game.scores[game.course.current] = game.ball.strokes;
                game.hole_done_t = 0.0f;
                game.state = GS_HOLE_DONE;

                /* record a new best for this hole */
                int idx = game.course.current;
                if (game.best[idx] == 0 || game.ball.strokes < game.best[idx]) {
                    game.best[idx] = game.ball.strokes;
                }
                TraceLog(LOG_INFO, "hole %d: %d strokes (par %d) -- %s",
                         hole->number, game.ball.strokes, hole->par,
                         score_name(game.ball.strokes, hole->par));
            }
            else if (game.ball.state == BALL_AIM &&
                     game.ball.strokes >= hole->par + STROKE_LIMIT_OVER_PAR) {
                game.scores[game.course.current] =
                    hole->par + STROKE_LIMIT_OVER_PAR;      /* picked up */
                game.hole_done_t = 0.0f;
                game.state = GS_HOLE_DONE;
            }
            break;

        case GS_HOLE_DONE:
            game.hole_done_t += dt;
            if (game.hole_done_t >= HOLE_DONE_PAUSE || in.confirm) {
                if (course_advance(&game.course)) StartHole(&game);
                else                              game.state = GS_SCOREBOARD;
            }
            break;

        case GS_SCOREBOARD:
            if (in.confirm) {                    /* play again */
                game.course.current = 0;
                for (int i = 0; i < MAX_HOLES; i++) game.scores[i] = 0;
                StartHole(&game);
            }
            break;

        default:
            break;
        }

        /* ---- 4. camera ------------------------------------------------- */
        RenderUpdateCamera(&game.render, &game.ball, hole, &game.putter, dt);

        /* ---- 5. draw --------------------------------------------------- */
        BeginDrawing();
            ClearBackground((Color){ 92, 92, 56, 255 });

            BeginMode2D(game.render.cam);          /* --- world space --- */
                DrawCourse(hole);
                DrawBall(&game.ball);
                DrawRails(hole);
                if (game.state == GS_PLAYING &&
                    game.ball.state == BALL_AIM)
                    DrawAimGuide(&game.ball, hole, in.aim_angle,
                                 game.putter.power);
                putter_draw(&game.putter, &game.ball);
            EndMode2D();                           /* --- screen space --- */

            DrawHUD(hole, &game.ball, &game.putter,
                    game.course.current, game.course.hole_count,
                    Course_total(game.scores, game.course.hole_count));

            if (game.state == GS_HOLE_DONE) {
                const char *msg = score_name(game.scores[game.course.current],
                                             hole->par);
                int tw = MeasureText(msg, 54);
                DrawText(msg, GetScreenWidth()/2 - tw/2,
                         GetScreenHeight()/2 - 40, 54,
                         (Color){ 255, 214, 102, 255 });
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

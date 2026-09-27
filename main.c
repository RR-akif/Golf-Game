#include<stdio.h>
#include<string.h>

#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include "ball.h"
#include "level.h"
#include "input.h"
#include "putter.h"
#include "render.h"

#define MAX_PLAYERS 100
#define MAX_NAME_LENGTH 30
#define LEADERBOARD_FILE "leaderboard.txt" //For showing leaderboard including their name
#define STROKE_LIMIT_OVER_PAR 5 //If this limit is crossed then automatic the game shifts to next level
#define HOLE_DONE_PAUSE 1.60f //pause time when one hole is completed(show birdir , par or else....)

typedef struct
{
    char name[MAX_NAME_LENGTH];
    int score;
} PlayerScore;


typedef enum {   
    SCREEN_MAIN_MENU,
    SCREEN_NAME_INPUT,
    SCREEN_GAME,
    SCREEN_TUTORIAL,
    SCREEN_LEADERBOARD,
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
    int best[MAX_HOLES]; //best scores per hole individually
    int bestCourseScore; //to show best score after the entire game (course - total combination of 12 holes)
    bool newBestScore; //to determine whether the current player created  a new record or not
    float hole_done_t; //After a hole is completed it is set to zero ,then added with dt determine when this value crosses 1.6 , then new hole will be appeared.
    float prev_speed;
    
    //Menu variables
    ScreenState screen;
    Texture2D menuBackground;
    Texture2D tutorialImage;
    Texture2D aboutImage;
    Texture2D windSprite;
    Music menuMusic;
    Music gameMusic;

    Sound waterSound;
    Sound wallSound;
    Sound cupSound;//These are short sound effects , so datatype is sound.
    Sound putterSound;
    bool muted;

    //LeaderBoard section
    char playerName[MAX_NAME_LENGTH];
    int playerNameLength;

    PlayerScore leaderboard[MAX_PLAYERS];
    int leaderboardCount;
} GameApp;


// Function prototypes
int LoadBestScore(void);
void SaveBestScore(int score);
void CheckFinalScore(GameApp *g);
void RestartGame(GameApp *g);
void ResetGameProgress(GameApp *g);
int DrawFinalScoreScreen(GameApp *g);
void StartHole(GameApp *g);
void StartGameMusic(GameApp *g);
void StartMenuMusic(GameApp *g);
//Leaderboard related functions
void LoadLeaderboard(GameApp *g);
void SaveLeaderboard(GameApp *g);
void UpdateLeaderboard(GameApp *g,int score);
void SortLeaderboard(GameApp *g);
void DrawNameInputScreen(GameApp *g);
void DrawLeaderboardScreen(GameApp *g);


void DrawFullscreenTexture(Texture2D texture)
{
    Rectangle source = {0,0,(float)texture.width,(float)texture.height}; //What portion of the image i want to draw
    Rectangle destination = {0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};
    Vector2 origin = {0,0};
    DrawTexturePro(texture,source,destination,origin,0.0f,WHITE); //Source is used to select a certain portion from our texture , destination is our  screen at which plavce we are going to place it.Origin is the mid of the screen , and next parameter is the rotation of our texture . The last parameter basically is multiplied by the original color of the texture giving a white tint., White--dont change the original color
}


bool DrawMenuButton(Rectangle rect,const char *text)
{
    Vector2 mouse=GetMousePosition(); //get the exact coordinate of the mouse  (point)
    bool hovered=CheckCollisionPointRec(mouse,rect); //detecting whether the mouse is at the position of the rectangle or it is colliding with the rectangle

    Color buttonColor;
    if (hovered) buttonColor=(Color){70,120,70,230};
    else buttonColor=(Color){30,50,30,250};

    DrawRectangleRounded(rect,0.20f,10,buttonColor);
    DrawRectangleRoundedLines(rect,0.20f,10,(Color){220,220,180,255});

    int fontSize=30;
    int textWidth=MeasureText(text,fontSize); //Returns the width of the text in pixel
    DrawText(text,(int)(rect.x + rect.width/2 - textWidth/2),(int)(rect.y + rect.height/2-fontSize/2),fontSize,WHITE); //Treating the fontsize as the text height. Drawing the text at the middle of the rectangle

    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return true;
    return false;
}


void StartHole(GameApp *g)
{
    BallInit(&g->ball,course_current(&g->course)->tee_pos); //ballinit function receives a pointer , so we passed the address. 2nd parameter: place the ball at the tee position of the current course.
    putter_init(&g->putter);
    g->render.hole_time=0.0f;
    g->render.cam.target=g->ball.pos;
    g->state=GS_PLAYING;
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


void ToggleMute(GameApp *g) //This function will be called when sound button is pressed
{
    g->muted = !g->muted; //Toggling sound

    if (g->muted) {
        SetMusicVolume(g->menuMusic,0.0f);
        SetMusicVolume(g->gameMusic,0.0f);
    } 
    else {
        SetMusicVolume(g->menuMusic,0.6f);
        SetMusicVolume(g->gameMusic,0.45f);
    }
}

#define BEST_SCORE_FILE "best_score.txt"

int LoadBestScore(void)
{
    FILE *file=fopen(BEST_SCORE_FILE,"r"); //this opens the file in read mode
    if(file==NULL)
    {
        return -1;
    }
    int bestScore = -1; //default value
    if(fscanf(file,"%d",&bestScore)!=1) //It attempts to read the integer value from the file and stores it inside bestscore, if the file contains 42, then bestscore becomes 42, if it can read successfully then it returns 1, else -1.
    {
        bestScore=-1; //the above condition returns 1 if it is read successfully
    }
    fclose(file);
    return bestScore;
}


void SaveBestScore(int score)
{
    FILE*file =fopen(BEST_SCORE_FILE,"w"); //creates or overwrites the file
    if(file==NULL)
    {
        return;
    }
    fprintf(file,"%d",score);
    fclose(file);
}

void LoadLeaderboard(GameApp *g)
{
    g->leaderboardCount=0;
    FILE *file=fopen(LEADERBOARD_FILE,"r");
    if(file==NULL)
    {
        return;
    }
    while(g->leaderboardCount<MAX_PLAYERS)
    {
        char name[MAX_NAME_LENGTH];
        int score;
        if(fscanf(file,"%29s %d",name,&score)!=2) //This function reads character name and his score and stores inside name , score
        {
            break;
        }
        strcpy(g->leaderboard[g->leaderboardCount].name,name); //storing players name in the corresponding array
        g->leaderboard[g->leaderboardCount].score=score;

        g->leaderboardCount++;
    }
    fclose(file);
}

void SaveLeaderboard(GameApp *g)
{
    FILE *file=fopen(LEADERBOARD_FILE,"w");
    if(file==NULL)
    {
        return;
    }
    for(int i=0;i<g->leaderboardCount;i++)
    {
        fprintf(file,"%s %d\n",g->leaderboard[i].name,g->leaderboard[i].score); //writing the name and his score inside the .txt file
    }
    fclose(file);
}


void SortLeaderboard(GameApp *g)
{
    for(int i=0;i<g->leaderboardCount-1;i++)
    {
        for(int j=i+1;j<g->leaderboardCount;j++)
        {
            if(g->leaderboard[j].score<g->leaderboard[i].score)
            {
                PlayerScore temp=g->leaderboard[i];
                g->leaderboard[i]=g->leaderboard[j];
                g->leaderboard[j]=temp;
            }
        }
    }
}

void UpdateLeaderboard(GameApp *g,int score)
{
    int playerIndex=-1;
    for(int i=0;i<g->leaderboardCount;i++)
    {
        if(strcmp(g->leaderboard[i].name,g->playerName)==0)
        {
            playerIndex=i; //When same player plays two different times , then i have to replace his older score with new only if new<old, so toi do that we have to find out the index of the same palyer, thats what we did here.
            break;
        }
    }
    if(playerIndex>=0) //current plyer name already exists inside the file
    {
        if(score<g->leaderboard[playerIndex].score)
        {
            g->leaderboard[playerIndex].score=score; //updating the same players value with new value (if new<old)
        }
    }
    else //players name doesnt exit , so we will just add his name and score in the next position.
    {
        if(g->leaderboardCount<MAX_PLAYERS)
        {
            strcpy(g->leaderboard[g->leaderboardCount].name,g->playerName);
            g->leaderboard[g->leaderboardCount].score=score;
            g->leaderboardCount++;
        }
    }
    SortLeaderboard(g);
    SaveLeaderboard(g);
}


void CheckFinalScore(GameApp *g)
{
    int ownScore = Course_total(g->scores,g->course.hole_count); //this function calculates the total score
    g->newBestScore = false;

    if(g->bestCourseScore<0) //g->bestCourseScore=LoadBestScore() inside game init , for the first gameplay the .txt didnt exist , so this integer value will be -1 which is less than zero . So we deliberately put ownscore value inside that .txt file.
    {
        g->bestCourseScore=ownScore;
        g->newBestScore=true;
        SaveBestScore(g->bestCourseScore);
    }
    else if(ownScore<g->bestCourseScore)
    {
        g->bestCourseScore=ownScore;
        g->newBestScore=true;
        SaveBestScore(g->bestCourseScore);
    }
    UpdateLeaderboard(g,ownScore);
}


void RestartGame(GameApp *g)
{
    g->course.current=0; //go back to the first hole.
    for(int i=0;i<MAX_HOLES;i++)
    {
        g->scores[i]=0; //set scores for all holes to zero.
    }
    g->newBestScore=false;
    StartHole(g);
}


void ResetGameProgress(GameApp *g)
{
    g->course.current=0;
    for(int i=0;i<MAX_HOLES;i++)
    {
        g->scores[i]=0;
    }
    g->newBestScore=false;
    BallInit(&g->ball,course_current(&g->course)->tee_pos);
    putter_init(&g->putter);
    g->state=GS_PLAYING;
}


int DrawFinalScoreScreen(GameApp *g)
{
    int sw=GetScreenWidth();
    int sh=GetScreenHeight();
    int ownScore=Course_total(g->scores,g->course.hole_count); //This function calculates total score(sum of the array)

    ClearBackground((Color){10,18,14,255});
    DrawRectangleGradientH(0,0,sw,sh,(Color){18,42,30,255},(Color){5,12,10,255}); //Alters the color horizontally from left to right 
    if(g->newBestScore)
    {
        const char *title="CONGRATULATIONS!";
        int titleSize = 52;
        DrawText(title,sw/2-MeasureText(title,titleSize)/2,100,titleSize,GOLD);
        const char *message="You made the best score!";
        DrawText(message,sw/2-MeasureText(message,30)/2,175,30,RAYWHITE);
    }
    else
    {
        const char *title="COURSE COMPLETE";
        DrawText(title,sw/2-MeasureText(title,52)/2,110,52,RAYWHITE);
    }

    char ownText[64];
    char bestText[64];
    snprintf(ownText,sizeof(ownText),"Your Score: %d",ownScore); //snprintf(destination-where to store , max size of that array , format of text , replacing values - here ownscore is the replacing value of %d)
    snprintf(bestText,sizeof(bestText),"Best Score: %d",g->bestCourseScore); //snprintf() is used to store the formatted text inside a string/char array
    DrawText(ownText,sw/2-MeasureText(ownText,34)/2,250,34,RAYWHITE);
    DrawText(bestText,sw/2-MeasureText(bestText,34)/2,300,34,RAYWHITE);
    float buttonWidth=220.0f;
    float buttonHeight=60.0f;

    float gap=30.0f;
    float totalWidth =buttonWidth*3 + gap*2; //Total allocated space for 3 consecutive buttons (horizontally placed)
    float startX=sw/2.0f-totalWidth/2.0f;
    float buttonY=420.0f;

    Rectangle menuButton={startX,buttonY,buttonWidth,buttonHeight};
    Rectangle restartButton = {startX+buttonWidth+gap,buttonY,buttonWidth,buttonHeight};
    Rectangle exitButton = {startX+(buttonWidth + gap)*2,buttonY,buttonWidth,buttonHeight};

    if(DrawMenuButton(menuButton,"MENU"))
    {
        ResetGameProgress(g);
        g->screen=SCREEN_MAIN_MENU;
        StartMenuMusic(g);
    }
    if(DrawMenuButton(restartButton,"RESTART"))
    {
        HideCursor();
        RestartGame(g);
    }
    if(DrawMenuButton(exitButton,"EXIT"))
    {
        return 1;
    }
    return 0;
}


void GameInit(GameApp *g)
{
    *g = (GameApp){0};
    course_load(&g->course,"levels");
    for (int i = 0;i < MAX_HOLES;i++) g->scores[i] = 0;

    g->bestCourseScore=LoadBestScore();
    g->newBestScore = false;
    
    g->playerName[0]='\0';
    g->playerNameLength=0;
    g->leaderboardCount=0;

    LoadLeaderboard(g);
    SortLeaderboard(g);

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
    g->aboutImage = LoadTexture("Credits.png");
    g->windSprite = LoadTexture("wind_sprite.png");
    SetTextureFilter(g->windSprite,TEXTURE_FILTER_BILINEAR);
    g->menuMusic = LoadMusicStream("golfmenu.mp3");
    g->gameMusic = LoadMusicStream("gamemusic.mp3");

    g->waterSound = LoadSound("water.mp3");
    g->wallSound = LoadSound("wallcollision.mp3");
    g->cupSound = LoadSound("ballInCup.mp3");
    g->putterSound = LoadSound("BallStrike.mp3");

    SetSoundVolume(g->waterSound,1.0f);
    SetSoundVolume(g->wallSound,1.0f);
    SetSoundVolume(g->cupSound,0.8f);
    SetSoundVolume(g->putterSound,1.00f);

    g->menuMusic.looping = true;
    g->gameMusic.looping = true; //music will be continued after ending
    SetMusicVolume(g->menuMusic,0.6f);
    SetMusicVolume(g->gameMusic,0.45f);
    PlayMusicStream(g->menuMusic);
}


bool DrawMainMenu(GameApp *g)
{
    DrawFullscreenTexture(g->menuBackground); //menu texture

    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),(Color){0,0,0,70});

    const char *title="MINI GOLF";
    int titleFontSize=70;
    int titleWidth=MeasureText(title,titleFontSize);
    DrawText(title,GetScreenWidth()/2-titleWidth/2,80,titleFontSize,WHITE); // drawtext receives a pointer(pre defined) , so we declared it as a pointer.

    float buttonWidth=260.0f;
    float buttonHeight=60.0f;
    float centerX=GetScreenWidth()/2.0f-buttonWidth/2.0f;

    Rectangle playButton={centerX,190,buttonWidth,buttonHeight};
    Rectangle leaderboardButton={centerX,265,buttonWidth,buttonHeight};
    Rectangle tutorialButton={centerX,340,buttonWidth,buttonHeight};
    Rectangle aboutButton={centerX,415,buttonWidth,buttonHeight};
    Rectangle soundButton={centerX,490,buttonWidth,buttonHeight};
    Rectangle exitButton={centerX,565,buttonWidth,buttonHeight};

    if (DrawMenuButton(playButton,"PLAY")) {
        g->playerName[0]='\0';
        g->playerNameLength=0;
        g->screen=SCREEN_NAME_INPUT;
    }
    if(DrawMenuButton(leaderboardButton,"LEADERBOARD"))
    {
        g->screen=SCREEN_LEADERBOARD;
    }
    if (DrawMenuButton(tutorialButton,"TUTORIAL")) g->screen = SCREEN_TUTORIAL;
    if (DrawMenuButton(aboutButton,"ABOUT US")) g->screen = SCREEN_ABOUT;

    const char *soundText;
    if (g->muted) soundText = "SOUND: OFF";
    else soundText = "SOUND: ON";
    if (DrawMenuButton(soundButton,soundText)) ToggleMute(g); //The function checks whether the button is pressed or not
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

void DrawNameInputScreen(GameApp *g)
{
    int sw=GetScreenWidth();
    int sh=GetScreenHeight();
    ClearBackground((Color){10,18,14,255});
    const char *title="ENTER YOUR NAME";
    int titleSize=50;

    DrawText(title,sw/2-MeasureText(title,titleSize)/2,140,titleSize,RAYWHITE);
    Rectangle inputBox={sw/2.0-250,250,500,70};
    DrawRectangleRounded(inputBox,0.15,10,BLACK);
    DrawRectangleRoundedLines(inputBox,0.15,10,RAYWHITE);
    DrawText(g->playerName,(int)inputBox.x+20,(int)inputBox.y+18,32,WHITE);
    int key=GetCharPressed(); //did the user type any character?

    while(key>0)
    {
        if(key>=32 && key<=126 && g->playerNameLength<MAX_NAME_LENGTH-1) 
        {
            g->playerName[g->playerNameLength]=(char)key;
            g->playerNameLength++;

            g->playerName[g->playerNameLength]='\0'; //after adding each character we are assigning null
        }
        key=GetCharPressed();
    }
    if(IsKeyPressed(KEY_BACKSPACE) && g->playerNameLength>0)
    {
        g->playerNameLength--;
        g->playerName[g->playerNameLength]='\0'; //When backspace is pressed immediate previous character is wiped out
    }

    const char *instruction="Press ENTER to start";
    DrawText(instruction,sw/2-MeasureText(instruction,25)/2,350,25,LIGHTGRAY);
    if(IsKeyPressed(KEY_ENTER) && g->playerNameLength>0)  
    {
        ResetGameProgress(g);
        g->screen=SCREEN_GAME;
        StartGameMusic(g);
        HideCursor();
    }
    Rectangle backButton={30,30,160,55};
    if(DrawMenuButton(backButton,"BACK"))
    {
        g->screen=SCREEN_MAIN_MENU;
    }
}


void DrawLeaderboardScreen(GameApp *g)
{
    int sw=GetScreenWidth();
    ClearBackground((Color){10,18,14,255});

    const char *title="LEADERBOARD";
    int titleSize=55;
    DrawText(title,sw/2-MeasureText(title,titleSize)/2,60,titleSize,GOLD);
    DrawText("RANK",220,150,25,RAYWHITE);
    DrawText("PLAYER",380,150,25,RAYWHITE);
    DrawText("SCORE",700,150,25,RAYWHITE);

    int y=200;
    int numberToShow=g->leaderboardCount;
    if(numberToShow>10)
    {
        numberToShow=10;
    }
    for(int i=0;i<numberToShow;i++)
    {
        char rankText[20];
        char scoreText[20];
        snprintf(rankText,sizeof(rankText),"%d",i+1); //Storing formating text inside the string 
        snprintf(scoreText,sizeof(scoreText),"%d",g->leaderboard[i].score);
        Color color=RAYWHITE;
        if(i==0)
        {
            color=GOLD;
        }
        DrawText(rankText,240,y,28,color);
        DrawText(g->leaderboard[i].name,380,y,28,color);
        DrawText(scoreText,720,y,28,color);
        y+=42;
    }
    if(g->leaderboardCount==0)
    {
        const char *empty="No scores recorded yet.";
        DrawText(empty,sw/2-MeasureText(empty,30)/2,300,30,LIGHTGRAY);
    }
    Rectangle backButton={30,30,160,55};
    if(DrawMenuButton(backButton,"BACK"))
    {
        g->screen=SCREEN_MAIN_MENU;
    }
}


int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(1280,720,"Mini Golf By Akif Rafin");
    SetExitKey(KEY_NULL); //Normally esc is assumed as exit key, so if we press exit at any time then it gets out of the game . But we want if esc is pressed then , it should return to our game menu, so exit key is set to be null.
    InitAudioDevice();
    SetTargetFPS(60);

    GameApp game;
    GameInit(&game);
    bool shouldQuit = false; //When player presses exit , then it becomes true


    while (!WindowShouldClose() && !shouldQuit) {
        float dt = GetFrameTime();

        UpdateMusicStream(game.menuMusic);
        UpdateMusicStream(game.gameMusic);//This is applied per frame

        if (game.screen == SCREEN_MAIN_MENU) {
            BeginDrawing();
            ClearBackground(BLACK);
            shouldQuit=DrawMainMenu(&game); //Suppose user presses play , when this function is called then it checks which function is called, for "play" it returns true so game_screen brcomes screen_game , game.screen==screen_main_menu becomes false and this menu block is not executed
            EndDrawing();
            continue; // If we dont use continue, then compiler will go downward and execute the rest task , and main game will be started, so we used continue , so that it moves to next iteration.
        }

        if(game.screen==SCREEN_NAME_INPUT)
        {
            BeginDrawing();
            ClearBackground(BLACK);
            DrawNameInputScreen(&game);
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

        if(game.screen==SCREEN_LEADERBOARD)
        {
            if(IsKeyPressed(KEY_ESCAPE))
            {
                game.screen=SCREEN_MAIN_MENU;
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawLeaderboardScreen(&game);
            EndDrawing();
            continue;
        }

        Hole *hole = course_current(&game.course); //get the current hole
        InputState in = InputPoll(&game.input_sys,game.ball.pos,game.render.cam,dt); //This collects current useer input and stores inside in

        if (IsKeyPressed(KEY_ESCAPE)) {
            game.screen = SCREEN_MAIN_MENU;
            ShowCursor();
            StartMenuMusic(&game);
            continue;
        }

        switch (game.state) {
        case GS_PLAYING:
            HideCursor();
            if (IsKeyPressed(KEY_R)) {
                game.ball.pos = hole->tee_pos;
                game.ball.vel = Vector2Zero(); //R key resets the ball
                game.ball.state = BALL_AIM;
            }

            putter_update(&game.putter,&in,&game.ball,dt);
            if (game.ball.hitByPutter)
            {
                PlaySound(game.putterSound);
                game.ball.hitByPutter = false;
            }

            BallUpdate(&game.ball,hole,dt);

            if (game.ball.hitWater)
            {
                PlaySound(game.waterSound);
            }

            if (game.ball.hitWall)
            {
                PlaySound(game.wallSound);
            }

            if (game.ball.hitCup)
            {
                PlaySound(game.cupSound);
            }

            if (game.ball.state == BALL_SUNK) { //completed the current hole
                game.scores[game.course.current]=game.ball.strokes;
                game.hole_done_t=0.0f;
                game.state = GS_HOLE_DONE;
                int idx = game.course.current; //index of current hole
                if (game.best[idx]==0 || game.ball.strokes < game.best[idx]) game.best[idx] = game.ball.strokes;
            }

            else if (game.ball.state == BALL_AIM && game.ball.strokes >= hole->par + STROKE_LIMIT_OVER_PAR) {
                game.scores[game.course.current] = hole->par + STROKE_LIMIT_OVER_PAR;
                game.hole_done_t = 0.0f;
                game.state = GS_HOLE_DONE;
            }
            break;

        case GS_HOLE_DONE:
            game.hole_done_t += dt; //calculate the time passed after the hole is completed , by adding dt
            if (game.hole_done_t>=HOLE_DONE_PAUSE || in.confirm) {
                if (course_advance(&game.course)) StartHole(&game); //Attempts to move to the next hole
                else
                {
                    CheckFinalScore(&game);
                    ShowCursor();
                    game.state = GS_SCOREBOARD;
                } 
            }
            break;

        case GS_SCOREBOARD:
            BeginDrawing();
            ClearBackground(BLACK);
            if(DrawFinalScoreScreen(&game))
            {
                shouldQuit=true; //If the return parameter is 1 that means user pressed exit , so shouldQuit should become true.
            }
            EndDrawing();
            continue;

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
        DrawHUD(hole,&game.ball,&game.putter,game.course.current,game.course.hole_count,Course_total(game.scores,game.course.current));
        if (game.state==GS_HOLE_DONE) {
            const char *msg=score_name(game.scores[game.course.current],hole->par); //par , eagle , birdie...
            int tw=MeasureText(msg,54);
            DrawText(msg,GetScreenWidth()/2-tw/2,GetScreenHeight()/2-40,54,(Color){255,214,102,255});
        }
        EndDrawing();
    }

    StopMusicStream(game.menuMusic);
    StopMusicStream(game.gameMusic);
    UnloadMusicStream(game.menuMusic);
    UnloadMusicStream(game.gameMusic);
    UnloadSound(game.waterSound);
    UnloadSound(game.wallSound);
    UnloadSound(game.cupSound);
    UnloadSound(game.putterSound);
    UnloadTexture(game.menuBackground);
    UnloadTexture(game.tutorialImage);
    UnloadTexture(game.aboutImage);
    UnloadTexture(game.windSprite);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

#include "level.h"
#include "types.h"
#include <raymath.h>
#include <string.h>
#include <raylib.h>
#include <stdio.h>
#include <math.h>

#define RAil 12.0f



// addzone function o lekha baki
void AddZone(Hole *h, SurfaceType t, float x, float y, float w, float ht, Vector2 wind){
    if(h->zonecount >= MAX_ZONES) return;
    h->zones[h->zonecount++] = (Zone){(Rectangle){x,y,w,ht},t,wind};
}

// for the course loading && file handling for creating the map or course

bool Validate_hole(Hole *h,const char *path){
    bool ok = true;

    if(h->par < 1 || h->par > 9) h->par = 3;
    if(h->cupradius < 4.0f) h->cupradius = 14.0f;
    if(h->bounds.width < 100.0f || h->bounds.height < 100.0f) ok = false;
    if(!CheckCollisionPointRec(h->teepos, h->bounds)) ok = false;
    if(!CheckCollisionPointRec(h->cupPos, h->bounds)) ok = false;
    if(h-> wallcount == 0) ok = false; // im not sure about this one, need to check !!!!!!!
    return ok;
}

// add_wall function lekha baki ekhono
void AddWall(Hole *h, float x, float y, float w, float ht, float bounce){{
    if(h->wallcount >= MAX_WALLS) return;
    h->walls[h->wallcount++] = (Wall){(Rectangle){x,y,w,ht},bounce};
}




}

// taking input from the lines with this function......

void Parseline(Hole *h,char *line, int line_no, const char *path){
    while(*line == ' ') line++;
    if(*line == '#' || *line == '\0') return;


    char k[32];
    if(sscanf(line, "%31s",k) != 1) return;
    
    if(strcmp(k,"hole") == 0){
        sscanf(line, "%*s %d",&h->number);
    }
    else if(strcmp(k, "par") == 0){
        sscanf(line,"%*s %d",&h->par );
    }
    else if(strcmp(k, "bounds") == 0){
        sscanf(line, "%*s %f %f %f %f",&h->bounds.x,&h->bounds.y,&h->bounds.width, &h->bounds.height);
    }
    else if(strcmp(k, "tee") == 0){
        sscanf(line, "%*s %f %f",&h->teepos.x, &h->teepos.y);
    }
    else if(strcmp(k, "cup") == 0){
        sscanf(line, "%*s %f %f %f",&h->cupPos.x, &h->cupPos.y, &h->cupradius);
    }
    else if(strcmp(k, "drop") == 0){
        sscanf(line, "%*s %f %f %f %f", &h->dropZone.x, &h->dropZone.y, &h->dropZone.width, &h->dropZone.height);
    }
    else if(strcmp(k, "wall") == 0){
        float x, y, w, ht, b = 0.72f;
        sscanf(line, "%*s %f %f %f %f %f",&x, &y, &w, &ht, &b);
        AddWall(h,x,y,w,ht,b);
    }
    else if(strcmp(k,"zone") == 0){
        int zone_number; // here i will take the enum of the zone directly no name conversion required
        float x , y ,w , ht, wx = 0.0f, wy = 0.0f;
        sscanf(line, "%*s %d %f %f %f %f %f %f",&zone_number,&x,&y,&w, &ht, &wx, &wy);
        AddZone(h,zone_number,x,y,w,ht,(Vector2){wx, wy});
    }
    else if(strcmp(k, "end") == 0){
        // some decoration here...
    }
}

bool LoadHoleFromFile(Hole *h, const char *path){
    if(!FileExists(path)) return false;

    char *text = LoadFileText(path);
    if(!text) return false;

    *h = (Hole) {0}; // clearing the struct data, so that no overlap happens in future;

    //setting the defualt;
    h->par = 3;
    h->cupradius = 14.0f;
    h->bounds = (Rectangle){0,0,1600,900};

    int line_no = 0;
    char *line = strtok(text, "\r\n");

    while(line){
        line_no++;
        Parseline(h,line,line_no,path);
        line = strtok(NULL, "\r\n");
    }
    UnloadFileText(text);

    return Validate_hole(h,path);

}



#define LEVEL_TEXT_MAX 16384
// later
bool SaveHoleToFile(const Hole *h, const char *path){
    char buf[LEVEL_TEXT_MAX];
    int n = 0;
    /*  ei function pore likhbo, this one for editing option and stuffs for saving */
    return false;
}


int course_load(Course *c, const char *dir){
    c->hole_count = 0;
    c->current = 0;

    for(int i = 1; i < MAX_HOLES; i++){
        const char *path = TextFormat("%s/hole%02d.txt",dir,i);
        if(!FileExists(path)) break;

        if(LoadHoleFromFile(&c->holes[c->hole_count], path)) c->hole_count++;
    }

    return c->hole_count;
}

bool course_advance(Course *c){
    if(c->current + 1 >= c->hole_count) return false;
    c->current++;
    return true;
}

Hole *course_current(Course *c){
    if(c->current < 0 || c.current >= c->hole_count) return &c->holes[0];
    return &c->holes[c->current];
}

// now for scoring part

const char *score_name(int strokes, int par){
    if(strokes == 1) return "HOLE IN ONE!!!";
    switch(strokes - par){
        case - 3 : return "ALBATROSS";
        case - 2 : return "EAGLE";
        case - 1 : return "BIRDIE";
        case 0 : return "PAR";
        case 1: return "BOGEY";
        case 2: return "DOUBLE BOGEY";
        case 3: return "TRIPLE BOGEY";
        default : return (strokes < par) ? "GREAT" : "YOU SUCK, NOOB";

    }
}


int Course_total(const int *scores, int hole_count){
    int total = 0;
    for(int i = 0; i < hole_count; i++){
        if(scores[i] > 0){
            total += scores[i];
        }
    }
    return total;
}
// this function needs some reading for mee...
int course_to_par(const Course *c, const int *scores){
    int diff = 0;
    for(int i = 0; i < c->hole_count; i++){
        if(scores[i] > 0) diff+= scores[i] - c->holes[i].par;
    }

    return diff;
}

// save file lekha lagbe ekhane.......



// editor

void EditorInit(Edtior *e){
    e->tool = TOOL_WALL;
    e->zonetype = SURF_SAND;
    e->dragging = false;
    e->grid_size = 10;
    e->selected = -1;
}

Vector2 Snap2Grid(Vector2 p, int grid){
    if(grid <= 1) return p;
    return (Vector2){ roundf(p.x / grid) * grid, roundf(p.y / grid) * grid};
}

Rectangle RectFromCorners(Vector2 a, Vector2 b){
    return (Rectangle){
        fminf(a.x,b.x), fminf(a.y,b.y),fabsf(b.x - a.x),fabsf(b.y - a.y)
    };
}

// its simply like removing a element from middle of an array and shifting the position
void RemoveWall(Hole *h, int index){
    if(index < 0 || index >= h->wallcount) return;
    for(int i = index; i < h->wallcount - 1; i++){
        h->walls[i] = h->walls[i + 1];
    }
    h->wallcount--;
}

int WallAtPoint(const Hole *h, Vector2 p){
    for(int i = h->wallcount - 1; i >= 0; i--){
        if(CheckCollisionPointRec(p, h->walls[i].rect)) return i;
    }
    return -1;
}
// for undoing things while the editor is open ... hell yeah , we are making our own map
Hole UndoBuffer;
bool UndoValid = false;

void PushUndo(const Hole *h){
    UndoBuffer = *h; 
    UndoValid = true;
}

void PopUndo(Hole *h){
    if(!UndoValid) return;
    Hole temporary = *h;
    *h = UndoBuffer;
    UndoBuffer = temporary;
}

void Editor_Update(Edtior *e, Hole *h, const InputState *in){
    Vector2 p = Snap2Grid(in->pointer_world, e->grid_size);
    e->selected = WallAtPoint(h, in->pointer_world);


    // for the tools
    if(IsKeyPressed(KEY_ONE)) e->tool = TOOL_WALL;
    if(IsKeyPressed(KEY_TWO)) e->tool = TOOL_ZONE;
    if(IsKeyPressed(KEY_THREE)) e->tool = TOOL_TEE;
    if(IsKeyPressed(KEY_FOUR)) e->tool = TOOL_CUP;
    if(IsKeyPressed((KEY_FIVE))) e->tool = TOOL_DROP;

    // cycle of zonetype baby

    if(IsKeyPressed(KEY_TAB)){
        e->zonetype = (SurfaceType)((e->zonetype + 1) % SURF_COUNT);
    }

    //// another cycle but with if else loop _ for grid sizing
    if(IsKeyPressed(KEY_G)){
        if(e->grid_size == 1) e->grid_size = 5;
        else if(e->grid_size == 5) e->grid_size = 10;
        else if(e->grid_size == 10) e->grid_size = 20;
        else e->grid_size = 1;
    }

    // main edit starts here...
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        PushUndo(h);
        e->dragging = true;
        e->drag_start = p;
    }
    
    if(e->dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        e->dragging = false;
        Rectangle r = RectFromCorners(e->drag_start, p);

        switch(e->tool){
            case TOOL_TEE:
                h->teepos = p; 
                break;
            case TOOL_CUP:
                h->cupPos = p;
                break;
            case TOOL_DROP:
                if(r.width >= 4.0f && r.height >= 4.0f){
                    h->dropZone = r;
                }
                break;
            case TOOL_WALL : 
                if(r.width >= 4.0f && r.height >= 4.0f)
                    AddWall(h,r.x,r.y,r.width,r.height,0.72f);
                break;
            case TOOL_ZONE:
                if(r.width >= 4.0f && r.height >= 4.0f){
                    AddZone(h,e->zonetype, r.x, r.y, r.width, r.height,Vector2Zero());
                }
                break;
                
        }

    }
    // forwhat





}
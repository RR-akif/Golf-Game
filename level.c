#include "level.h"
#include "types.h"
#include <raymath.h>
#include <string.h>
#include <raylib.h>
#include <stdio.h>






// addzone function o lekha baki
void AddZone(Hole *h, SurfaceType t, float x, float y, float w, float ht, Vector2 wind){
    if(h->zone_count >= MAX_ZONES) return;
    h->zones[h->zone_count++] = (Zone){(Rectangle){x,y,w,ht},t,wind};
}

// for the course loading && file handling for creating the map or course

bool Validate_hole(Hole *h,const char *path){
    bool ok = true;

    if(h->par < 1 || h->par > 9) h->par = 3;
    if(h->cup_radius < 4.0f) h->cup_radius = 14.0f;
    if(h->bounds.width < 100.0f || h->bounds.height < 100.0f) ok = false;
    if(!CheckCollisionPointRec(h->tee_pos, h->bounds)) ok = false;
    if(!CheckCollisionPointRec(h->cup_pos, h->bounds)) ok = false;
    if(h-> wall_count == 0) ok = false; // im not sure about this one, need to check !!!!!!!
    return ok;
}

// add_wall function lekha baki ekhono
void AddWall(Hole *h, float x, float y, float w, float ht, float bounce){{
    if(h->wall_count >= MAX_WALLS) return;
    h->walls[h->wall_count++] = (Wall){(Rectangle){x,y,w,ht},bounce};
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
        sscanf(line, "%*s %f %f",&h->tee_pos.x, &h->tee_pos.y);
    }
    else if(strcmp(k, "cup") == 0){
        sscanf(line, "%*s %f %f %f",&h->cup_pos.x, &h->cup_pos.y, &h->cup_radius);
    }
    else if(strcmp(k, "drop") == 0){
        sscanf(line, "%*s %f %f %f %f", &h->drop_zone.x, &h->drop_zone.y, &h->drop_zone.width, &h->drop_zone.height);
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

    *h = (Hole){0}; // clearing the struct data, so that no overlap happens in future;

    //setting the defualt;
    h->par = 3;
    h->cup_radius = 14.0f;
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
    if(c->current < 0 || (c->current >= c->hole_count)) return &c->holes[0];
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
        default : return (strokes < par) ? "GREAT" : "You suck,noob";

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

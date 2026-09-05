#ifndef LEVEL_H
#define LEVEL_H

#include "input.h"
#include "types.h"

// defining the data structure for COURSE;
typedef struct{
    Hole holes[MAX_HOLES];
    int hole_count;
    int current;
} Course;

//editor tool option
typedef enum{
    TOOL_WALL,
    TOOL_ZONE,
    TOOL_TEE,
    TOOL_CUP,
    TOOL_DROP
} Edit_Tool;

//editor struct

typedef struct{
    Edit_Tool tool;
    SurfaceType zone_type;
    bool dragging;
    Vector2 drag_start;
    int grid_size;
    int selected; 
}Edtior;


// removed the build hole function... as i will draw map from text files..... so only about that is declared here...    


bool LoadHoleFromFile(Hole *h,const char *path);
bool SaveHoleToFile(const Hole *h, const char *path);


int course_load(Course *c, const char *dir);
bool course_advance(Course *c);
Hole *course_current(Course *c);

// for scoring this functions;


const char *score_name(int strokes, int par);
int Course_total(const int *scores, int hole_count);
int course_to_par(const Course *c, const int  *scores);
void save_best_scores(const int *best, int hole_count);
void load_best_scores(int *best, int hole_count);


//// for surfaces...

SurfaceType surface_from_name(const char *name);
const char *surface_name(SurfaceType s);

void EditorInit(Edtior *e);
void editor_update(Edtior *e, Hole *h, const InputState *in);





#endif
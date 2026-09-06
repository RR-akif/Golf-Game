#include "putter.h"
#include "ball.h"

#include "raymath.h"


#define CHARGE_TIME 1.15f
#define MIN_POWER 0.05f


// no animation version. the putter only holds the charge and fires the ball.
// the swing animation lives in unused/putter_anim.c

void putter_init(Putter *p){
    p->phase = putter_idle;
    p->t = p->power = p->launch_power = p->aim_angle = p->offset = 0.0f;
    p->dir = 1.0f;
    p->fired = false;
}

bool PutterIsCharging(const Putter *p){
    return p->phase == putter_backswing;
}

float PutterPower(const Putter *p){
    return p->power;
}

void putter_update(Putter *p, const InputState *in, Ball *ball, float dt){
    p->aim_angle = in->aim_angle;

    bool addressable = ball->state == BALL_AIM;

    if(p->phase == putter_idle){
        p->power = 0.0f;
        p->launch_power = 0.0f;
        p->fired = false;

        if(addressable && in->charge_pressed){
            p->phase = putter_backswing;
            p->dir = 1.0f;
        }
        return;
    }

    // charging. the bar fills up to full, then drains back to zero, over and over,
    // so the player has to release at the right moment
    if(in->charge_held){
        p->power += p->dir * dt / CHARGE_TIME;
        if(p->power > 1.0f){ p->power = 1.0f; p->dir = -1.0f; }
        if(p->power < 0.0f){ p->power = 0.0f; p->dir =  1.0f; }
    }

    if(in->cancel){
        // no launch, no stroke -- the ball is never touched
        p->power = 0.0f;
        p->phase = putter_idle;
        return;
    }

    if(in->charge_released){
        // releasing at the bottom of the sweep still counts as a stroke, just a weak one
        p->launch_power = (p->power < MIN_POWER) ? MIN_POWER : p->power;
        p->fired = true;
        BallLaunch(ball, DirFromAngle(p->aim_angle), p->launch_power);
        p->power = 0.0f;
        p->phase = putter_idle;
    }
}

void putter_draw(const Putter *p, const Ball *ball){
    // nothing to draw for now, no club, no animation
    (void)p;
    (void)ball;
}

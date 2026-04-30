/* boost.c — Boost entity management, rendering, collision */
#include "boost.h"
#include "screen.h"
#include "math.h"

void boost_spawn_at(Boost *b,int max,int x,int y,int *count){
    int i;
    for(i=0;i<max;i++){
        if(!b[i].active){
            b[i].active=1; b[i].x=x; b[i].y=y;
            b[i].type=(BoostType)custom_rand_range(0,3);
            (*count)++; return;
        }
    }
}

void boost_update(Boost *b,int max,int tick,int play_x,int *count){
    int i;
    for(i=0;i<max;i++){
        if(!b[i].active) continue;
        if(tick&1) b[i].x--;
        if(b[i].x<play_x){b[i].active=0;(*count)--;}
    }
}

void boost_render(Boost *b,int max,int tick){
    int i;
    for(i=0;i<max;i++){
        if(!b[i].active) continue;
        if((tick&7)==0) continue;
        int px=b[i].x,py=b[i].y;
        char ch;int cl,gl;int gw=(tick&2)?1:0;
        switch(b[i].type){
            case BOOST_HEAL:  ch='+';cl=COL_BRIGHT_GREEN; gl=COL_GREEN;break;
            case BOOST_RAPID: ch='F';cl=COL_BRIGHT_YELLOW;gl=COL_YELLOW;break;
            case BOOST_SHIELD:ch='O';cl=COL_BRIGHT_MAGENTA;gl=COL_MAGENTA;break;
            case BOOST_DOUBLE:ch='D';cl=COL_BRIGHT_WHITE;  gl=COL_WHITE;break;
        }
        int fc=gw?cl:gl;
        screen_put(py-1,px,'.',gw?cl:COL_WHITE,COL_RESET);
        screen_put(py-1,px+1,ch,fc,COL_RESET);
        screen_put(py-1,px+2,'.',gw?cl:COL_WHITE,COL_RESET);
        screen_put(py,px,'<',cl,COL_RESET);
        screen_put(py,px+1,ch,gw?COL_BRIGHT_WHITE:cl,COL_RESET);
        screen_put(py,px+2,'>',cl,COL_RESET);
        screen_put(py+1,px,'.',gw?cl:COL_WHITE,COL_RESET);
        screen_put(py+1,px+1,ch,fc,COL_RESET);
        screen_put(py+1,px+2,'.',gw?cl:COL_WHITE,COL_RESET);
    }
}

int boost_check_collect(Boost *b,int px,int py,int pw,int ph){
    Rect br={b->x,b->y-1,BOOST_W,BOOST_HT};
    Rect pr={px,py,pw,ph};
    return check_collision(br,pr);
}

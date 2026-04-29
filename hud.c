/* hud.c — Premium Retro Arcade HUD Rendering */
#include "hud.h"
#include "screen.h"
#include "string.h"
#include "math.h"

static char  g_notif[80];
static int   g_notif_color;
static int   g_notif_timer;

void hud_init(void){g_notif[0]='\0';g_notif_timer=0;}

void hud_notify(const char *msg,int color){
    str_copy(g_notif,msg);g_notif_color=color;g_notif_timer=NOTIF_DURATION;
}

void hud_tick(void){if(g_notif_timer>0)g_notif_timer--;}

static void ns(int v,char *b){int_to_str(v,b,16);}

void hud_draw_top(int score,int level,int hp,int max_hp,
                  int gun_mode,int shield_hits,
                  int kills,int missed,int max_missed){
    char buf[16];int c;
    /* separator */
    for(c=0;c<SCREEN_W;c++) screen_put(0,c,'-',COL_BRIGHT_BLUE,COL_RESET);
    screen_put(0,0,'|',COL_BRIGHT_BLUE,COL_RESET);
    screen_put(0,SCREEN_W-1,'|',COL_BRIGHT_BLUE,COL_RESET);

    /* HP section: HP [##########] 100% */
    screen_draw_string(0,1,"HP",COL_BRIGHT_RED,COL_RESET);
    screen_put(0,4,'[',COL_WHITE,COL_RESET);
    {int pct=custom_div(hp*10,max_hp);int h;
     int hc=pct>4?COL_BRIGHT_GREEN:pct>2?COL_YELLOW:COL_BRIGHT_RED;
     for(h=0;h<10;h++){
         screen_put(0,5+h,h<pct?'#':'-',h<pct?hc:COL_WHITE,COL_RESET);
     }}
    screen_put(0,15,']',COL_WHITE,COL_RESET);
    {int pv=custom_div(hp*100,max_hp);ns(pv,buf);
     screen_draw_string(0,17,buf,COL_BRIGHT_WHITE,COL_RESET);
     screen_put(0,17+str_len(buf),'%',COL_BRIGHT_WHITE,COL_RESET);}

    screen_put(0,22,'|',COL_BRIGHT_BLUE,COL_RESET);

    /* GUN section */
    screen_draw_string(0,23,"GUN",COL_BRIGHT_YELLOW,COL_RESET);
    if(gun_mode==2)
        screen_draw_string(0,27,"[SPREAD]",COL_BRIGHT_CYAN,COL_RESET);
    else if(gun_mode==3)
        screen_draw_string(0,27,"[DOUBLE]",COL_BRIGHT_WHITE,COL_RESET);
    else if(gun_mode==1)
        screen_draw_string(0,27,"[RAPID]",COL_BRIGHT_YELLOW,COL_RESET);
    else
        screen_draw_string(0,27,"[NORMAL]",COL_WHITE,COL_RESET);

    screen_put(0,36,'|',COL_BRIGHT_BLUE,COL_RESET);

    /* SHIELD section */
    screen_draw_string(0,37,"SH",COL_BRIGHT_MAGENTA,COL_RESET);
    if(shield_hits>0){
        screen_draw_string(0,39,"[ON:",COL_BRIGHT_GREEN,COL_RESET);
        ns(shield_hits,buf);
        screen_draw_string(0,43,buf,COL_BRIGHT_WHITE,COL_RESET);
        screen_put(0,43+str_len(buf),']',COL_BRIGHT_GREEN,COL_RESET);
    } else {
        screen_draw_string(0,39,"[OFF]",COL_RED,COL_RESET);
    }

    screen_put(0,46,'|',COL_BRIGHT_BLUE,COL_RESET);

    /* SCORE + LVL */
    ns(score,buf);
    screen_draw_string(0,47,"SC:",COL_BRIGHT_GREEN,COL_RESET);
    screen_draw_string(0,50,buf,COL_BRIGHT_WHITE,COL_RESET);

    ns(level,buf);
    screen_draw_string(0,57,"LV:",COL_BRIGHT_YELLOW,COL_RESET);
    screen_draw_string(0,60,buf,COL_BRIGHT_WHITE,COL_RESET);

    /* KILLS + MISSED */
    ns(kills,buf);
    screen_draw_string(0,63,"K:",COL_BRIGHT_CYAN,COL_RESET);
    screen_draw_string(0,65,buf,COL_BRIGHT_WHITE,COL_RESET);

    screen_put(0,69,'|',COL_BRIGHT_BLUE,COL_RESET);

    {char mb[16]="M:";ns(missed,buf);str_concat(mb,buf);
     str_concat(mb,"/");ns(max_missed,buf);str_concat(mb,buf);
     int mc=missed>=max_missed-3?COL_BRIGHT_RED:COL_BRIGHT_MAGENTA;
     screen_draw_string(0,70,mb,mc,COL_RESET);}
}

void hud_draw_bottom(int tick,int rapid_t,int spread_t,
                     int double_t,int shield_h,
                     int enemy_count,int hi_score){
    char buf[16];int by=SCREEN_H-1,bx=1;
    (void)tick;
    for(int c=0;c<SCREEN_W;c++)screen_put(by,c,'-',COL_BRIGHT_BLUE,COL_RESET);
    screen_put(by,0,'|',COL_BRIGHT_BLUE,COL_RESET);
    screen_put(by,SCREEN_W-1,'|',COL_BRIGHT_BLUE,COL_RESET);

    if(spread_t>0){
        screen_put(by,bx,'<',COL_CYAN,COL_RESET);
        screen_draw_string(by,bx+1,"SPR:",COL_BRIGHT_CYAN,COL_RESET);
        ns(spread_t,buf);screen_draw_string(by,bx+5,buf,COL_BRIGHT_WHITE,COL_RESET);
        screen_put(by,bx+5+str_len(buf),'>',COL_CYAN,COL_RESET);bx+=7+str_len(buf);
    }
    if(rapid_t>0){
        screen_put(by,bx,'<',COL_YELLOW,COL_RESET);
        screen_draw_string(by,bx+1,"RPD:",COL_BRIGHT_YELLOW,COL_RESET);
        ns(rapid_t,buf);screen_draw_string(by,bx+5,buf,COL_BRIGHT_WHITE,COL_RESET);
        screen_put(by,bx+5+str_len(buf),'>',COL_YELLOW,COL_RESET);bx+=7+str_len(buf);
    }
    if(double_t>0){
        screen_put(by,bx,'<',COL_WHITE,COL_RESET);
        screen_draw_string(by,bx+1,"DBL:",COL_BRIGHT_WHITE,COL_RESET);
        ns(double_t,buf);screen_draw_string(by,bx+5,buf,COL_BRIGHT_WHITE,COL_RESET);
        screen_put(by,bx+5+str_len(buf),'>',COL_WHITE,COL_RESET);bx+=7+str_len(buf);
    }
    if(shield_h>0){
        screen_put(by,bx,'<',COL_MAGENTA,COL_RESET);
        screen_draw_string(by,bx+1,"SH:",COL_BRIGHT_MAGENTA,COL_RESET);
        ns(shield_h,buf);screen_draw_string(by,bx+4,buf,COL_BRIGHT_WHITE,COL_RESET);
        screen_put(by,bx+4+str_len(buf),'>',COL_MAGENTA,COL_RESET);bx+=6+str_len(buf);
    }

    ns(enemy_count,buf);
    screen_draw_string(by,55,"ENM:",COL_MAGENTA,COL_RESET);
    screen_draw_string(by,59,buf,COL_BRIGHT_WHITE,COL_RESET);

    ns(hi_score,buf);
    screen_draw_string(by,65,"HI:",COL_BRIGHT_MAGENTA,COL_RESET);
    screen_draw_string(by,68,buf,COL_BRIGHT_WHITE,COL_RESET);
}

void hud_draw_notification(int tick){
    if(g_notif_timer<=0) return;
    int y=SCREEN_H-2;
    /* flashing notification */
    if((tick&3)<3){
        int nx=SCREEN_W/2-str_len(g_notif)/2-4;
        if(nx<1)nx=1;
        screen_draw_string(y,nx,">>> ",COL_BRIGHT_WHITE,COL_RESET);
        screen_draw_string(y,nx+4,g_notif,g_notif_color,COL_RESET);
        screen_draw_string(y,nx+4+str_len(g_notif)," <<<",COL_BRIGHT_WHITE,COL_RESET);
    }
}

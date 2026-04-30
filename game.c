/* ============================================================
 *  game.c — Horizontal Space Shooter (premium retro arcade)
 * ============================================================ */
 #include "screen.h"
 #include "game.h"
 #include "keyboard.h"
 #include "memory.h"
 #include "math.h"
 #include "string.h"
 #include "boost.h"
 #include "hud.h"
 #include "sound.h"
 #include <stdio.h>
 #include <unistd.h>
 #include <time.h>
 
 /* ---- helpers ---- */
 static void ns(int v,char*b){int_to_str(v,b,16);}
 static int ecolor(int l){
     int p[]={COL_RED,COL_MAGENTA,COL_YELLOW,COL_CYAN,
              COL_BRIGHT_RED,COL_ORANGE,COL_PINK,
              COL_BRIGHT_MAGENTA,COL_BRIGHT_CYAN,COL_BRIGHT_YELLOW};
     return p[custom_mod(l,10)];
 }
 
 /* ---- forward decls ---- */
 static void state_menu(GameState*);
 static void state_controls(GameState*);
 static void state_playing(GameState*);
 static void state_gameover(GameState*);
 static void init_game(GameState*);
 static void free_ent(GameState*);
 static void process_input(GameState*);
 static void update(GameState*);
 static void render(GameState*);
 static void fire_bullet(GameState*);
 static void enemy_fire(GameState*,int);
 static void spawn_enemy(GameState*);
 static void spawn_boss(GameState*);
 static void spawn_terrain(GameState*);
 static void spawn_asteroid(GameState*);
 static void add_explosion(GameState*,int,int);
 
 void game_run(void){
     GameState gs;
     gs.running=1; gs.mode=STATE_MENU; gs.high_score=0;
     gs.bullets=NULL; gs.enemy_bullets=NULL; gs.enemies=NULL;
     gs.obstacles=NULL; gs.explosions=NULL; gs.boosts=NULL;
     custom_srand((unsigned int)time(NULL));
     sound_init();
     while(gs.running){
         switch(gs.mode){
             case STATE_MENU:     state_menu(&gs);     break;
             case STATE_CONTROLS: state_controls(&gs); break;
             case STATE_PLAYING:  state_playing(&gs);  break;
             case STATE_GAMEOVER: state_gameover(&gs);  break;
         }
     }
 }
 
 /* =============== MENU =============== */
 static void state_menu(GameState*gs){
     screen_invalidate(); screen_clear_buf();
     int ty=2,tx=15;
     screen_draw_string(ty,  tx," ___  ___   _    ___ ___",     COL_BRIGHT_CYAN,COL_RESET);
     screen_draw_string(ty+1,tx,"/ __||  _ \\/ \\  / __| __|", COL_CYAN,COL_RESET);
     screen_draw_string(ty+2,tx,"\\__ \\| |_)/ _ \\| (__| _|", COL_BRIGHT_BLUE,COL_RESET);
     screen_draw_string(ty+3,tx,"|___/|___/_/ \\_\\___|___|",   COL_BRIGHT_MAGENTA,COL_RESET);
     int ty2=ty+5,tx2=10;
     screen_draw_string(ty2,  tx2," ___ _  _  ___   ___ _____ ___ ___",   COL_BRIGHT_RED,COL_RESET);
     screen_draw_string(ty2+1,tx2,"/ __| || |/ _ \\ / _ \\_   _| __| _ \\",COL_ORANGE,COL_RESET);
     screen_draw_string(ty2+2,tx2,"\\__ \\ __ | (_) | (_) || | | _||   /", COL_YELLOW,COL_RESET);
     screen_draw_string(ty2+3,tx2,"|___/|_||_|\\___/ \\___/ |_| |___|_|_\\",COL_BRIGHT_YELLOW,COL_RESET);
     int my=ty2+5;
     screen_draw_string(my,  24,">>>  [1]  START GAME  <<<",COL_BRIGHT_GREEN,COL_RESET);
     screen_draw_string(my+1,24,"     [2]  CONTROLS      ",COL_BRIGHT_YELLOW,COL_RESET);
     screen_draw_string(my+2,24,"     [3]  QUIT          ",COL_BRIGHT_RED,COL_RESET);
     if(gs->high_score>0){
         char h[48]="HIGH SCORE: ";char n[16];ns(gs->high_score,n);str_concat(h,n);
         screen_draw_string(my+4,24,h,COL_BRIGHT_MAGENTA,COL_RESET);
     }
     screen_draw_box(0,0,SCREEN_W,SCREEN_H,COL_BRIGHT_BLUE,COL_RESET);
     screen_flush();
     while(1){
         if(kb_hit()){int k=kb_read_key();
             if(k=='1'){gs->mode=STATE_PLAYING;return;}
             if(k=='2'){gs->mode=STATE_CONTROLS;return;}
             if(k=='3'){gs->running=0;return;}
         }
         screen_ping_resize();
         usleep(50000);
     }
 }
 
 /* =============== CONTROLS =============== */
 static void state_controls(GameState*gs){
     screen_invalidate(); screen_clear_buf();
     screen_draw_box(0,0,SCREEN_W,SCREEN_H,COL_BRIGHT_BLUE,COL_RESET);
     screen_draw_string(1,28,"=== CONTROLS ===",COL_BRIGHT_CYAN,COL_RESET);
     int y=3;
     screen_draw_string(y,  8,"W / UP      Move Up",       COL_BRIGHT_GREEN,COL_RESET);
     screen_draw_string(y+1,8,"S / DOWN    Move Down",     COL_BRIGHT_GREEN,COL_RESET);
     screen_draw_string(y+2,8,"A / LEFT    Move Backward", COL_BRIGHT_GREEN,COL_RESET);
     screen_draw_string(y+3,8,"D / RIGHT   Move Forward",  COL_BRIGHT_GREEN,COL_RESET);
     screen_draw_string(y+4,8,"SPACE       Fire",          COL_BRIGHT_YELLOW,COL_RESET);
     screen_draw_string(y+5,8,"P / Q       Pause / Quit",  COL_BRIGHT_RED,COL_RESET);
     screen_draw_string(y+7,8,"BOOSTS (drop on enemy kill):",COL_BRIGHT_WHITE,COL_RESET);
     screen_draw_string(y+8,8,"<+> Health      +20 HP restored",COL_BRIGHT_GREEN,COL_RESET);
     screen_draw_string(y+9,8,"<F> Rapid Fire  Auto-shoot 10 sec",COL_BRIGHT_YELLOW,COL_RESET);
     screen_draw_string(y+10,8,"<O> Shield     Absorbs 5 hits",COL_BRIGHT_MAGENTA,COL_RESET);
     screen_draw_string(y+11,8,"<D> Double Shot Two bullets at once",COL_BRIGHT_WHITE,COL_RESET);
     {char mb[48]="Game over: HP=0 or ";char n[8];ns(MAX_MISSED,n);
      str_concat(mb,n);str_concat(mb," ships escape");
      screen_draw_string(y+13,8,mb,COL_ORANGE,COL_RESET);}
     screen_draw_string(SCREEN_H-2,20,"Press any key to return...",COL_WHITE,COL_RESET);
     screen_flush();
     while(1){
         if(kb_hit()){kb_read_key();gs->mode=STATE_MENU;return;}
         screen_ping_resize();
         usleep(50000);
     }
 }
 
 /* =============== INIT / FREE =============== */
 static void init_game(GameState*gs){
     gs->bullets      =(Bullet*)     mem_alloc((int)sizeof(Bullet)*MAX_BULLETS);
     gs->enemy_bullets=(EnemyBullet*)mem_alloc((int)sizeof(EnemyBullet)*MAX_ENEMY_BULLETS);
     gs->enemies      =(Enemy*)      mem_alloc((int)sizeof(Enemy)*MAX_ENEMIES);
     gs->obstacles    =(Obstacle*)   mem_alloc((int)sizeof(Obstacle)*MAX_OBSTACLES);
     gs->explosions   =(Explosion*)  mem_alloc((int)sizeof(Explosion)*MAX_EXPLOSIONS);
     gs->boosts       =(Boost*)      mem_alloc((int)sizeof(Boost)*MAX_BOOSTS);
     int i;
     for(i=0;i<MAX_BULLETS;i++)       gs->bullets[i].active=0;
     for(i=0;i<MAX_ENEMY_BULLETS;i++) gs->enemy_bullets[i].active=0;
     for(i=0;i<MAX_ENEMIES;i++)       gs->enemies[i].active=0;
     for(i=0;i<MAX_OBSTACLES;i++)     gs->obstacles[i].active=0;
     for(i=0;i<MAX_EXPLOSIONS;i++)    gs->explosions[i].active=0;
     for(i=0;i<MAX_BOOSTS;i++)        gs->boosts[i].active=0;
 
     gs->player.x=PLAYER_START_X;
     gs->player.y=PLAY_Y+PLAY_H/2-PLAYER_H/2;
     gs->player.hp=PLAYER_MAX_HP;
     gs->player.fire_cd=0; gs->player.invincible=0;
     gs->player.rapid_timer=0; gs->player.spread_timer=0;
     gs->player.shield_hits=0; gs->player.double_timer=0;
 
     gs->bullet_count=gs->enemy_bullet_count=0;
     gs->enemy_count=gs->obstacle_count=gs->explosion_count=gs->boost_count=0;
     gs->score=0; gs->level=1; gs->ships_destroyed=0; gs->ships_missed=0;
     gs->spawn_cd=BASE_SPAWN_CD; gs->spawn_timer=gs->spawn_cd;
     gs->terrain_timer=1; gs->asteroid_timer=ASTEROID_SPAWN_CD;
     gs->tick=0;
     hud_init();
     screen_invalidate();
 }
 
 static void free_ent(GameState*gs){
     if(gs->bullets)      {mem_free(gs->bullets);      gs->bullets=NULL;}
     if(gs->enemy_bullets){mem_free(gs->enemy_bullets);gs->enemy_bullets=NULL;}
     if(gs->enemies)      {mem_free(gs->enemies);      gs->enemies=NULL;}
     if(gs->obstacles)    {mem_free(gs->obstacles);     gs->obstacles=NULL;}
     if(gs->explosions)   {mem_free(gs->explosions);    gs->explosions=NULL;}
     if(gs->boosts)       {mem_free(gs->boosts);        gs->boosts=NULL;}
 }
 
 /* =============== GAME LOOP =============== */
 static void state_playing(GameState*gs){
     init_game(gs);
     while(gs->mode==STATE_PLAYING && gs->running){
         process_input(gs);
         update(gs);
         screen_clear_buf();
         render(gs);
         screen_flush();
         usleep(FRAME_DELAY_US);
     }
     free_ent(gs);
 }
 
 /* ---- INPUT (flag-based: all keys read, then actions applied once) ---- */
 static void process_input(GameState*gs){
     int mu=0,md=0,ml=0,mr=0,fire=0;
     while(kb_hit()){
         int k=kb_read_key();
         if(k==27){if(kb_hit()){int k2=kb_read_key();
             if(k2=='['&&kb_hit()){int k3=kb_read_key();
                 if(k3=='A')k='w'; else if(k3=='B')k='s';
                 else if(k3=='C')k='d'; else if(k3=='D')k='a';
                 else continue;
             }else continue;}else continue;}
         if(k=='w'||k=='W') mu=1;
         if(k=='s'||k=='S') md=1;
         if(k=='a'||k=='A') ml=1;
         if(k=='d'||k=='D') mr=1;
         if(k==' ') fire=1;
         if(k=='q'||k=='Q'){gs->mode=STATE_MENU;return;}
         if(k=='p'||k=='P'){
             screen_draw_string(SCREEN_H/2,SCREEN_W/2-5,"= PAUSED =",
                                COL_BRIGHT_YELLOW,COL_RESET);
             screen_flush();
             while(1){if(kb_hit()){kb_read_key();break;}screen_ping_resize();usleep(50000);}
         }
     }
     /* apply movement with per-cell stepping (never skip past obstacles) */
     int spd=PLAYER_SPEED;
     {int s,ci,ok;
      /* UP */
      if(mu) for(s=0;s<spd;s++){
          int ny=gs->player.y-1; if(ny<PLAY_Y) break;
          Rect pr={gs->player.x,ny,PLAYER_W,PLAYER_H}; ok=1;
          for(ci=0;ci<MAX_OBSTACLES;ci++){
              if(!gs->obstacles[ci].active) continue;
              Rect obr={gs->obstacles[ci].x,gs->obstacles[ci].y,
                        gs->obstacles[ci].w,gs->obstacles[ci].h};
              if(check_collision(pr,obr)){ok=0;break;}
          }
          if(!ok) break; gs->player.y=ny;
      }
      /* DOWN */
      if(md) for(s=0;s<spd;s++){
          int ny=gs->player.y+1; if(ny>PLAY_Y+PLAY_H-PLAYER_H) break;
          Rect pr={gs->player.x,ny,PLAYER_W,PLAYER_H}; ok=1;
          for(ci=0;ci<MAX_OBSTACLES;ci++){
              if(!gs->obstacles[ci].active) continue;
              Rect obr={gs->obstacles[ci].x,gs->obstacles[ci].y,
                        gs->obstacles[ci].w,gs->obstacles[ci].h};
              if(check_collision(pr,obr)){ok=0;break;}
          }
          if(!ok) break; gs->player.y=ny;
      }
      /* LEFT */
      if(ml) for(s=0;s<spd;s++){
          int nx=gs->player.x-1; if(nx<PLAYER_MIN_X) break;
          Rect pr={nx,gs->player.y,PLAYER_W,PLAYER_H}; ok=1;
          for(ci=0;ci<MAX_OBSTACLES;ci++){
              if(!gs->obstacles[ci].active) continue;
              Rect obr={gs->obstacles[ci].x,gs->obstacles[ci].y,
                        gs->obstacles[ci].w,gs->obstacles[ci].h};
              if(check_collision(pr,obr)){ok=0;break;}
          }
          if(!ok) break; gs->player.x=nx;
      }
      /* RIGHT */
      if(mr) for(s=0;s<spd;s++){
          int nx=gs->player.x+1; if(nx>PLAYER_MAX_X) break;
          Rect pr={nx,gs->player.y,PLAYER_W,PLAYER_H}; ok=1;
          for(ci=0;ci<MAX_OBSTACLES;ci++){
              if(!gs->obstacles[ci].active) continue;
              Rect obr={gs->obstacles[ci].x,gs->obstacles[ci].y,
                        gs->obstacles[ci].w,gs->obstacles[ci].h};
              if(check_collision(pr,obr)){ok=0;break;}
          }
          if(!ok) break; gs->player.x=nx;
      }
     }
     if(fire) fire_bullet(gs);
 }
 
 /* ---- FIRE (spread = 3 bullets) ---- */
 static void fire_bullet(GameState*gs){
     int cd=gs->player.rapid_timer>0 ? PLAYER_FIRE_CD/2 : PLAYER_FIRE_CD;
     if(gs->player.fire_cd>0) return;
     int spread=gs->player.spread_timer>0;
     int dirs[]={0,-1,1}; int nd=spread?3:1;
     int d;
     for(d=0;d<nd;d++){
         int i;
         for(i=0;i<MAX_BULLETS;i++){
             if(!gs->bullets[i].active){
                 gs->bullets[i].active=1;
                 gs->bullets[i].x=gs->player.x+PLAYER_W;
                 gs->bullets[i].y=gs->player.y+1;
                 gs->bullets[i].dy=dirs[d];
                 gs->bullet_count++;
                 break;
             }
         }
     }
     /* double shot: extra bullet from bottom of ship */
     if(gs->player.double_timer>0){
         int i;
         for(i=0;i<MAX_BULLETS;i++){
             if(!gs->bullets[i].active){
                 gs->bullets[i].active=1;
                 gs->bullets[i].x=gs->player.x+PLAYER_W;
                 gs->bullets[i].y=gs->player.y+PLAYER_H-1;
                 gs->bullets[i].dy=0;
                 gs->bullet_count++;
                 break;
             }
         }
     }
     gs->player.fire_cd=cd;
     sound_play(SND_FIRE);
 }
 
 /* ---- ENEMY FIRE ---- */
 static void enemy_fire(GameState*gs,int idx){
     int i;
     for(i=0;i<MAX_ENEMY_BULLETS;i++){
         if(!gs->enemy_bullets[i].active){
             gs->enemy_bullets[i].active=1;
             gs->enemy_bullets[i].x=gs->enemies[idx].x-1;
             gs->enemy_bullets[i].y=gs->enemies[idx].y+(gs->enemies[idx].is_boss?1:0);
             gs->enemy_bullet_count++;
             return;
         }
     }
 }
 
 /* ---- SPAWNERS ---- */
 /* helper: check if a rect overlaps any active obstacle */
 static int overlaps_obstacle(GameState*gs,int x,int y,int w,int h){
     int j;
     Rect r={x,y,w,h};
     for(j=0;j<MAX_OBSTACLES;j++){
         if(!gs->obstacles[j].active) continue;
         Rect obr={gs->obstacles[j].x,gs->obstacles[j].y,
                   gs->obstacles[j].w,gs->obstacles[j].h};
         if(check_collision(r,obr)) return 1;
     }
     return 0;
 }
 
 static void spawn_enemy(GameState*gs){
     int i;
     for(i=0;i<MAX_ENEMIES;i++){
         if(!gs->enemies[i].active){
             /* try up to 8 positions to avoid spawning behind obstacles */
             int sx=PLAY_X+PLAY_W-ENEMY_W;
             int sy,attempt,ok=0;
             for(attempt=0;attempt<8;attempt++){
                 sy=custom_rand_range(PLAY_Y+2,PLAY_Y+PLAY_H-ENEMY_H-2);
                 if(!overlaps_obstacle(gs,sx,sy,ENEMY_W,ENEMY_H)){ok=1;break;}
             }
             if(!ok) return; /* no clear slot, skip this spawn */
             gs->enemies[i].active=1;
             gs->enemies[i].x=sx;
             gs->enemies[i].y=sy;
             gs->enemies[i].hp=1; gs->enemies[i].is_boss=0;
             gs->enemies[i].speed_quarter=ENEMY_BASE_SPD_Q+(gs->level-1)*ENEMY_SPD_INC_Q;
             gs->enemies[i].move_acc=0;
             gs->enemies[i].color=ecolor(gs->level);
             gs->enemies[i].fire_cd=custom_rand_range(40,ENEMY_FIRE_CD_BASE);
             gs->enemy_count++;
             return;
         }
     }
 }
 
 static void spawn_boss(GameState*gs){
     int i;
     for(i=0;i<MAX_ENEMIES;i++){
         if(!gs->enemies[i].active){
             int sx=PLAY_X+PLAY_W-BOSS_W;
             int sy=PLAY_Y+PLAY_H/2-BOSS_H/2;
             /* find clear Y for boss */
             int attempt;
             for(attempt=0;attempt<8;attempt++){
                 if(!overlaps_obstacle(gs,sx,sy,BOSS_W,BOSS_H)) break;
                 sy=custom_rand_range(PLAY_Y+2,PLAY_Y+PLAY_H-BOSS_H-2);
             }
             gs->enemies[i].active=1;
             gs->enemies[i].x=sx;
             gs->enemies[i].y=sy;
             gs->enemies[i].hp=BOSS_HP+gs->level;
             gs->enemies[i].is_boss=1;
             gs->enemies[i].speed_quarter=2;
             gs->enemies[i].move_acc=0;
             gs->enemies[i].color=COL_BRIGHT_RED;
             gs->enemies[i].fire_cd=15;
             gs->enemy_count++;
             return;
         }
     }
 }
 
 static void spawn_terrain(GameState*gs){
     /* top wall segment — thin (1 row, teeth=2 max) */
     int i;
     for(i=0;i<MAX_OBSTACLES;i++){
         if(!gs->obstacles[i].active){
             int th=1;
             if(custom_rand_range(1,5)==1) th=2;  /* 20% teeth */
             gs->obstacles[i].active=1;
             gs->obstacles[i].type=OBS_WALL_TOP;
             gs->obstacles[i].w=custom_rand_range(6,14);
             gs->obstacles[i].h=th;
             gs->obstacles[i].x=PLAY_X+PLAY_W-gs->obstacles[i].w;
             gs->obstacles[i].y=PLAY_Y;
             gs->obstacle_count++;
             break;
         }
     }
     /* bottom wall segment */
     for(i=0;i<MAX_OBSTACLES;i++){
         if(!gs->obstacles[i].active){
             int bh=1;
             if(custom_rand_range(1,5)==1) bh=2;
             gs->obstacles[i].active=1;
             gs->obstacles[i].type=OBS_WALL_BOT;
             gs->obstacles[i].w=custom_rand_range(6,14);
             gs->obstacles[i].h=bh;
             gs->obstacles[i].x=PLAY_X+PLAY_W-gs->obstacles[i].w;
             gs->obstacles[i].y=PLAY_Y+PLAY_H-gs->obstacles[i].h;
             gs->obstacle_count++;
             break;
         }
     }
 }
 
 static void spawn_asteroid(GameState*gs){
     int i;
     for(i=0;i<MAX_OBSTACLES;i++){
         if(!gs->obstacles[i].active){
             gs->obstacles[i].active=1;
             gs->obstacles[i].type=OBS_ASTEROID;
             gs->obstacles[i].w=custom_rand_range(2,4);
             gs->obstacles[i].h=custom_rand_range(2,3);
             gs->obstacles[i].x=PLAY_X+PLAY_W-gs->obstacles[i].w;
             gs->obstacles[i].y=custom_rand_range(PLAY_Y+3,PLAY_Y+PLAY_H-4);
             gs->obstacle_count++;
             return;
         }
     }
 }
 
 
 
 
 static void add_explosion(GameState*gs,int x,int y){
     int i;
     for(i=0;i<MAX_EXPLOSIONS;i++){
         if(!gs->explosions[i].active){
             gs->explosions[i].active=1;
             gs->explosions[i].x=x; gs->explosions[i].y=y;
             gs->explosions[i].timer=EXPLOSION_TICKS;
             gs->explosion_count++;
             return;
         }
     }
 }
 
 /* =============== UPDATE =============== */
 static void update(GameState*gs){
     int i,j;
     gs->tick++;
     if(gs->player.fire_cd>0) gs->player.fire_cd--;
     if(gs->player.invincible>0) gs->player.invincible--;
     if(gs->player.rapid_timer>0) gs->player.rapid_timer--;
     if(gs->player.spread_timer>0) gs->player.spread_timer--;
     if(gs->player.double_timer>0) gs->player.double_timer--;
 
     /* rapid fire: auto-shoot every 3 frames */
     if(gs->player.rapid_timer>0 && custom_mod(gs->tick,3)==0){
         fire_bullet(gs);
     }
     hud_tick();
     sound_tick();
 
     /* ---- bullets RIGHT ---- */
     for(i=0;i<MAX_BULLETS;i++){
         if(!gs->bullets[i].active) continue;
         gs->bullets[i].x+=BULLET_SPEED;
         /* diagonal spread movement every other tick */
         if(gs->bullets[i].dy!=0 && (gs->tick&1))
             gs->bullets[i].y+=gs->bullets[i].dy;
         if(gs->bullets[i].x>=PLAY_X+PLAY_W ||
            gs->bullets[i].y<PLAY_Y || gs->bullets[i].y>=PLAY_Y+PLAY_H){
             gs->bullets[i].active=0; gs->bullet_count--;
         }
     }
 
     /* ---- enemy bullets LEFT ---- */
     for(i=0;i<MAX_ENEMY_BULLETS;i++){
         if(!gs->enemy_bullets[i].active) continue;
         gs->enemy_bullets[i].x-=ENEMY_BULLET_SPEED;
         if(gs->enemy_bullets[i].x<PLAY_X){
             gs->enemy_bullets[i].active=0; gs->enemy_bullet_count--;
         }
     }
 
     /* ---- enemies LEFT ---- */
     for(i=0;i<MAX_ENEMIES;i++){
         if(!gs->enemies[i].active) continue;
         gs->enemies[i].move_acc+=gs->enemies[i].speed_quarter;
         while(gs->enemies[i].move_acc>=4){
             gs->enemies[i].x--; gs->enemies[i].move_acc-=4;
         }
         int ew=gs->enemies[i].is_boss?BOSS_W:ENEMY_W;
         int eh=gs->enemies[i].is_boss?BOSS_H:ENEMY_H;
         if(gs->enemies[i].x+ew<PLAY_X){
             gs->enemies[i].active=0; gs->enemy_count--;
             if(!gs->enemies[i].is_boss) gs->ships_missed++;
             if(gs->ships_missed>=MAX_MISSED){gs->mode=STATE_GAMEOVER;return;}
             continue;
         }
         gs->enemies[i].fire_cd--;
         if(gs->enemies[i].fire_cd<=0){
             enemy_fire(gs,i);
             int lo=custom_max(20,ENEMY_FIRE_CD_BASE-gs->level*4);
             gs->enemies[i].fire_cd=custom_rand_range(lo,ENEMY_FIRE_CD_BASE);
         }
         if(gs->player.invincible==0){
             Rect er={gs->enemies[i].x,gs->enemies[i].y,ew,eh};
             Rect pr={gs->player.x,gs->player.y,PLAYER_W,PLAYER_H};
             if(check_collision(er,pr)){
                 if(gs->player.shield_hits>0){
                     gs->player.shield_hits--;gs->player.invincible=PLAYER_INV_TIME;
                     if(gs->player.shield_hits==0){hud_notify("SHIELD BROKEN!",COL_BRIGHT_RED);sound_play(SND_SHIELD_BREAK);}
                 } else {
                     gs->player.hp-=DMG_ENEMY;gs->player.invincible=PLAYER_INV_TIME;
                     sound_play(SND_HIT);
                 }
                 add_explosion(gs,gs->enemies[i].x,gs->enemies[i].y);
                 gs->enemies[i].active=0;gs->enemy_count--;gs->ships_destroyed++;
                 if(gs->player.hp<=0){gs->player.hp=0;gs->mode=STATE_GAMEOVER;return;}
             }
         }
     }
 
     /* ---- obstacles LEFT ---- */
     for(i=0;i<MAX_OBSTACLES;i++){
         if(!gs->obstacles[i].active) continue;
         if(gs->tick&1) gs->obstacles[i].x--;
         if(gs->obstacles[i].x+gs->obstacles[i].w<PLAY_X){
             gs->obstacles[i].active=0; gs->obstacle_count--;
         }
     }
 
     /* ---- boosts LEFT + collect ---- */
     boost_update(gs->boosts,MAX_BOOSTS,gs->tick,PLAY_X,&gs->boost_count);
     for(i=0;i<MAX_BOOSTS;i++){
         if(!gs->boosts[i].active) continue;
         if(boost_check_collect(&gs->boosts[i],gs->player.x,gs->player.y,PLAYER_W,PLAYER_H)){
             switch(gs->boosts[i].type){
                 case BOOST_HEAL:
                     gs->player.hp+=HEAL_AMOUNT;
                     if(gs->player.hp>PLAYER_MAX_HP)gs->player.hp=PLAYER_MAX_HP;
                     hud_notify("+20 HP RESTORED",COL_BRIGHT_GREEN);
                     sound_play(SND_BOOST);
                     break;
                 case BOOST_RAPID:
                     gs->player.rapid_timer=RAPID_DUR;
                     hud_notify("RAPID FIRE ACTIVATED",COL_BRIGHT_YELLOW);
                     sound_play(SND_BOOST);
                     break;
                 case BOOST_SHIELD:
                     gs->player.shield_hits=SHIELD_MAX_HITS;
                     hud_notify("SHIELD ACTIVATED (5 HITS)",COL_BRIGHT_MAGENTA);
                     sound_play(SND_BOOST);
                     break;
                 case BOOST_DOUBLE:
                     gs->player.double_timer=DOUBLE_DUR;
                     hud_notify("DOUBLE SHOT ACTIVATED",COL_BRIGHT_WHITE);
                     sound_play(SND_BOOST);
                     break;
             }
             gs->boosts[i].active=0;gs->boost_count--;
         }
     }
 
     /* ---- bullet-enemy (swept hitbox) ---- */
     for(i=0;i<MAX_BULLETS;i++){
         if(!gs->bullets[i].active) continue;
         int bx0=gs->bullets[i].x-BULLET_SPEED+1;
         if(bx0<PLAY_X)bx0=PLAY_X;
         Rect br={bx0,gs->bullets[i].y,gs->bullets[i].x-bx0+1,1};
         for(j=0;j<MAX_ENEMIES;j++){
             if(!gs->enemies[j].active) continue;
             int ew=gs->enemies[j].is_boss?BOSS_W:ENEMY_W;
             int eh=gs->enemies[j].is_boss?BOSS_H:ENEMY_H;
             Rect er={gs->enemies[j].x,gs->enemies[j].y,ew,eh};
             if(check_collision(br,er)){
                 gs->bullets[i].active=0;gs->bullet_count--;
                 gs->enemies[j].hp--;
                 if(gs->enemies[j].hp<=0){
                     add_explosion(gs,gs->enemies[j].x,gs->enemies[j].y);
                     sound_play(SND_EXPLODE);
                     gs->enemies[j].active=0;gs->enemy_count--;
                     gs->ships_destroyed++;
                     gs->score+=gs->enemies[j].is_boss?50*gs->level:10;
                     if(gs->score>MAX_SCORE)gs->score=MAX_SCORE;
                     /* 30% chance to drop a boost */
                     if(custom_rand_range(1,100)<=BOOST_DROP_PCT)
                         boost_spawn_at(gs->boosts,MAX_BOOSTS,
                                        gs->enemies[j].x,gs->enemies[j].y,
                                        &gs->boost_count);
                 }
                 break;
             }
         }
     }
 
     /* ---- enemy bullet-player (swept) ---- */
     if(gs->player.invincible==0){
         Rect pr={gs->player.x,gs->player.y,PLAYER_W,PLAYER_H};
         for(i=0;i<MAX_ENEMY_BULLETS;i++){
             if(!gs->enemy_bullets[i].active) continue;
             Rect ebr={gs->enemy_bullets[i].x,gs->enemy_bullets[i].y,
                       ENEMY_BULLET_SPEED+1,1};
             if(check_collision(ebr,pr)){
                 gs->enemy_bullets[i].active=0;gs->enemy_bullet_count--;
                 if(gs->player.shield_hits>0){
                     gs->player.shield_hits--;gs->player.invincible=PLAYER_INV_TIME;
                     if(gs->player.shield_hits==0){hud_notify("SHIELD BROKEN!",COL_BRIGHT_RED);sound_play(SND_SHIELD_BREAK);}
                 } else {
                     gs->player.hp-=DMG_BULLET;gs->player.invincible=PLAYER_INV_TIME;
                     sound_play(SND_HIT);
                 }
                 if(gs->player.hp<=0){gs->player.hp=0;gs->mode=STATE_GAMEOVER;return;}
             }
         }
     }
 
     /* ---- bullet-obstacle ---- */
     for(i=0;i<MAX_OBSTACLES;i++){
         if(!gs->obstacles[i].active) continue;
         Rect obr={gs->obstacles[i].x,gs->obstacles[i].y,
                   gs->obstacles[i].w,gs->obstacles[i].h};
         for(j=0;j<MAX_BULLETS;j++){
             if(!gs->bullets[j].active) continue;
             Rect br={gs->bullets[j].x,gs->bullets[j].y,1,1};
             if(check_collision(br,obr)){gs->bullets[j].active=0;gs->bullet_count--;}
         }
         for(j=0;j<MAX_ENEMY_BULLETS;j++){
             if(!gs->enemy_bullets[j].active) continue;
             Rect ebr={gs->enemy_bullets[j].x,gs->enemy_bullets[j].y,1,1};
             if(check_collision(ebr,obr)){
                 gs->enemy_bullets[j].active=0;gs->enemy_bullet_count--;
             }
         }
     }
 
     /* ---- player-obstacle (BLOCK, don't damage — crush kills) ---- */
     {
         Rect pr={gs->player.x,gs->player.y,PLAYER_W,PLAYER_H};
         for(i=0;i<MAX_OBSTACLES;i++){
             if(!gs->obstacles[i].active) continue;
             Rect obr={gs->obstacles[i].x,gs->obstacles[i].y,
                       gs->obstacles[i].w,gs->obstacles[i].h};
             if(check_collision(pr,obr)){
                 /* push player left out of obstacle */
                 gs->player.x=gs->obstacles[i].x-PLAYER_W;
                 /* crushed off-screen? */
                 if(gs->player.x<PLAY_X){
                     gs->player.hp=0;gs->mode=STATE_GAMEOVER;return;
                 }
             }
         }
         /* also push vertically if still overlapping after horizontal push */
         for(i=0;i<MAX_OBSTACLES;i++){
             if(!gs->obstacles[i].active) continue;
             pr.x=gs->player.x; pr.y=gs->player.y;
             Rect obr2={gs->obstacles[i].x,gs->obstacles[i].y,
                        gs->obstacles[i].w,gs->obstacles[i].h};
             if(check_collision(pr,obr2)){
                 if(gs->obstacles[i].type==OBS_WALL_TOP)
                     gs->player.y=gs->obstacles[i].y+gs->obstacles[i].h;
                 else
                     gs->player.y=gs->obstacles[i].y-PLAYER_H;
                 gs->player.y=custom_clamp(gs->player.y,PLAY_Y,PLAY_Y+PLAY_H-PLAYER_H);
             }
         }
     }
 
     /* ---- spawn timers ---- */
     gs->spawn_timer--;
     if(gs->spawn_timer<=0){spawn_enemy(gs);gs->spawn_timer=gs->spawn_cd;}
 
     gs->terrain_timer--;
     if(gs->terrain_timer<=0){
         spawn_terrain(gs);
         gs->terrain_timer=TERRAIN_SPAWN_CD;
     }
     gs->asteroid_timer--;
     if(gs->asteroid_timer<=0){
         spawn_asteroid(gs);
         gs->asteroid_timer=custom_max(ASTEROID_SPAWN_CD-gs->level*5,50);
     }
 
     /* ---- level ---- */
     {int nl=1+custom_div(gs->score,SCORE_PER_LEVEL);
      if(nl>MAX_LEVEL)nl=MAX_LEVEL;
      if(nl>gs->level){
          gs->level=nl;
          gs->spawn_cd=BASE_SPAWN_CD-gs->level;
          if(gs->spawn_cd<MIN_SPAWN_CD)gs->spawn_cd=MIN_SPAWN_CD;
          if(custom_mod(gs->level,BOSS_INTERVAL)==0){spawn_boss(gs);sound_play(SND_BOSS);}
          sound_play(SND_LEVELUP);
      }
     }
 
     /* ---- explosions ---- */
     for(i=0;i<MAX_EXPLOSIONS;i++){
         if(!gs->explosions[i].active) continue;
         gs->explosions[i].timer--;
         if(gs->explosions[i].timer<=0){
             gs->explosions[i].active=0;gs->explosion_count--;
         }
     }
 }
 
 /* =============== RENDER =============== */
 static void render(GameState*gs){
     int i,r,c;
 
     /* starfield */
     for(i=0;i<5;i++){
         int sr=custom_rand_range(PLAY_Y,PLAY_Y+PLAY_H-1);
         int sc=custom_rand_range(PLAY_X,PLAY_X+PLAY_W-1);
         screen_put(sr,sc,'.',(custom_rand()&1)?COL_WHITE:COL_BLUE,COL_RESET);
     }
 
     /* border */
     screen_draw_box(PLAY_Y-1,PLAY_X-1,PLAY_W+2,PLAY_H+2,COL_BRIGHT_BLUE,COL_RESET);
 
     /* ---- obstacles (terrain + asteroids) ---- */
     for(i=0;i<MAX_OBSTACLES;i++){
         if(!gs->obstacles[i].active) continue;
         int ox=gs->obstacles[i].x,oy=gs->obstacles[i].y;
         int ow=gs->obstacles[i].w,oh=gs->obstacles[i].h;
         if(gs->obstacles[i].type==OBS_ASTEROID){
             /* asteroid: @ chars in orange with yellow edge */
             for(r=0;r<oh;r++)
                 for(c=0;c<ow;c++){
                     int cl=(r==0||r==oh-1||c==0||c==ow-1)?COL_BRIGHT_YELLOW:COL_ORANGE;
                     screen_put(oy+r,ox+c,'@',cl,COL_RESET);
                 }
         } else {
             /* wall: solid # with highlighted inner edge */
             int inner_r=(gs->obstacles[i].type==OBS_WALL_TOP)?oh-1:0;
             for(r=0;r<oh;r++)
                 for(c=0;c<ow;c++){
                     int cl=(r==inner_r)?COL_BRIGHT_RED:COL_RED;
                     char ch=(r==inner_r)?'=':'#';
                     screen_put(oy+r,ox+c,ch,cl,COL_RESET);
                 }
         }
     }
 
     /* ---- enemies ---- */
     for(i=0;i<MAX_ENEMIES;i++){
         if(!gs->enemies[i].active) continue;
         int ex=gs->enemies[i].x,ey=gs->enemies[i].y;
         int ec=gs->enemies[i].color;
         if(gs->enemies[i].is_boss){
             screen_put(ey,ex,  '/',COL_RED,COL_RESET);
             screen_put(ey,ex+1,'{',ec,COL_RESET);
             screen_put(ey,ex+2,'=',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(ey,ex+3,'@',COL_BRIGHT_WHITE,COL_RESET);
             screen_put(ey,ex+4,'=',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(ey,ex+5,'}',ec,COL_RESET);
             screen_put(ey,ex+6,'\\',COL_RED,COL_RESET);
             screen_put(ey+1,ex,'|',ec,COL_RESET);
             screen_put(ey+1,ex+1,'-',ec,COL_RESET);
             screen_put(ey+1,ex+2,'-',ec,COL_RESET);
             screen_put(ey+1,ex+3,'X',COL_BRIGHT_RED,COL_RESET);
             screen_put(ey+1,ex+4,'-',ec,COL_RESET);
             screen_put(ey+1,ex+5,'-',ec,COL_RESET);
             screen_put(ey+1,ex+6,'|',ec,COL_RESET);
             screen_put(ey+2,ex,'\\',COL_RED,COL_RESET);
             screen_put(ey+2,ex+1,'{',ec,COL_RESET);
             screen_put(ey+2,ex+2,'=',COL_ORANGE,COL_RESET);
             screen_put(ey+2,ex+3,'=',COL_ORANGE,COL_RESET);
             screen_put(ey+2,ex+4,'=',COL_ORANGE,COL_RESET);
             screen_put(ey+2,ex+5,'}',ec,COL_RESET);
             screen_put(ey+2,ex+6,'/',COL_RED,COL_RESET);
             {char hb[16]="HP:";char n[8];ns(gs->enemies[i].hp,n);str_concat(hb,n);
              screen_draw_string(ey>PLAY_Y?ey-1:ey+3,ex,hb,COL_BRIGHT_RED,COL_RESET);}
         } else {
             screen_put(ey,  ex,  '<',ec,COL_RESET);
             screen_put(ey,  ex+1,'*',COL_BRIGHT_WHITE,COL_RESET);
             screen_put(ey,  ex+2,'>',ec,COL_RESET);
             screen_put(ey+1,ex,  '/',ec,COL_RESET);
             screen_put(ey+1,ex+1,'^',COL_BRIGHT_WHITE,COL_RESET);
             screen_put(ey+1,ex+2,'\\',ec,COL_RESET);
         }
     }
 
     /* ---- boosts (rendered by boost module) ---- */
     boost_render(gs->boosts, MAX_BOOSTS, gs->tick);
 
     /* ---- player bullets ---- */
     for(i=0;i<MAX_BULLETS;i++){
         if(!gs->bullets[i].active) continue;
         int cl=(gs->tick&1)?COL_BRIGHT_YELLOW:COL_BRIGHT_CYAN;
         screen_put(gs->bullets[i].y,gs->bullets[i].x,'=',cl,COL_RESET);
     }
 
     /* ---- enemy bullets ---- */
     for(i=0;i<MAX_ENEMY_BULLETS;i++){
         if(!gs->enemy_bullets[i].active) continue;
         screen_put(gs->enemy_bullets[i].y,gs->enemy_bullets[i].x,
                    '~',COL_BRIGHT_RED,COL_RESET);
     }
 
     /* ---- player ---- */
     if(!(gs->player.invincible>0 && gs->player.shield_hits<=0 && (gs->tick&2))){
         int px=gs->player.x,py=gs->player.y;
         if(gs->player.shield_hits>0){
             /* shielded sprite: <(A)> */
             screen_put(py,  px+1,'(',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(py,  px+2,'=',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(py,  px+3,'>',COL_BRIGHT_WHITE,COL_RESET);
             screen_put(py+1,px,  '<',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(py+1,px+1,'(',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(py+1,px+2,'A',COL_BRIGHT_WHITE,COL_RESET);
             screen_put(py+1,px+3,')',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(py+2,px+1,'(',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(py+2,px+2,'=',COL_BRIGHT_YELLOW,COL_RESET);
             screen_put(py+2,px+3,'>',COL_BRIGHT_WHITE,COL_RESET);
         } else {
             int sc=COL_CYAN, sg=COL_NEON_GREEN;
             screen_put(py,  px+1,'/',sc,COL_RESET);
             screen_put(py,  px+2,'=',sc,COL_RESET);
             screen_put(py,  px+3,'>',COL_BRIGHT_WHITE,COL_RESET);
             screen_put(py+1,px,  '>',sg,COL_RESET);
             screen_put(py+1,px+1,'=',sg,COL_RESET);
             screen_put(py+1,px+2,'=',sg,COL_RESET);
             screen_put(py+1,px+3,'|',COL_BRIGHT_WHITE,COL_RESET);
             screen_put(py+2,px+1,'\\',sc,COL_RESET);
             screen_put(py+2,px+2,'=',sc,COL_RESET);
             screen_put(py+2,px+3,'>',COL_BRIGHT_WHITE,COL_RESET);
             {int gc=(gs->tick&2)?COL_ORANGE:COL_BRIGHT_YELLOW;
              screen_put(py+1,px,'>',gc,COL_RESET);}
         }
     }
 
     /* ---- explosions ---- */
     {const char fr[]={'#','*','o','.',' '};
      for(i=0;i<MAX_EXPLOSIONS;i++){
          if(!gs->explosions[i].active) continue;
          int ex=gs->explosions[i].x,ey=gs->explosions[i].y;
          int t=EXPLOSION_TICKS-gs->explosions[i].timer;
          int fi=custom_clamp(custom_div(t*5,EXPLOSION_TICKS),0,4);
          char ch=fr[fi];
          int cl=(t<2)?COL_BRIGHT_YELLOW:(t<4)?COL_ORANGE:COL_RED;
          screen_put(ey,ex,ch,cl,COL_RESET);
          screen_put(ey,ex+1,ch,cl,COL_RESET);
          screen_put(ey,ex-1,ch,cl,COL_RESET);
          screen_put(ey-1,ex,ch,cl,COL_RESET);
          screen_put(ey+1,ex,ch,cl,COL_RESET);
      }}
 
     /* ====== HUD (via hud module) ====== */
     { int gun=gs->player.spread_timer>0?2:gs->player.double_timer>0?3:gs->player.rapid_timer>0?1:0;
       hud_draw_top(gs->score,gs->level,gs->player.hp,PLAYER_MAX_HP,
                    gun,gs->player.shield_hits,
                    gs->ships_destroyed,gs->ships_missed,MAX_MISSED);
       hud_draw_bottom(gs->tick,gs->player.rapid_timer,gs->player.spread_timer,
                       gs->player.double_timer,gs->player.shield_hits,
                       gs->enemy_count,gs->high_score);
       hud_draw_notification(gs->tick);
     }
 }
 
 /* =============== GAME OVER =============== */
 static void state_gameover(GameState*gs){
     if(gs->score>gs->high_score) gs->high_score=gs->score;
     sound_play(SND_GAMEOVER);
     free_ent(gs);
     screen_invalidate(); screen_clear_buf();
     screen_draw_box(0,0,SCREEN_W,SCREEN_H,COL_BRIGHT_RED,COL_RESET);
 
     int cy=2,cx=SCREEN_W/2-14;
     screen_draw_string(cy,  cx,"  ____    _    __  __ _____",  COL_BRIGHT_RED,COL_RESET);
     screen_draw_string(cy+1,cx," / ___|  / \\  |  \\/  | ____|",COL_RED,COL_RESET);
     screen_draw_string(cy+2,cx,"| |  _  / _ \\ | |\\/| |  _|",  COL_ORANGE,COL_RESET);
     screen_draw_string(cy+3,cx,"| |_| |/ ___ \\| |  | | |___", COL_YELLOW,COL_RESET);
     screen_draw_string(cy+4,cx," \\____/_/   \\_\\_|  |_|_____|",COL_BRIGHT_YELLOW,COL_RESET);
 
     int ry=cy+6;
     if(gs->player.hp<=0)
         screen_draw_string(ry,cx+2,"Ship destroyed!",COL_BRIGHT_RED,COL_RESET);
     else
         screen_draw_string(ry,cx,"Too many enemies escaped!",COL_BRIGHT_RED,COL_RESET);
 
     int sy=ry+2;
     {char s[48];char n[16];
      str_copy(s,"SCORE:     ");ns(gs->score,n);str_concat(s,n);
      screen_draw_string(sy,cx+2,s,COL_BRIGHT_GREEN,COL_RESET);
      str_copy(s,"HIGH:      ");ns(gs->high_score,n);str_concat(s,n);
      screen_draw_string(sy+1,cx+2,s,COL_BRIGHT_CYAN,COL_RESET);
      str_copy(s,"LEVEL:     ");ns(gs->level,n);str_concat(s,n);
      screen_draw_string(sy+2,cx+2,s,COL_BRIGHT_YELLOW,COL_RESET);
      str_copy(s,"DESTROYED: ");ns(gs->ships_destroyed,n);str_concat(s,n);
      screen_draw_string(sy+3,cx+2,s,COL_BRIGHT_MAGENTA,COL_RESET);
      str_copy(s,"MISSED:    ");ns(gs->ships_missed,n);str_concat(s,n);
      str_concat(s,"/");ns(MAX_MISSED,n);str_concat(s,n);
      screen_draw_string(sy+4,cx+2,s,COL_RED,COL_RESET);
     }
     screen_draw_string(sy+6,cx,"[R] RESTART     [Q] QUIT",COL_BRIGHT_WHITE,COL_RESET);
     screen_flush();
 
     while(1){if(kb_hit()){int k=kb_read_key();
         if(k=='r'||k=='R'){gs->mode=STATE_PLAYING;return;}
         if(k=='q'||k=='Q'){gs->mode=STATE_MENU;return;}
     }screen_ping_resize();usleep(50000);}
 }
 
/* Reconstructed Bierkuehler; included after pong.inc. Not yet play-tested. */
#define BEER_MAX 8u
#define BEER_HOLD 180u
static int16_t beer_temp[BEER_MAX];
static uint16_t beer_hold[BEER_MAX];
static uint8_t beer_done[BEER_MAX];
static uint8_t beer_count, beer_sel, beer_tick;
static void beer_blank(void) {
    uint8_t x,y;
    for(y=0;y<18;y++) for(x=0;x<20;x++) set_bkg_tile_xy(x,y,0);
}
static void beer_draw(void) {
    uint8_t i,j,y,bars;
    print_at(0,0,"BIERKUEHLER LV"); print_num(15,0,level,3);
    print_at(0,1,"LEBEN"); print_num(6,1,lives,1);
    for(i=0;i<beer_count;i++) {
        y=2+i;
        print_at(0,y,i==beer_sel?">":" "); print_num(1,y,i+1,1);
        print_at(3,y,"DIEBELS");
        print_num(11,y,(uint16_t)beer_temp[i]/10,2);
        print_at(13,y,"."); print_num(14,y,(uint16_t)beer_temp[i]%10,1);
        print_at(15,y,"C"); print_at(17,y,beer_done[i]?"OK":"  ");
    }
    print_at(0,11,"ZIEL 5.0-8.0C: 3 SEK");
    print_at(0,12,"TEMP 0"); print_at(17,12,"29");
    bars=(uint8_t)((uint16_t)beer_temp[beer_sel]*18u/290u);
    if(bars>18) bars=18;
    for(j=0;j<18;j++) set_bkg_tile_xy(1+j,13,char_tile(j<bars?'-':'.'));
    print_at(0,14,"HALTEN"); print_num(8,14,beer_hold[beer_sel]*100u/BEER_HOLD,3);
    print_at(0,16,"L/R DOSE  A KUEHLEN"); print_at(0,17,"START PAUSE");
}
static void beer_start_level(void) {
    uint8_t i;
    beer_count=level>=7?BEER_MAX:level+1; beer_sel=0; beer_tick=0;
    for(i=0;i<beer_count;i++) {
        beer_temp[i]=180+i*5; beer_hold[i]=0; beer_done[i]=0;
    }
    beer_blank(); beer_draw();
}
static void beer_wait_key(void) {
    do {prev_keys=keys; wait_vbl_done(); keys=joypad();}
    while(!PRESSED(J_A)&&!PRESSED(J_START));
}
static void beer_new_game(void) {
    uint8_t i,complete,failed,warming,redraw;
    level=start_level; lives=3; reset_powerups();
    for(i=0;i<40;i++) move_sprite(i,0,0);
    beer_start_level();
    while(1) {
        wait_vbl_done(); prev_keys=keys; keys=joypad();
        if(PRESSED(J_START)) {
            beer_blank(); print_at(7,6,"PAUSE");
            print_at(2,9,"START: WEITER"); print_at(2,11,"SELECT: MENU");
            do {prev_keys=keys; wait_vbl_done(); keys=joypad();}
            while(!PRESSED(J_START)&&!PRESSED(J_SELECT));
            if(keys&J_SELECT) {menu_enter(); return;}
            beer_blank(); beer_draw(); continue;
        }
        redraw=0; failed=0;
        if(PRESSED(J_LEFT)) {beer_sel=beer_sel?beer_sel-1:beer_count-1; redraw=1;}
        if(PRESSED(J_RIGHT)) {beer_sel=(beer_sel+1)%beer_count; redraw=1;}
        if(PRESSED(J_A)&&!beer_done[beer_sel]) {
            beer_temp[beer_sel]-=10; snd_paddle(); redraw=1;
            if(beer_temp[beer_sel]<0) failed=1;
        }
        warming=level>=9?6:15-level;
        if(++beer_tick>=warming) {
            beer_tick=0; redraw=1;
            for(i=0;i<beer_count;i++)
                if(!beer_done[i]&&++beer_temp[i]>290) failed=1;
        }
        if(failed) {
            snd_lose(); --lives; beer_blank();
            print_at(4,6,lives?"DOSE KAPUTT":"GAME OVER");
            print_at(3,10,"A/START: WEITER"); beer_wait_key();
            if(!lives) {menu_enter(); return;}
            beer_start_level(); continue;
        }
        complete=1;
        for(i=0;i<beer_count;i++) {
            if(beer_done[i]) continue;
            if(beer_temp[i]>=50&&beer_temp[i]<=80) {
                if(++beer_hold[i]>=BEER_HOLD) {beer_done[i]=1; snd_powerup(); redraw=1;}
            } else beer_hold[i]=0;
            if(!beer_done[i]) complete=0;
        }
        if(complete) {
            snd_win(); beer_blank(); print_at(4,6,"LEVEL CLEAR");
            print_at(3,10,"A/START: WEITER"); beer_wait_key();
            if(level<255) ++level;
            beer_start_level();
        } else if(redraw||beer_tick==0) beer_draw();
    }
}

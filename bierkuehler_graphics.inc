/* Bierkuehler background graphics: 16x24 cans and 8x24 thermometers. */
#define BEER_GFX_BASE BG_TILE_COUNT
#define BEER_THERM_BASE (BEER_GFX_BASE + 6u)
static uint8_t beer_gfx_cache[BEER_MAX];
static uint16_t beer_gfx_hold_cache[BEER_MAX];
static uint8_t beer_gfx_sel_cache;
static uint8_t beer_gfx_tile[16];
static uint8_t beer_can_pixel(uint8_t x,uint8_t y) {
    if(y==0 || y==23) return (x>=3 && x<=12)?3:0;
    if(y==1 || y==22) return (x>=1 && x<=14)?3:0;
    if(x==0 || x==15) return 0;
    if(x==1 || x==14) return 3;
    if(y==2 || y==21) return 1;
    if(y<6) {
        if(y==3 && x>=5 && x<=10) return 3;
        if(y==4 && (x==5 || x==10)) return 3;
        return 1;
    }
    if(y>=8 && y<=17 && x>=3 && x<=12) {
        /* Light D monogram on a dark label. */
        if((x==5 && y>=10 && y<=15) ||
           ((y==10 || y==15) && x>=5 && x<=8) ||
           (x==9 && y>=11 && y<=14)) return 0;
        if(y==8 || y==17) return 1;
        return 3;
    }
    if(x==2 || x==3) return 0;
    if(x<=5) return 1;
    return 2;
}
static void beer_graphics_reset(void) {
    uint8_t i;
    for(i=0;i<BEER_MAX;i++) {beer_gfx_cache[i]=255;beer_gfx_hold_cache[i]=65535u;}
    beer_gfx_sel_cache=255;
}
static void beer_graphics_load(void) {
    uint8_t tx,ty,x,y,v,lo,hi;
    for(ty=0;ty<3;ty++) for(tx=0;tx<2;tx++) {
        for(y=0;y<8;y++) {
            lo=0;hi=0;
            for(x=0;x<8;x++) {
                v=beer_can_pixel(tx*8+x,ty*8+y);
                if(v&1) lo|=(uint8_t)(0x80u>>x);
                if(v&2) hi|=(uint8_t)(0x80u>>x);
            }
            beer_gfx_tile[y*2]=lo;beer_gfx_tile[y*2+1]=hi;
        }
        set_bkg_data(BEER_GFX_BASE+ty*2+tx,1,beer_gfx_tile);
    }
    beer_graphics_reset();
}
static void beer_thermometer(uint8_t dose,uint8_t temp) {
    uint8_t ty,x,y,py,v,lo,hi,fill;
    fill=(uint8_t)((uint16_t)temp*17u/29u);
    for(ty=0;ty<3;ty++) {
        for(y=0;y<8;y++) {
            py=ty*8+y;lo=0;hi=0;
            for(x=0;x<8;x++) {
                v=0;
                if(py==0 && x>=2 && x<=5) v=3;
                if(py>=1 && py<=18 && (x==2 || x==5)) v=3;
                if(py>=1 && py<=18 && (x==3 || x==4)) {
                    if(fill && py>=19-fill) v=3;
                    else if(py>=14 && py<=16) v=1;
                }
                if(x>=6 && (py==1 || py==14 || py==16 || py==18)) v=3;
                if(py>=19 && py<=22 && x>=1 && x<=6) v=3;
                if(py==23 && x>=2 && x<=5) v=3;
                if(v&1) lo|=(uint8_t)(0x80u>>x);
                if(v&2) hi|=(uint8_t)(0x80u>>x);
            }
            beer_gfx_tile[y*2]=lo;beer_gfx_tile[y*2+1]=hi;
        }
        set_bkg_data(BEER_THERM_BASE+dose*3+ty,1,beer_gfx_tile);
    }
}
static void beer_graphics_draw(void) {
    uint8_t i,x,y,tx,ty,temp;
    uint16_t progress;
    print_at(0,0,"BIERKUEHLER");
    print_at(0,1,"LV");print_num(3,1,level,3);
    print_at(7,1,"L");print_num(9,1,lives,1);
    print_at(11,1,"ZIEL 5-8C");
    for(i=0;i<beer_count;i++) {
        x=(i%4)*5;y=2+(i/4)*7;
        temp=(uint8_t)((uint16_t)beer_temp[i]/10u);
        progress=beer_done[i]?181u:beer_hold[i]*100u/BEER_HOLD;
        if(beer_gfx_cache[i]!=temp || beer_gfx_sel_cache!=beer_sel ||
           beer_gfx_hold_cache[i]!=progress) {
            print_at(x,y,i==beer_sel?">":" ");print_num(x+1,y,i+1,1);
            print_at(x+2,y,beer_done[i]?"OK ":"   ");
            for(ty=0;ty<3;ty++) for(tx=0;tx<2;tx++)
                set_bkg_tile_xy(x+tx,y+1+ty,BEER_GFX_BASE+ty*2+tx);
            beer_thermometer(i,temp);
            for(ty=0;ty<3;ty++) set_bkg_tile_xy(x+3,y+1+ty,BEER_THERM_BASE+i*3+ty);
            print_num(x,y+4,temp,2);print_at(x+2,y+4,".");
            print_num(x+3,y+4,(uint16_t)beer_temp[i]%10u,1);print_at(x+4,y+4,"C");
            if(beer_done[i]) print_at(x,y+5,"KALT ");
            else {print_at(x,y+5,"H");print_num(x+1,y+5,progress,3);print_at(x+4,y+5," ");}
            beer_gfx_cache[i]=temp;beer_gfx_hold_cache[i]=progress;
        } else {
            print_num(x+3,y+4,(uint16_t)beer_temp[i]%10u,1);
        }
    }
    beer_gfx_sel_cache=beer_sel;
    print_at(0,16,"L/R DOSE  A KUEHLEN");
    print_at(0,17,"START PAUSE");
}

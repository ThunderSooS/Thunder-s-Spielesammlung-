/* Cached renderer with decorative cooler frame and no instruction labels. */
#define BG_CAN BG_TILE_COUNT
#define BG_THERM (BG_CAN+12u)
#define BG_BAR (BG_CAN+77u)
#define BG_ARROW (BG_BAR+24u)
#define BG_DEGREE (BG_ARROW+1u)
#define BG_FRAME (BG_DEGREE+1u)
static uint8_t beer_gfx_tile[16],beer_gfx_selection,beer_gfx_hud;
static uint16_t beer_gfx_temp[BEER_MAX];
static uint8_t beer_gfx_progress[BEER_MAX],beer_gfx_done[BEER_MAX];
static const uint8_t beer_letters[6][5]={
 {6,5,5,5,6},{7,2,2,2,7},{7,4,6,4,7},
 {6,5,6,5,6},{4,4,4,4,7},{7,4,7,1,7}
}; /* D I E B L S */
static uint8_t beer_can_pixel(uint8_t x,uint8_t y){
 uint8_t edge,k,g,cx,cy;
 edge=(y==0||y==31)?3:(y==1||y==30)?1:0;
 if(x<edge||x>23-edge)return 0;
 if(x==edge||x==23-edge||y==0||y==31)return 3;
 if(y==1||y==30)return 1;
 if(y==2||y==29)return 3;
 if(y>=9&&y<=23){
  if(y>=10&&y<=14){cy=y-10;for(k=0;k<3;k++){cx=5+k*5;if(x>=cx&&x<cx+3)return (beer_letters[k][cy]&(4u>>(x-cx)))?3:0;}}
  if(y>=17&&y<=21){cy=y-17;for(k=0;k<4;k++){cx=4+k*4;g=k==0?3:k==1?2:k==2?4:5;if(x>=cx&&x<cx+3)return (beer_letters[g][cy]&(4u>>(x-cx)))?3:0;}}
  return 0;
 }
 if(x==3||x==4)return 1;
 return 2;
}
static uint8_t beer_therm_tile(uint8_t dose,uint8_t tx,uint8_t ty){
 uint8_t tile=BG_THERM+dose*8+ty*2+tx;
 if(tile>=BG_TILE_COUNT+40u)tile++;
 return tile;
}
static void beer_graphics_reset(void){
 uint8_t i;beer_gfx_selection=255;beer_gfx_hud=0;
 for(i=0;i<BEER_MAX;i++){beer_gfx_temp[i]=65535u;beer_gfx_progress[i]=255;beer_gfx_done[i]=255;}
}
static const uint8_t beer_frame_tiles[48]={
 0xFF,0xFF,0xFF,0x81,0xC3,0x81,0xDB,0x99,0xDB,0x99,0xC3,0x81,0xFF,0x81,0xFF,0xFF,
 0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0xFF,0xFF,
 0x00,0xFF,0x6D,0xFF,0x6D,0xFF,0x6D,0xFF,0x6D,0xFF,0x6D,0xFF,0x00,0xFF,0xFF,0xFF
};
static void beer_frame_draw(void){
 uint8_t x,t;
 for(x=0;x<20;x++){
  t=(x==0||x==19)?BG_FRAME:BG_FRAME+1;
  set_bkg_tile_xy(x,1,t);set_bkg_tile_xy(x,16,t);
  set_bkg_tile_xy(x,17,(x==0||x==19)?BG_FRAME:BG_FRAME+2);
 }
}
static void beer_graphics_load(void){
 uint8_t tx,ty,x,y,v,lo,hi;
 static const uint8_t arrow[16]={0,0,0,0,0xFE,0xFE,0x7C,0x7C,0x38,0x38,0x10,0x10,0,0,0,0};
 static const uint8_t degree[16]={0x18,0x18,0x24,0x24,0x24,0x24,0x18,0x18,0,0,0,0,0,0,0,0};
 for(ty=0;ty<4;ty++)for(tx=0;tx<3;tx++){
  for(y=0;y<8;y++){lo=hi=0;for(x=0;x<8;x++){v=beer_can_pixel(tx*8+x,ty*8+y);if(v&1)lo|=0x80u>>x;if(v&2)hi|=0x80u>>x;}beer_gfx_tile[y*2]=lo;beer_gfx_tile[y*2+1]=hi;}
  set_bkg_data(BG_CAN+ty*3+tx,1,beer_gfx_tile);
 }
 set_bkg_data(BG_ARROW,1,arrow);set_bkg_data(BG_DEGREE,1,degree);
 set_bkg_data(BG_FRAME,3,beer_frame_tiles);beer_graphics_reset();
}
static const uint8_t beer_therm_outline[128]={
 0x01,0x01,0x02,0x02,0x02,0x02,0x0E,0x0E,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
 0x80,0x80,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
 0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
 0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
 0x02,0x02,0x0E,0x0E,0x0F,0x0A,0x0F,0x0A,0x0F,0x0A,0x0F,0x0A,0x0E,0x0E,0x02,0x02,
 0x40,0x40,0x70,0x70,0xF0,0x50,0xF0,0x50,0xF0,0x50,0xF0,0x50,0x70,0x70,0x40,0x40,
 0x02,0x02,0x02,0x02,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x03,0x03,0x00,0x00,
 0x40,0x40,0x40,0x40,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xC0,0xC0,0x00,0x00,
};
static void beer_thermometer(uint8_t dose,uint16_t temp){
 uint8_t tx,ty,y,py,fill,mask,offset;
 fill=(uint8_t)(temp*22u/290u);
 for(ty=0;ty<4;ty++)for(tx=0;tx<2;tx++){
  offset=(ty*2+tx)*16;mask=tx?0x80:0x01;
  for(y=0;y<8;y++){
   beer_gfx_tile[y*2]=beer_therm_outline[offset+y*2];
   beer_gfx_tile[y*2+1]=beer_therm_outline[offset+y*2+1];
   py=ty*8+y;
   if(py>=2&&py<=23&&fill&&py>=24-fill){beer_gfx_tile[y*2]|=mask;beer_gfx_tile[y*2+1]|=mask;}
  }
  set_bkg_data(beer_therm_tile(dose,tx,ty),1,beer_gfx_tile);
 }
}
static void beer_progress_bar(uint8_t dose,uint8_t progress){
 uint8_t tx,x,y,px,mask,border,fill;
 fill=(uint8_t)((uint16_t)progress*22u/100u);
 for(tx=0;tx<3;tx++){
  mask=0;border=tx==0?0x80:tx==2?0x01:0;
  for(x=0;x<8;x++){px=tx*8+x;if(px>=1&&px<=fill)mask|=0x80u>>x;}
  for(y=0;y<8;y++){
   if(y==1||y==5){beer_gfx_tile[y*2]=255;beer_gfx_tile[y*2+1]=255;}
   else if(y>=2&&y<=4){beer_gfx_tile[y*2]=border|mask;beer_gfx_tile[y*2+1]=border|(y==2?0:mask);}
   else {beer_gfx_tile[y*2]=0;beer_gfx_tile[y*2+1]=0;}
  }
  set_bkg_data(BG_BAR+dose*3+tx,1,beer_gfx_tile);
 }
}
static void beer_graphics_draw(void){
 uint8_t i,x,y,tx,ty,p,first;
 uint16_t temp,old;
 if(!beer_gfx_hud){
  print_at(0,0,"LV");print_num(3,0,level,3);print_at(12,0,"LEBEN");print_num(19,0,lives,1);
  beer_frame_draw();beer_gfx_hud=1;
 }
 for(i=0;i<beer_count;i++){
  x=(i%4)*5;y=2+(i/4)*7;old=beer_gfx_temp[i];first=old==65535u;temp=(uint16_t)beer_temp[i];
  if(first){
   for(ty=0;ty<4;ty++)for(tx=0;tx<3;tx++)set_bkg_tile_xy(x+tx,y+1+ty,BG_CAN+ty*3+tx);
   for(ty=0;ty<4;ty++)for(tx=0;tx<2;tx++)set_bkg_tile_xy(x+3+tx,y+1+ty,beer_therm_tile(i,tx,ty));
   for(tx=0;tx<3;tx++)set_bkg_tile_xy(x+tx,y+6,BG_BAR+i*3+tx);
   print_num(x,y,i+1,1);set_bkg_tile_xy(x+2,y+5,BG_DEGREE);print_at(x+3,y+5,"C");
  }
  if(first||beer_gfx_selection!=beer_sel)set_bkg_tile_xy(x+1,y,(i==beer_sel)?BG_ARROW:0);
  if(first||beer_gfx_done[i]!=beer_done[i]){print_at(x+2,y,beer_done[i]?"OK":"  ");beer_gfx_done[i]=beer_done[i];}
  if(first||old!=temp){
   if(first||old*22u/290u!=temp*22u/290u)beer_thermometer(i,temp);
   if(first||old/10u!=temp/10u)print_num(x,y+5,temp/10u,2);
   beer_gfx_temp[i]=temp;
  }
  p=beer_done[i]?100:(uint8_t)(beer_hold[i]*100u/BEER_HOLD);
  if(first||beer_gfx_progress[i]!=p){
   if(first||(uint16_t)beer_gfx_progress[i]*22u/100u!=(uint16_t)p*22u/100u)beer_progress_bar(i,p);
   beer_gfx_progress[i]=p;
  }
 }
 beer_gfx_selection=beer_sel;
}

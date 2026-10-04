/* Large artwork and two-can paging based on the user's reference. */
#define BG_CAN BG_TILE_COUNT
#define BG_THERM (BG_CAN+12u)
#define BG_BAR (BG_THERM+14u)
#define BG_ARROW (BG_BAR+6u)
#define BG_DEGREE (BG_ARROW+1u)
static uint8_t beer_gfx_page,beer_gfx_tile[16];
static uint8_t beer_gfx_temp[2],beer_gfx_progress[2],beer_gfx_selection;
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
static void beer_graphics_reset(void){
 beer_gfx_page=255;beer_gfx_selection=255;
 beer_gfx_temp[0]=beer_gfx_temp[1]=255;
 beer_gfx_progress[0]=beer_gfx_progress[1]=255;
}
static void beer_graphics_load(void){
 uint8_t tx,ty,x,y,v,lo,hi;
 static const uint8_t arrow[16]={0,0,0,0,0xFE,0xFE,0x7C,0x7C,0x38,0x38,0x10,0x10,0,0,0,0};
 static const uint8_t degree[16]={0x18,0x18,0x24,0x24,0x24,0x24,0x18,0x18,0,0,0,0,0,0,0,0};
 for(ty=0;ty<4;ty++)for(tx=0;tx<3;tx++){
  for(y=0;y<8;y++){lo=hi=0;for(x=0;x<8;x++){v=beer_can_pixel(tx*8+x,ty*8+y);if(v&1)lo|=0x80u>>x;if(v&2)hi|=0x80u>>x;}beer_gfx_tile[y*2]=lo;beer_gfx_tile[y*2+1]=hi;}
  set_bkg_data(BG_CAN+ty*3+tx,1,beer_gfx_tile);
 }
 set_bkg_data(BG_ARROW,1,arrow);set_bkg_data(BG_DEGREE,1,degree);beer_graphics_reset();
}
static void beer_thermometer(uint8_t slot,uint16_t temp){
 uint8_t ty,x,y,py,v,lo,hi,fill;
 uint8_t goal_top=48u-(80u*46u/290u);
 uint8_t goal_bottom=48u-(50u*46u/290u);
 fill=(uint8_t)(temp*46u/290u);
 for(ty=0;ty<7;ty++){
  for(y=0;y<8;y++){py=ty*8+y;lo=hi=0;
   for(x=0;x<8;x++){v=0;
    /* Fixed target band, independent of the moving mercury. */
    if(py>=goal_top&&py<=goal_bottom)v=1;
    if(py==0&&x>=3&&x<=4)v=3;
    if(py>=1&&py<=49&&(x==2||x==5))v=3;
    if(py>=2&&py<=47&&(x==3||x==4)&&fill&&py>=48-fill)v=3;
    if(x<=2&&py==7)v=3;
    if(x>=5&&py==47)v=3;
    /* Black brackets surround the 5-8 C band on both sides. */
    if((py==goal_top-1||py==goal_bottom+1)&&(x<=1||x>=6))v=3;
    if(py>=goal_top-1&&py<=goal_bottom+1&&(x==0||x==7))v=3;
    if(py>=50&&py<=53&&x>=1&&x<=6)v=3;
    if(py==54&&x>=2&&x<=5)v=3;
    if(v&1)lo|=0x80u>>x;if(v&2)hi|=0x80u>>x;
   }beer_gfx_tile[y*2]=lo;beer_gfx_tile[y*2+1]=hi;
  }set_bkg_data(BG_THERM+slot*7+ty,1,beer_gfx_tile);
 }
}
static void beer_progress_bar(uint8_t slot,uint8_t progress){
 uint8_t tx,x,y,px,v,lo,hi,fill;
 fill=(uint8_t)((uint16_t)progress*22u/100u);
 for(tx=0;tx<3;tx++){
  for(y=0;y<8;y++){lo=hi=0;
   for(x=0;x<8;x++){px=tx*8+x;v=0;
    if(y==1||y==5)v=3;
    if(y>=2&&y<=4&&(px==0||px==23))v=3;
    if(y>=2&&y<=4&&px>=1&&px<=fill)v=(y==2)?1:3;
    if(v&1)lo|=0x80u>>x;if(v&2)hi|=0x80u>>x;
   }beer_gfx_tile[y*2]=lo;beer_gfx_tile[y*2+1]=hi;
  }set_bkg_data(BG_BAR+slot*3+tx,1,beer_gfx_tile);
 }
}
static void beer_graphics_draw(void){
 uint8_t page,slot,i,x,y,tx,ty,p,first;
 page=beer_sel/2;first=beer_gfx_page!=page;
 if(first){
  for(y=0;y<18;y++)for(x=0;x<20;x++)set_bkg_tile_xy(x,y,0);
  for(y=1;y<=2;y++)for(x=0;x<20;x++)set_bkg_tile_xy(x,y,TILE_BRICK0+(y-1)*2+(x&1));
  print_at(0,0,"LV");print_num(3,0,level,3);print_at(12,0,"LEBEN");print_num(19,0,lives,1);
  print_at(2,13,"DOSEN");print_num(8,13,page*2+1,1);print_at(9,13,"-");
  print_num(10,13,(page*2+2>beer_count)?beer_count:page*2+2,1);print_at(12,13,"VON");print_num(16,13,beer_count,1);
  print_at(0,15,"<0C ODER >29C:PENG");print_at(1,16,"ZIEL 5-8C 3 SEK.");print_at(0,17,"A:KUEHLEN <>:DOSE");
  beer_gfx_temp[0]=beer_gfx_temp[1]=255;beer_gfx_progress[0]=beer_gfx_progress[1]=255;
 }
 for(slot=0;slot<2;slot++){
  i=page*2+slot;if(i>=beer_count)continue;x=6+slot*4;
  if(first){
   for(ty=0;ty<4;ty++)for(tx=0;tx<3;tx++)set_bkg_tile_xy(x+tx,5+ty,BG_CAN+ty*3+tx);
   for(ty=0;ty<7;ty++)set_bkg_tile_xy(x+3,3+ty,BG_THERM+slot*7+ty);
   for(tx=0;tx<3;tx++)set_bkg_tile_xy(x+tx,11,BG_BAR+slot*3+tx);
  }
  if(first||beer_gfx_selection!=beer_sel)set_bkg_tile_xy(x+1,4,(i==beer_sel)?BG_ARROW:0);
  if(first||beer_gfx_temp[slot]!=(uint8_t)beer_temp[i]){
   beer_thermometer(slot,(uint16_t)beer_temp[i]);beer_gfx_temp[slot]=(uint8_t)beer_temp[i];
  }
  print_num(x,10,(uint16_t)beer_temp[i]/10,2);set_bkg_tile_xy(x+2,10,BG_DEGREE);print_at(x+3,10,"C");
  p=beer_done[i]?100:(uint8_t)(beer_hold[i]*100u/BEER_HOLD);
  if(first||beer_gfx_progress[slot]!=p){beer_progress_bar(slot,p);beer_gfx_progress[slot]=p;}
  print_at(x,12,beer_done[i]?"KALT":"    ");
 }
 beer_gfx_page=page;beer_gfx_selection=beer_sel;
}

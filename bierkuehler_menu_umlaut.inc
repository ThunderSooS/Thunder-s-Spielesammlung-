/* Menu title with a real uppercase U umlaut, without changing FONT_CHARS. */
#define BEER_MENU_UMLAUT_TILE (BG_TILE_COUNT+40u)
static void beer_menu_print_title(uint8_t x,uint8_t y){
 static const uint8_t u[16]={0x24,0x24,0,0,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x38,0x38};
 set_bkg_data(BEER_MENU_UMLAUT_TILE,1,u);
 print_at(x,y,"BIERK");set_bkg_tile_xy(x+5,y,BEER_MENU_UMLAUT_TILE);print_at(x+6,y,"HLER");
}

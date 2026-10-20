/* SPDX-License-Identifier: GPL-3.0-only
 * Device scenes ported from design/APPROVED_24_INTERFACES.html, playerHtml().
 * The device IS the layout: metadata, lyrics, transport and audio readouts
 * occupy its own windows. Static chassis is cached; no web runtime on device.
 * Coordinates are the Brick Pro 1024x768 design space, scaled by u(). */
#include "nxt_eq.h"
#include "nxt_controls.h"
typedef struct {int x,y,w,h;} Box;
typedef struct {
 Box song,lyrics,spectrum,progress,transport,art;
 int volume_x,volume_y,song_palette,lyric_palette;
} Scene;
static const Scene scenes[24]={
 {0},
 /* Cassette: label -> wide tape -> lyrics -> spectrum -> mechanical keys. */
 {{150,191,790,74},{66,394,892,101},{70,502,884,28},{68,540,888,26},{68,576,350,40},{70,193,66,66},663,578,0,0},
 /* Radio: grille / tuner and paper LCD. */
 {{350,273,566,77},{347,354,570,111},{356,473,552,31},{64,528,892,28},{64,567,350,42},{0},660,570,2,2},
 /* Walkman: door with tape and lyrics; vertical side keys. */
 {{139,208,636,74},{139,397,636,108},{147,513,620,30},{68,553,886,25},{828,247,80,185},{65,209,61,61},687,584,-1,-1},
 /* Open reel: two reels; meters / green lyrics console. */
 {{478,342,462,74},{478,418,462,107},{70,537,880,21},{68,568,600,25},{69,459,350,42},{0},699,582,1,1},
 /* Hi-Fi: amplifier, music deck, transport rack. */
 {{150,316,771,73},{71,396,867,106},{76,509,857,27},{64,549,892,25},{64,585,350,38},{75,320,62,62},661,584,1,1},
 /* CD: tray / green music LCD. */
 {{511,211,421,86},{510,306,422,130},{520,453,402,35},{67,525,886,26},{67,566,350,44},{228,329,61,61},665,572,1,1},
 /* Jukebox: selection paper, tubes, centered lyrics and keys. */
 {{180,240,682,82},{124,335,776,119},{135,461,754,33},{112,503,800,26},{291,542,442,42},{125,247,63,63},0,0,6,-1},
 /* CRT: song / lyrics split within phosphor terminal. */
 {{88,230,421,90},{535,223,402,183},{92,345,407,110},{87,447,844,26},{69,564,480,42},{0},702,577,5,5},
 /* Turntable: platter / lyrics console. */
 {{490,214,443,86},{490,313,443,142},{497,469,429,39},{67,527,886,27},{67,566,350,44},{74,239,305,305},665,573,-1,-1},
 /* Focus: large title and airy lyrics. */
 {{224,209,720,89},{72,347,880,153},{77,509,869,20},{73,548,878,26},{72,584,350,39},{76,205,128,128},685,583,-1,-1},
 /* Ambient: artwork / translucent lyrics; integrated dock. */
 {{450,216,482,87},{450,316,482,173},{76,506,867,31},{75,551,873,25},{84,587,350,37},{98,227,291,291},669,586,-1,-1},
 /* Boombox: upper keys; twin cones / track display; lyrics below. */
 {{299,310,426,77},{70,469,880,87},{304,402,416,39},{70,568,880,25},{72,211,350,43},{0},691,214,1,-1},
 /* MiniDisc: cartridge / green LCD. */
 {{500,212,429,85},{500,309,429,125},{508,451,413,48},{69,526,886,26},{69,567,350,42},{173,349,92,92},668,572,3,3},
 /* Pocket: left green screen, right click wheel with real key labels. */
 {{153,223,391,88},{77,336,467,152},{83,498,455,20},{80,547,461,28},{0},{80,221,61,61},724,541,4,4},
 /* Classic: time/track, EQ rack, lyrics, queue window. */
 {{269,202,670,63},{69,416,868,99},{274,269,664,31},{72,310,865,23},{68,348,350,36},{0},680,350,5,5},
 /* Valve amplifier: four tubes, meters / lyrics. */
 {{150,364,323,80},{532,359,395,164},{80,524,838,24},{67,560,600,24},{67,589,350,31},{75,370,61,61},669,580,-1,-1},
 /* Gramophone: horn over platter, paper lyrics. */
 {{493,227,434,91},{493,336,434,163},{82,518,850,23},{67,554,885,25},{67,587,350,37},{92,391,277,277},667,586,-1,6},
 /* Car deck: left presets, central LCD, right volume knob. */
 {{180,230,651,87},{180,328,651,124},{188,465,635,35},{67,531,885,26},{67,571,350,43},{0},660,577,5,5},
 /* Synth: LCD / nine genre pads, piano keys under transport. */
 {{82,219,493,78},{82,307,493,123},{88,442,481,39},{67,500,885,24},{67,530,350,37},{0},676,452,1,1},
 /* Scope: signal grid with song, waveform and lyrics; side knobs. */
 {{92,215,698,83},{92,418,698,107},{96,309,690,99},{94,539,694,23},{67,580,350,35},{0},671,578,5,5},
 /* Bento: album/song tile, lyric/spectrum tile, full-width control tile. */
 {{68,426,355,78},{462,205,476,228},{470,451,459,76},{69,550,885,25},{69,587,350,34},{136,199,210,210},677,586,-1,-1},
 /* E-Ink: journal masthead, album/title, centered lyrics, ink controls. */
 {{228,225,715,91},{79,352,866,153},{84,522,856,18},{73,555,878,26},{73,589,350,35},{77,220,128,128},686,587,-1,-1},
 /* Hologram: album orbit / glass lyrics; floating dock. */
 {{501,222,428,86},{501,324,428,152},{509,488,412,31},{75,548,873,25},{85,588,350,34},{116,292,245,245},669,585,-1,-1}
};
static const unsigned window_pal[][4]={
 {0xebdcb6,0x302c24,0x61553e,0x8a774e},
 {0x182721,0xd7eab7,0xa6be94,0x56714b},
 {0xd1bd89,0x3c3424,0x5e4e35,0x9e8155},
 {0x90ab9b,0x21362b,0x354e3f,0x577c64},
 {0xe3eacb,0x2e4232,0x5c705a,0x83977f},
 {0x152f23,0xc8e9ad,0xa3ca91,0x628253},
 {0xe4d6b3,0x3c2d21,0x715843,0x8c6d45}
};
static SDL_Color wc(int palette,int role){return palette>=0?rgb(window_pal[palette][role]):NxtSkin_color(role==0?NXT_BG:role==1?NXT_INK:role==2?NXT_MUTED:NXT_LINE);}
static void window_box(SDL_Surface *s,Box b,int palette,int radius){rect(s,b.x-5,b.y-5,b.w+10,b.h+10,wc(palette,3),radius+2);rect(s,b.x-3,b.y-3,b.w+6,b.h+6,wc(palette,0),radius);}
static void inset(SDL_Surface *s,int x,int y,int w,int h,unsigned edge,unsigned body,int radius){rect(s,x,y,w,h,rgb(edge),radius);rect(s,x+5,y+5,w-10,h-10,rgb(body),radius>5?radius-4:1);}
static void grain(SDL_Surface *s,int x,int y,int w,int h,unsigned base,unsigned stripe){rect(s,x,y,w,h,rgb(base),8);for(int j=3;j<h-4;j+=6)rect(s,x+6,y+j,w-12,2,rgb(stripe),0);}
static void speaker(SDL_Surface *s,int cx,int cy,int r){circle(s,cx,cy,r,rgb(0x202b34));for(int j=r-8;j>r/3;j-=5)circle(s,cx,cy,j,rgb(j%2?0x40525c:0x18272e));circle(s,cx,cy,r/3,rgb(0x819ba3));circle(s,cx,cy,r/4,rgb(0x354954));}
static void dot_grille(SDL_Surface *s,int x,int y,int w,int h){inset(s,x,y,w,h,0x483929,0xb3a17c,24);for(int yy=y+10;yy<y+h-8;yy+=7)for(int xx=x+10;xx<x+w-8;xx+=7)rect(s,xx,yy,2,2,rgb(0x322c22),0);}
static void meter_box(SDL_Surface *s,int x,int y,int w,int h,const char *label){inset(s,x,y,w,h,0x2b322e,0xe6d7a6,5);for(int k=0;k<15;k++)rect(s,x+13+k*(w-27)/14,y+15,1,k%4?9:15,rgb(0x686349),0);text(s,15,"−20       −5       0     +3",x+12,y+4,w-24,rgb(0x292e25));text(s,16,label,x+w/2-27,y+h-28,80,rgb(0x292e25));}
static void vu_pair(SDL_Surface *s,int x,int y,int w,int h){meter_box(s,x,y,(w-12)/2,h,"L · VU");meter_box(s,x+(w+12)/2,y,(w-12)/2,h,"R · VU");}
static void ellipse(SDL_Surface *s,int cx,int cy,int rx,int ry,SDL_Color ink){for(int y=-ry;y<=ry;y++){int width=(int)(rx*sqrt(1.-(double)y*y/(ry*ry)));rect(s,cx-width,cy+y,2*width+1,1,ink,0);}}
static void horn(SDL_Surface *s){
 static const int vx[]={138,323,362,309,259,206,162,121},vy[]={204,204,233,330,366,346,291,249};int xs[8],ys[8];float a=-13.f*M_PI/180.f;
 for(int i=0;i<8;i++){xs[i]=242+(vx[i]-242)*cosf(a)-(vy[i]-285)*sinf(a);ys[i]=285+(vx[i]-242)*sinf(a)+(vy[i]-285)*cosf(a);}
 for(int y=177;y<381;y++){int crosses[8],n=0;for(int i=0;i<8;i++){int j=(i+1)%8;if((ys[i]<=y&&ys[j]>y)||(ys[j]<=y&&ys[i]>y))crosses[n++]=xs[i]+(y-ys[i])*(xs[j]-xs[i])/(ys[j]-ys[i]);}for(int i=0;i<n;i++)for(int j=i+1;j<n;j++)if(crosses[j]<crosses[i]){int t=crosses[j];crosses[j]=crosses[i];crosses[i]=t;}for(int i=0;i+1<n;i+=2)rect(s,crosses[i],y,crosses[i+1]-crosses[i]+1,1,rgb(y%9<3?0xcda56a:0x96764e),0);}
 ellipse(s,238,237,109,49,rgb(0xdcb878));ellipse(s,238,237,97,41,rgb(0x816335));ellipse(s,238,237,88,35,rgb(0xf2d294));ellipse(s,238,237,81,30,rgb(0xa28348));ellipse(s,238,237,58,21,rgb(0x503d1e));
}
static const char *deck_names[]={"","MIXTAPE / C-60","WOOD RADIO","PERSONAL STEREO","REEL STUDIO","REFERENCE AMPLIFIER","COMPACT DISC DECK","MUSIC DINER","MUSIC TERMINAL","ANALOG TURNTABLE","FOCUS","AMBIENT","STREET BOX","MINI DISC","POCKET / NXT","CLASSIC MUSIC PLAYER","VALVE AMPLIFIER","PHONOGRAPH","CAR AUDIO","SYNTH STUDIO","AUDIO SCOPE","BENTO","MUSIC JOURNAL","HOLOGRAM"};
static SDL_Surface *chassis,*gray_cover;static int chassis_style=-1;
static SDL_Surface *gray_source;static unsigned gray_generation;
static SDL_Surface *eink_art(SDL_Surface *s,SDL_Surface *art,int edge){
 if(!art)return NULL;int size=u(s,edge);unsigned generation=NxtPlayback_generation();
 if(gray_cover&&(gray_source!=art||gray_generation!=generation||gray_cover->w!=size)){SDL_FreeSurface(gray_cover);gray_cover=NULL;}
 if(!gray_cover){gray_cover=SDL_CreateRGBSurfaceWithFormat(0,size,size,32,SDL_PIXELFORMAT_ARGB8888);if(!gray_cover)return art;SDL_BlitScaled(art,NULL,gray_cover,NULL);SDL_LockSurface(gray_cover);for(int y=0;y<size;y++){Uint32 *pixels=(Uint32*)((Uint8*)gray_cover->pixels+y*gray_cover->pitch);for(int x=0;x<size;x++){Uint8 r,g,b,a;SDL_GetRGBA(pixels[x],gray_cover->format,&r,&g,&b,&a);Uint8 v=(r*77+g*150+b*29)>>8;pixels[x]=SDL_MapRGBA(gray_cover->format,v,v,v,a);}}SDL_UnlockSurface(gray_cover);gray_source=art;gray_generation=generation;}
 return gray_cover;
}
static void chassis_draw(SDL_Surface *s,int style){
 SDL_Color panel=NxtSkin_color(NXT_PANEL),ln=NxtSkin_color(NXT_LINE),ink=NxtSkin_color(NXT_INK);
 rect(s,32,144,960,528,ln,18);rect(s,37,149,950,518,panel,14);
 SDL_Rect clip={u(s,37),u(s,149),u(s,950),u(s,518)};SDL_SetClipRect(s,&clip);
 switch(style){
 case 1:
  inset(s,32,144,960,528,0x303b41,0xbda975,17);rect(s,46,158,932,500,rgb(0xd6c594),9);
  for(int j=0;j<4;j++){int x=j%2?975:47,y=j/2?650:158;circle(s,x,y,5,rgb(0x7b8a90));line(s,x-3,y-3,x+3,y+3,rgb(0x223137),2);}
  line(s,66,186,958,186,rgb(0xae4f3f),3);line(s,66,276,958,276,rgb(0xae4f3f),3);
  inset(s,65,286,894,96,0x222a2d,0x182b30,48);rect(s,210,307,604,45,rgb(0x253e40),3);break;
 case 2:
  grain(s,37,149,950,518,0x866748,0x806044);dot_grille(s,65,208,236,296);
  text(s,17,"NXT · FULL RANGE",91,469,191,rgb(0x342c21));inset(s,332,207,611,57,0x483e2e,0xd7c493,4);
  for(int k=0;k<53;k++)rect(s,347+k*11,235,1,k%5?13:21,rgb(0x5d5139),0);window_box(s,(Box){340,270,594,234},2,4);break;
 case 3:
  inset(s,37,149,950,518,0x7b9baf,0x19425e,18);inset(s,62,194,741,359,0x6b8a9c,0x142f45,9);
  inset(s,137,291,640,91,0x89928a,0x263d47,10);dial(s,868,461,33,-1);text(s,14,"MEGA BASS",824,505,110,ink);break;
 case 4:
  inset(s,37,149,950,518,0x313d3e,0xc6d0c8,7);line(s,292,310,350,337,rgb(0x5e4534),4);line(s,732,310,674,337,rgb(0x5e4534),4);
  inset(s,348,320,327,15,0x5e4534,0x526056,3);vu_pair(s,68,345,361,90);window_box(s,(Box){469,337,480,188},1,4);break;
 case 5:
  for(int j=0;j<3;j++){int y=j==0?147:j==1?301:544,h=j==0?154:j==1?243:77;inset(s,37,y,950,h,0x060c0d,0x303c40,3);}
  vu_pair(s,70,201,694,79);dial(s,865,239,45,-1);text(s,16,"DIGITAL MUSIC DECK",65,303,880,ink);window_box(s,(Box){65,314,880,224},1,3);break;
 case 6:
  inset(s,37,149,950,518,0x687774,0xcfd8d1,9);inset(s,68,209,379,303,0x4d5c5b,0x283333,8);
  circle(s,259,360,132,rgb(0xb4c4cb));circle(s,259,360,123,rgb(0xd8ddd4));circle(s,259,360,86,rgb(0x97abaf));circle(s,259,360,25,rgb(0x243334));
  window_box(s,(Box){496,205,450,301},1,4);break;
 case 7:
  inset(s,32,144,960,528,0xe7b879,0x4e3a3c,38);inset(s,96,171,832,58,0xae8460,0xedd2a0,27);
  text(s,27,"NXT · MUSIC DINER",284,178,512,rgb(0x473027));
  for(int k=0;k<2;k++){int x=k?932:61;inset(s,x,240,32,280,0x6c4a44,0xe9b68f,16);for(int y=249;y<509;y+=34)rect(s,x+5,y,22,15,rgb(0xdc9cb1),7);}
  inset(s,112,236,799,87,0x9b7d58,0xedddba,3);rect(s,122,333,779,123,rgb(0x1f2427),5);
  for(int k=0;k<74;k++)rect(s,73+k*12,598,3,15,rgb(0xc5a87d),0);break;
 case 8:
  inset(s,37,149,950,518,0x4b5e4d,0x10201b,27);inset(s,69,207,890,315,0x547e61,0x1b3025,13);
  for(int y=215;y<514;y+=4)rect(s,78,y,872,1,rgb(0x233c2c),0);window_box(s,(Box){530,218,412,193},5,1);break;
 case 9:
  grain(s,37,149,950,518,0x705237,0x6c4d33);inset(s,66,207,384,315,0x9b8057,0x514439,8);
  window_box(s,scenes[9].lyrics,-1,7);dial(s,930,577,23,-.8);break;
 case 10:rect(s,37,149,950,518,rgb(0xedf3ed),20);break;
 case 11:
  rect(s,37,149,950,518,rgb(0x252b48),24);circle(s,279,302,180,rgb(0x55435c));window_box(s,scenes[11].lyrics,-1,15);
  inset(s,69,581,886,40,0x83909e,0x212d3d,16);break;
 case 12:
  inset(s,37,149,950,518,0x28323a,0x657780,15);inset(s,229,157,566,34,0x1c2830,0x44515a,10);rect(s,241,169,542,26,rgb(0x657780),2);
  speaker(s,167,364,95);speaker(s,857,364,95);window_box(s,(Box){287,292,450,161},1,6);window_box(s,scenes[12].lyrics,-1,7);break;
 case 13:
  inset(s,37,149,950,518,0x6e8187,0xb3c5c4,22);inset(s,69,209,379,301,0x3b5963,0x517887,16);
  inset(s,122,236,263,236,0x293f48,0x284c5c,12);rect(s,264,253,93,77,rgb(0xb7c4be),2);
  window_box(s,(Box){493,205,444,234},3,4);break;
 case 14:
  inset(s,37,149,950,518,0xa5b3ac,0xe5eadd,25);window_box(s,(Box){70,211,483,372},4,13);
  circle(s,759,366,121,rgb(0x9baaa0));circle(s,759,366,115,rgb(0xc5cdc5));circle(s,759,366,43,rgb(0x91a195));circle(s,759,366,39,rgb(0xedf1e7));break;
 case 15:
  inset(s,37,149,950,518,0xc3c8d1,0x414d60,1);rect(s,55,161,914,29,rgb(0x596b86),0);
  text(s,18,"NXT · CLASSIC MUSIC PLAYER   [ _  □  × ]",193,158,710,rgb(0xeef0df));window_box(s,(Box){64,199,888,105},5,1);
  rect(s,57,390,910,20,rgb(0x596b86),0);text(s,15,"EQUALIZER  /  SELECT",70,388,682,rgb(0xeef0df));
  window_box(s,scenes[15].lyrics,5,1);rect(s,57,530,910,21,rgb(0x596b86),0);text(s,15,"PLAYLIST  /  X",70,528,862,rgb(0xeef0df));window_box(s,(Box){65,558,890,51},5,1);break;
 case 16:
  inset(s,37,149,950,518,0x382d23,0x836945,7);rect(s,43,156,938,184,rgb(0x1c2424),4);
  for(int k=0;k<4;k++){int x=285+k*121,h=k==1||k==2?123:106,y=327-h;inset(s,x,y,63,h,0x7d8875,0x48483a,24);rect(s,x+21,y+28,21,h-40,rgb(0xe59e67),7);rect(s,x+25,y+33,13,h-49,rgb(0xffe7a6),5);inset(s,x-6,325,75,16,0x333d35,0x5b625a,2);}
  vu_pair(s,75,457,397,59);window_box(s,scenes[16].lyrics,-1,6);break;
 case 17:
  grain(s,37,149,950,518,0x4f3b2c,0x473328);inset(s,69,470,364,54,0xb49362,0x705239,7);
  horn(s);
  line(s,211,365,288,443,rgb(0x96764e),14);window_box(s,scenes[17].lyrics,6,7);break;
 case 18:
  inset(s,37,149,950,518,0x4a555c,0x17242c,21);window_box(s,(Box){169,212,674,294},5,6);
  for(int j=0;j<3;j++){inset(s,68,281+j*59,76,45,0x798282,0x273137,2);text(s,18,j==0?"EQ":j==1?"LỜI":"PHỔ",80,289+j*59,70,ink);}
  dial(s,907,341,35,-1);text(s,18,"VOL",885,389,64,ink);break;
 case 19:
  inset(s,37,149,950,518,0x8b9d90,0xd9dfd6,7);window_box(s,(Box){73,206,511,281},1,4);
  for(int j=0;j<9;j++){int x=629+(j%3)*105,y=218+(j/3)*70;inset(s,x,y,96,58,0x779587,0xa4b9a8,5);text(s,17,NxtEQ_name(j),x+8,y+18,80,rgb(0x253d34));}
  text(s,17,"SELECT → EQ",632,434,292,rgb(0x253d34));
  for(int k=0;k<15;k++){int x=72+k*59;rect(s,x,575,54,36,rgb(0xeceddb),1);if(k%3!=2)rect(s,x+38,575,26,24,rgb(0x263b32),1);}break;
 case 20:
  inset(s,37,149,950,518,0xaab6a4,0x556b60,12);window_box(s,(Box){78,203,726,366},5,19);
  for(int x=90;x<799;x+=35)rect(s,x,213,1,349,rgb(0x2e4932),0);for(int y=217;y<563;y+=35)rect(s,88,y,709,1,rgb(0x2e4932),0);
  dial(s,906,300,33,-1);text(s,16,"LEVEL",878,344,83,ink);dial(s,906,448,33,-2);text(s,16,"TIME",880,491,78,ink);break;
 case 21:
  rect(s,37,149,950,518,rgb(0xd8e2da),20);rect(s,54,186,384,330,rgb(0xf2f5ee),17);rect(s,452,186,510,330,rgb(0xf2f5ee),17);rect(s,54,536,908,83,rgb(0xb6d0c0),17);break;
 case 22:
  rect(s,37,149,950,518,rgb(0xeee9dc),1);rect(s,54,155,916,3,rgb(0x514e42),0);rect(s,54,161,916,1,rgb(0x514e42),0);line(s,67,207,956,207,rgb(0x777164),1);
  text(s,22,"NXT / MUSIC JOURNAL",332,170,609,rgb(0x302f2b));break;
 case 23:
  inset(s,37,149,950,518,0x6294a3,0x122434,23);for(int j=0;j<4;j++){int radius=160-j*17;circle(s,262,366,radius,rgb(j%2?0x182d3e:0x466c71));}circle(s,262,366,103,rgb(0x203547));window_box(s,scenes[23].lyrics,-1,17);inset(s,69,581,886,40,0x6898a6,0x213544,16);break;
 }
 if(style!=7&&style!=15&&style!=21&&style!=22)text(s,18,deck_names[style],67,158,675,style==1?rgb(0x302c24):ink);
 if(style!=21&&style!=7&&style!=15&&style!=22)text(s,15,"NXT / STEREO",800,159,161,style==1?rgb(0x61553e):NxtSkin_color(NXT_MUTED));
 if(style!=14&&style!=20&&style!=2&&style!=6&&style!=9&&style!=13&&style!=19&&style!=18&&style!=4&&style!=5&&style!=8&&style!=15&&style!=21&&style!=22)
  window_box(s,scenes[style].lyrics,scenes[style].lyric_palette,style==1?3:7);
 line(s,64,619,960,619,nxt.interface_style==1?rgb(0x8a774e):NxtSkin_color(NXT_LINE),1);
 SDL_SetClipRect(s,NULL);
}
static const char *action_key(int action){for(int i=0;i<16;i++)if(nxt.playing_keys[i]==action)return Nxt_keyNames[i];return "—";}
static int scene_wrapped(SDL_Surface *s,int px,const char *value,int x,int y,int w,int h,SDL_Color ink){
 int line;TTF_Font *font;do{font=NxtSkin_font(s,px);int height=0;TTF_SizeUTF8(font,value,NULL,&height);line=(height>TTF_FontHeight(font)?height:TTF_FontHeight(font))+3;if(line<=u(s,h)||px<=12)break;px--;}while(true);
 return Nxt_drawWrapped(s,font,value,u(s,x),u(s,y),u(s,w),u(s,h),ink);
}
static void song_draw(SDL_Surface *s,Scene f){
 const TrackInfo *t=NxtPlayback_info();SDL_Color ink=wc(f.song_palette,1),muted=wc(f.song_palette,2);Box b=f.song;
 const char *source=NxtSoundCloud_hasMetadata()?"SOUNDCLOUD":NxtSpotify_hasMetadata()?"SPOTIFY":Nxt_text("SD MUSIC","NHẠC TRÊN THẺ");text(s,15,source,b.x,b.y,b.w,muted);
 const char *title=t->title[0]?t->title:Nxt_text("Choose your music","Chọn nhạc của bạn");
 int px=b.h>=85?29:25,titleheight=b.h>=85?54:40;
 scene_wrapped(s,px,title,b.x,b.y+18,b.w,titleheight,ink);
 char byline[768];snprintf(byline,sizeof(byline),"%s%s%s",t->artist[0]?t->artist:Nxt_text("Unknown artist","Chưa rõ nghệ sĩ"),t->album[0]?" · ":"",t->album);text(s,17,byline,b.x,b.y+b.h-15,b.w,muted);
}
static void lyrics_draw(SDL_Surface *s,Scene f){
 Box b=f.lyrics;SDL_Color ink=wc(f.lyric_palette,1),muted=wc(f.lyric_palette,2);char label[96];snprintf(label,sizeof(label),"%s     %s · %s",Nxt_text("LYRICS","LỜI BÀI HÁT"),action_key(NC_LYRICS),Nxt_text("Full screen","Toàn màn hình"));text(s,14,label,b.x+10,b.y+2,b.w-20,muted);
 int index=Lyrics_isSynced()?Lyrics_currentIndex(NxtPlayback_position()):0;if(index<0)index=0;
 const char *current=Settings_getBool(&SETTING_LYRICS_ENABLED)?Lyrics_lineText(index):NULL;
 if(!current){const char *status=!Settings_getBool(&SETTING_LYRICS_ENABLED)?Nxt_text("Lyrics disabled","Đã tắt lời bài hát"):NxtSoundCloud_active()?NxtSoundCloud_lyricsStatus():NxtSpotify_active()?NxtSpotify_lyricsStatus():Nxt_text("No lyrics · Add .lrc beside your track","Chưa có lời · Thêm .lrc cạnh bài hát");scene_wrapped(s,21,status,b.x+10,b.y+35,b.w-20,b.h-41,muted);return;}
 const char *prev=index>0?Lyrics_lineText(index-1):NULL,*next=Lyrics_lineText(index+1);if(prev)text(s,15,prev,b.x+10,b.y+24,b.w-20,muted);
 int px=nxt.lyric_size;if(px>28)px=28;if(px<19)px=19;int space=b.h-64;if(space<33)space=33;
 scene_wrapped(s,px,current,b.x+10,b.y+44,b.w-20,space,ink);
 if(next&&b.h>=115)text(s,15,next,b.x+10,b.y+b.h-21,b.w-20,muted);
}
static void progress_draw(SDL_Surface *s,Box b,SDL_Color ink,SDL_Color ln){int pos=NxtPlayback_position()/1000,dur=NxtPlayback_duration()/1000;if(pos<0)pos=0;if(dur>0&&pos>dur)pos=dur;rect(s,b.x,b.y,b.w,4,ln,2);if(dur>0){int x=(int)((long long)b.w*pos/dur);rect(s,b.x,b.y,x,4,ink,2);circle(s,b.x+x,b.y+2,5,ink);}char str[64];snprintf(str,sizeof(str),"%02d:%02d",pos/60,pos%60);text(s,17,str,b.x,b.y+6,120,ink);snprintf(str,sizeof(str),dur>0?"%02d:%02d":"--:--",dur/60,dur%60);text(s,17,str,b.x+b.w-90,b.y+6,90,ink);}
static void transport_draw(SDL_Surface *s,Box b,bool playing,int style){if(b.w<=0)return;bool vertical=b.h>b.w;int actions[]={NC_PREV,NC_PLAY,NC_NEXT};const char *icons[]={"skip-back",playing?"pause":"play","skip-forward"};for(int j=0;j<3;j++){int x=b.x+(vertical?0:j*(b.w/3+2)),y=b.y+(vertical?j*b.h/3:0),w=vertical?b.w:b.w/3-5,h=vertical?b.h/3-7:b.h;SDL_Color ink=style==1?rgb(j==1?0xfff2d6:0x302c24):NxtSkin_color(j==1?NXT_ACCENT_INK:NXT_INK),body=style==1?rgb(j==1?0xa44d3d:0xcabb91):NxtSkin_color(j==1?NXT_ACCENT:NXT_LINE);rect(s,x,y+3,w,h,style==1?rgb(0x6c604b):NxtSkin_color(NXT_BG),4);rect(s,x,y,w,h,body,style==10?17:4);NxtSkin_icon(s,icons[j],u(s,x+w/2-22),u(s,y+h/2-10),u(s,20),ink);text(s,14,action_key(actions[j]),x+w/2+5,y+h/2-12,w/2-8,ink);}}
static void volume_draw(SDL_Surface *s,Scene f){if(!f.volume_x)return;int vol=NxtVolume_getPercent();if(vol<0)vol=0;if(vol>100)vol=100;int x=f.volume_x,y=f.volume_y;SDL_Color ink=nxt.interface_style==1?rgb(0x302c24):NxtSkin_color(NXT_INK);text(s,15,"VOL",x,y,48,ink);rect(s,x+45,y+13,128,3,NxtSkin_color(NXT_LINE),2);rect(s,x+45,y+13,128*vol/100,3,ink,2);circle(s,x+45+128*vol/100,y+14,4,ink);char label[30];snprintf(label,sizeof(label),"%d%%",vol);text(s,16,label,x+184,y,58,ink);}
static void readout(SDL_Surface *s,int px,const char *value,int x,int y,int width,SDL_Color ink){NxtSkin_compactText(s,px,value,u(s,x),u(s,y),u(s,width),u(s,16),ink);}
static void audio_draw(SDL_Surface *s,bool shuffle,int repeat){
 const TrackInfo *t=NxtPlayback_info();char fmt[320],bits[40]="";if(t->bit_depth>0)snprintf(bits,sizeof(bits)," · %d-bit",t->bit_depth);
 if(t->sample_rate>0){snprintf(fmt,sizeof(fmt),"%s%s · %.1f kHz",t->codec,bits,t->sample_rate/1000.f);if(t->bitrate>0){size_t l=strlen(fmt);snprintf(fmt+l,sizeof(fmt)-l," · %d kbps",t->bitrate/1000);}}else snprintf(fmt,sizeof(fmt),"%s",Nxt_text("Waiting for source format…","Đang chờ thông số nguồn…"));
 SDL_Color ink=nxt.interface_style==1?rgb(0x302c24):NxtSkin_color(NXT_INK),muted=nxt.interface_style==1?rgb(0x61553e):NxtSkin_color(NXT_MUTED);
 NxtSkin_icon(s,"file-audio",u(s,64),u(s,627),u(s,18),ink);readout(s,17,fmt,90,625,627,ink);
 const char *icons[]={"shuffle",repeat==1?"repeat-1":"repeat","heart"};bool on[]={shuffle,repeat!=0,NxtPlayback_favorite()};for(int j=0;j<3;j++)NxtSkin_icon(s,icons[j],u(s,733+j*34),u(s,627),u(s,18),on[j]?ink:muted);
 int rem=Nxt_timerRemaining();char timer[40];snprintf(timer,sizeof(timer),rem?"%02d:%02d":"--:--",rem/60,rem%60);NxtSkin_icon(s,"timer",u(s,842),u(s,627),u(s,18),muted);readout(s,16,timer,870,625,93,muted);
 bool dac=Player_isUSBDACActive();NxtSkin_icon(s,dac?"usb":nxt.output==1?"bluetooth":NxtHardware_hasJack()?"headphones":"volume-2",u(s,64),u(s,649),u(s,18),ink);
 readout(s,17,NxtHardware_output(),90,648,dac?418:590,ink);if(dac){int rate=Player_getOutputRate(),depth=Player_getOutputBits();char pcm[80];if(rate>0&&depth>0)snprintf(pcm,sizeof(pcm),"PCM %.1fk · %db",rate/1000.f,depth);else snprintf(pcm,sizeof(pcm),"%s",NxtHardware_pcm());readout(s,15,pcm,520,648,228,muted);}
 snprintf(fmt,sizeof(fmt),"EQ · %s",nxt.dsp?NxtEQ_name(nxt.eq_preset):Nxt_text("Off","Tắt"));readout(s,16,fmt,774,648,187,muted);
}
static void tape_live(SDL_Surface *s,int style,float p){int left=style==3?190:125,right=style==3?725:899,cy=style==3?335:334,r=style==3?33:39;reel(s,left,cy,r,p,true);reel(s,right,cy,r,-p,true);char label[60];snprintf(label,sizeof(label),"%04d     STEREO / A",NxtPlayback_position()/1000);text(s,20,label,style==3?302:350,cy-12,style==3?315:397,rgb(0xe5e1ce));}
static void meters_live(SDL_Surface *s,int x,int y,int w,int h){float peak=NxtDSP_peak();if(peak<0)peak=0;if(peak>1)peak=1;for(int j=0;j<2;j++){int mw=(w-12)/2,cx=x+j*(w+12)/2+mw/2,cy=y+h-9;float a=-2.7f+peak*1.7f;line(s,cx,cy,cx+(int)(cosf(a)*(mw*.42f)),cy+(int)(sinf(a)*(h*.65f)),rgb(0x8e4432),2);}}
bool NxtTheme_playing(SDL_Surface *s,bool shuffle,int repeat){
 int style=nxt.interface_style;if(style<=0||style>=24)return false;Scene f=scenes[style];bool playing=NxtPlayback_state()==PLAYER_STATE_PLAYING;float p=phase(playing);
 if(chassis&&(chassis_style!=style||chassis->w!=s->w||chassis->h!=s->h)){SDL_FreeSurface(chassis);chassis=NULL;}
 if(!chassis){chassis=SDL_CreateRGBSurfaceWithFormat(0,s->w,s->h,32,SDL_PIXELFORMAT_ARGB8888);if(chassis){SDL_FillRect(chassis,NULL,0);chassis_draw(chassis,style);SDL_SetSurfaceBlendMode(chassis,SDL_BLENDMODE_BLEND);chassis_style=style;}}
 if(chassis)SDL_BlitSurface(chassis,NULL,s,NULL);else chassis_draw(s,style);
 SDL_Surface *art=NxtPlayback_art();if(!art&&!NxtSpotify_hasMetadata()&&!NxtSoundCloud_hasMetadata())art=album_art_get();
 if(style==22)art=eink_art(s,art,f.art.w);
 if(f.art.w>0){if(style==17)NxtRecord_drawTilted(s,(SDL_Rect){u(s,135),u(s,399),u(s,243),u(s,115)},art,NxtPlayback_generation(),playing&&nxt.animations);else if(style==9||(nxt.cover_mode&&(style==10||style==11||style==21||style==23))){int edge=style==9?287:f.art.w;NxtRecord_draw(s,(SDL_Rect){u(s,f.art.x),u(s,f.art.y),u(s,edge),u(s,edge)},art,NxtPlayback_generation(),playing&&nxt.animations);}else cover(s,art,f.art.x,f.art.y,f.art.w);}
 song_draw(s,f);lyrics_draw(s,f);
 Spectrum_setPosition(u(s,f.spectrum.x),u(s,f.spectrum.y),u(s,f.spectrum.w),u(s,f.spectrum.h));
 progress_draw(s,f.progress,style==1?wc(0,1):style==14||style==20?wc(f.song_palette,1):NxtSkin_color(NXT_INK),style==1?wc(0,3):NxtSkin_color(NXT_LINE));transport_draw(s,f.transport,playing,style);volume_draw(s,f);audio_draw(s,shuffle,repeat);
 if(style==1||style==3)tape_live(s,style,p);
 if(style==4){reel(s,294,253,68,p*.5f,true);reel(s,730,253,68,-p*.5f,true);meters_live(s,68,345,361,90);}
 if(style==5)meters_live(s,70,201,694,79);if(style==16)meters_live(s,75,457,397,59);
 if(style==2){int dur=NxtPlayback_duration(),pos=NxtPlayback_position();int tx=347+(dur>0?(int)((long long)580*pos/dur):0);if(tx>929)tx=929;if(tx<347)tx=347;line(s,tx,233,tx,260,rgb(0xa04e37),3);text(s,15,"TRACK / DIGITAL AUDIO",346,208,570,rgb(0x3d3526));}
 if(style==6){line(s,259,360,259+cosf(p)*99,360+sinf(p)*99,rgb(0xeff7f6),4);circle(s,259,360,16,rgb(0x283333));}
 if(style==14){NxtSkin_icon(s,playing?"pause":"play",u(s,748),u(s,355),u(s,23),rgb(0x345e4e));NxtSkin_icon(s,"skip-back",u(s,669),u(s,355),u(s,23),rgb(0x345e4e));NxtSkin_icon(s,"skip-forward",u(s,827),u(s,355),u(s,23),rgb(0x345e4e));text(s,16,action_key(NC_EQ),722,268,103,rgb(0x345e4e));text(s,16,action_key(NC_LYRICS),746,450,78,rgb(0x345e4e));text(s,15,action_key(NC_PLAY),750,395,71,rgb(0x345e4e));}
 if(style==15){int pos=NxtPlayback_position()/1000;char clock[32];snprintf(clock,sizeof(clock),"%02d:%02d",pos/60,pos%60);text(s,48,clock,80,211,177,wc(5,1));for(int k=0;k<5;k++){int x=390+k*93,y=396-(int)(nxt.eq[k]*.35f);rect(s,x,391,55,17,rgb(0x293c30),1);rect(s,x,y,55,4,rgb(0xc6d291),1);}text(s,16,NxtEQ_name(nxt.eq_preset),790,388,145,rgb(0xeef0df));text(s,17,NxtPlayback_info()->title,79,560,812,wc(5,1));char q[128];snprintf(q,sizeof(q),"%s · %s",action_key(NC_QUEUE),Nxt_text("Open current queue","Mở hàng đợi đang nghe"));text(s,15,q,79,585,812,wc(5,2));}
 if(style==19){for(int j=0;j<9;j++)if(j==nxt.eq_preset)rect(s,631+j%3*105,220+j/3*70,90,3,rgb(0xf0c487),1);}
 char hint[600];snprintf(hint,sizeof(hint),"%s/%s %s · %s %s · %s %s · %s %s · %s EQ · %s %s",action_key(NC_PREV),action_key(NC_NEXT),Nxt_text("Track","Bài"),action_key(NC_PLAY),playing?Nxt_text("Pause","Dừng"):Nxt_text("Play","Phát"),action_key(NC_QUEUE),Nxt_text("Queue","Danh sách"),action_key(NC_LYRICS),Nxt_text("Lyrics","Lời"),action_key(NC_EQ),action_key(NC_QUICK),Nxt_text("Options","Tùy chọn"));NxtSkin_footer(s,hint,false);return true;
}
void NxtTheme_release(void){if(chassis)SDL_FreeSurface(chassis);if(gray_cover)SDL_FreeSurface(gray_cover);chassis=gray_cover=gray_source=NULL;chassis_style=-1;header_palette=false;at=0;advancing=false;}

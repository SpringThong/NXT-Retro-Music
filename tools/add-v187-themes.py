#!/usr/bin/env python3
"""Append fourteen native, static chassis; existing theme IDs/assets are stable.
Runs only at build time. Runtime decodes/scales one selected chassis once.
"""
from pathlib import Path
import json, math
from PIL import Image, ImageDraw
R=Path(__file__).resolve().parents[1]
# bg, panel, text, secondary text, border, accent, selected text.
NEW=[
("Forest Dial",["152F29","23473C","E4EEE4","B8D0BE","416250","B4D194","18271D"],"dial"),
("Coral Tape",["482D32","65414A","F7E4DE","DEC0BA","886069","F1AC96","392329"],"tape"),
("Midnight Indigo",["1C2341","2D3656","E7E9F5","BCC3DF","4C5777","AAADEC","20243B"],"arc"),
("Copper Sunset",["422E22","624638","F4E6D6","D9C2AB","856453","E7B782","31251C"],"horizon"),
("Sakura Glass",["DED0D6","CAB6C0","46333E","503B47","AF969F","6F354F","F4E6ED"],"petal"),
("Cobalt Grid",["142E48","254966","E3ECF4","B1CDDD","41677F","79C4E2","142E48"],"grid"),
("Desert Radio",["DDD1BA","C6B899","44382C","504330","A59A7D","62402C","F3E9D8"],"radio"),
("Emerald Studio",["163D35","24574B","DDEFDF","B2D4BF","3F7763","8DDDAB","18372A"],"studio"),
("Lavender Paper",["D6D0E3","C0B6D1","3E344F","4D425E","A398B2","524067","F1EAF9"],"paper"),
("Ice Blue",["C9DEE0","AFCCCE","29454A","365157","87ADB0","24565C","EDF5F4"],"ice"),
("Amber Console",["353017","524A28","EFE9CD","D1C69A","73683C","DAC176","302A16"],"console"),
("Ruby Velvet",["3D1F32","5D334A","F1E2EB","D9B7C9","82536D","E3A0BD","351D2A"],"velvet"),
("Graphite Split",["24292D","3B434A","E8EBE9","C0CACD","5A646C","A7CBD1","233135"],"split"),
("Olive Notebook",["D5D6BE","BFC1A3","3B4130","454C38","9B9F80","46512C","EFF1E2"],"notebook"),
]
# Live areas in 1024x768 design space. No text or album baked into assets.
A=(86,188,350,350); B=(602,190,344,344); C=(76,174,390,390)
layouts=[
(A,(496,213,444,92),(500,403,435,72)),
(A,(496,206,444,92),(500,448,435,48)),
(B,(76,204,450,100),(80,408,445,60)),
(C,(527,200,411,98),(530,446,405,52)),
(A,(510,228,424,100),(510,432,425,48)),
(B,(82,220,448,100),(85,451,445,46)),
(A,(500,286,428,92),(500,426,430,48)),
(B,(78,200,452,100),(82,399,448,79)),
(C,(528,226,408,100),(528,441,409,49)),
(A,(505,208,437,100),(505,405,432,70)),
(B,(85,212,436,100),(85,416,431,56)),
(A,(494,229,448,100),(498,433,442,55)),
(B,(82,226,448,100),(84,428,441,53)),
(C,(532,228,405,100),(532,441,405,47)),
]
def create_chassis(i,values,kind,layout):
 bg,panel,ink,muted,border,accent,_=["#"+v for v in values]
 im=Image.new("RGB",(1024,768),bg);d=ImageDraw.Draw(im)
 d.rounded_rectangle((32,146,992,615),radius=22,fill=panel,outline=border,width=2)
 art,song,wave=layout
 ax,ay,aw,ah=art; sx,sy,sw,sh=song
 d.rounded_rectangle((ax-10,ay-10,ax+aw+10,ay+ah+10),radius=14,fill=bg,outline=border,width=2)
 # Quiet decorative areas never overlap album/title/spectrum.
 if kind=="dial":
  for j in range(21):
   a=math.pi*(1+j/20);cx,cy=724,392
   d.line((cx+math.cos(a)*124,cy+math.sin(a)*70,cx+math.cos(a)*132,cy+math.sin(a)*77),fill=accent,width=2)
 elif kind=="tape":
  for cx in (608,823):
   d.ellipse((cx-35,354,cx+35,424),outline=accent,width=3)
   d.ellipse((cx-10,379,cx+10,399),outline=border,width=3)
  d.line((610,389,820,389),fill=border,width=2)
 elif kind=="arc":
  for k in range(4):d.arc((84+k*15,327+k*8,484-k*15,402-k*8),180,360,fill=accent,width=2)
 elif kind=="horizon":
  for k in range(5):d.line((530,357+k*13,932,357+k*13),fill=border if k%2 else accent,width=1)
 elif kind=="petal":
  for k in range(5):
   cx=560+k*69;d.arc((cx,363,cx+48,401),0,180,fill=accent,width=2)
 elif kind=="grid":
  for x in range(86,513,30):d.line((x,355,x,394),fill=border,width=1)
  for y in range(355,395,13):d.line((86,y,516,y),fill=border,width=1)
 elif kind=="radio":
  for y in range(190,244,7):d.line((506,y,920,y),fill=border,width=2)
  for x in range(506,922,24):d.line((x,263,x,272 if x%48 else 278),fill=accent,width=2)
 elif kind=="studio":
  for j in range(14):
   x=89+j*31;h=12+(j*13)%39;d.rounded_rectangle((x,382-h,x+14,382),radius=3,fill=accent if j%3==0 else border)
 elif kind=="paper":
  for y in (352,365,378):d.line((533,y,926,y),fill=border,width=1)
  d.line((518,175,518,566),fill=accent,width=2)
 elif kind=="ice":
  d.polygon(((512,350),(603,381),(747,337),(930,386),(930,390),(510,390)),outline=border)
  d.line((512,380,930,380),fill=accent,width=1)
 elif kind=="console":
  for k in range(3):d.rounded_rectangle((89+k*145,351,209+k*145,378),radius=5,outline=accent if k==0 else border,width=2)
 elif kind=="velvet":
  for x in range(497,937,12):d.line((x,360,x,391),fill=border,width=1)
 elif kind=="split":
  d.line((550,177,550,570),fill=accent,width=3)
  d.rounded_rectangle((87,355,520,381),radius=10,outline=border,width=2)
 elif kind=="notebook":
  for y in range(183,573,42):d.line((58,y,69,y),fill=accent,width=3)
  d.line((514,180,514,574),fill=accent,width=2)
 d.line((sx,sy+sh+13,sx+sw,sy+sh+13),fill=border,width=1)
 target=R/f"upstream/res/themes/v18/theme-{i:02d}.png"
 temporary=target.with_suffix(".png.building")
 im.save(temporary,format="PNG",optimize=True)
 temporary.replace(target)
def main():
 p=R/"upstream/src/nxt_v18_palette.inc";t=p.read_text()
 # Idempotent append: production retains all original 36 rows.
 t=t.split("static const unsigned")[0]
 old=p.read_text().split("={\n",1)[1].split("\n};",1)[0].splitlines()[:36]
 names=json.loads("["+p.read_text().split("names[]={",1)[1].split("};",1)[0]+"]")[:36]
 p.write_text("static const char *names[]={"+",".join(json.dumps(x) for x in names+[n for n,_,_ in NEW])+"};\nstatic const unsigned colors[NXT_THEME_COUNT][7]={\n"+",\n".join(x.rstrip(",") for x in old)+",\n"+",\n".join("{"+",".join("0x"+v.lower() for v in vals)+"}" for _,vals,_ in NEW)+"\n};\n")
 p=R/"upstream/src/nxt_v18_scene_data.inc";old=p.read_text().split("={\n",1)[1].split("\n};",1)[0].splitlines()[:36]
 rows=[]
 for idx,((name,vals,kind),layout) in enumerate(zip(NEW,layouts),36):
  art,song,wave=layout
  rows.append("{"+",".join("{"+",".join(map(str,b))+"}" for b in (song,(0,0,0,0),wave,(72,608,880,32),art))+"}")
  create_chassis(idx,vals,kind,layout)
 p.write_text("static const Scene scenes[NXT_THEME_COUNT]={\n"+",\n".join(x.rstrip(",") for x in old)+",\n"+",\n".join(rows)+"\n};\n")
 (R/"upstream/res/themes/v18/new-themes-v187.json").write_text(json.dumps({"version":"1.8.7","count":50,"original_included":True,"new":[{"id":i,"name":n,"decor":k,"palette":v} for i,(n,v,k) in enumerate(NEW,36)],"runtime":"Static cached chassis; existing font, frame budgets and audio engine unchanged"},indent=2)+"\n")
 print("PASS appended themes 36–49; original 0–35 assets unchanged")
if __name__=="__main__":main()


# PhiVideo
#### 官谱文件命令行渲染器，支持老版本谱面

 - 不想给代码了，写得烂得要死
 - 直接给命令行帮助得了

```
PGR [OPTIONS] [File]


POSITIONALS:
  File TEXT                   The Path of the Chart file

OPTIONS:
  -h,     --help              Print this help message and exit
  -d,     --debug             Debug Mode
  -v,     --video BOOLEAN     Render Video
  -c,     --cover BOOLEAN     Render Cover
  -o,     --output TEXT       Output Name
  -p,     --picTime FLOAT     Render a Picture at the Time
  -z,     --zoom FLOAT:POSITIVE
                              Zoom
  -m,     --mv FLOAT          Music Volume
  -n,     --nv FLOAT          Notes Volume
  -W,     --width INT         Width
  -H,     --height INT        Height
  -a,     --aas FLOAT         Anti-Aliasing Scale
          --FPS INT:POSITIVE  FPS
          --CPU INT:INT in [1 - 24]
                              CPU Core Num
```

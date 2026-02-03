# PhiVideo

#### 一个基于 `C++` 的 `Phigros` 谱面渲染器

## 使用方法

### Release
- 下载 [`Release`](https://github.com/phigrostl/PhiVideo/releases) 页面中的 `PhiVideo.zip` 文件并解压

-  安装 [`FFmpeg`](https://ffmpeg.org/download.html) 并将其添加到系统环境变量中

#### 命令行参数
``` args
-h, --help                              输出帮助信息并退出
-d, --debug                             开启谱面调试模式
-v, --video         BOOLEAN             渲染视频
-c, --cover         BOOLEAN             渲染封面
-o, --output        TEXT                输出文件名
-p, --picTime       FLOAT               渲染图片的时间点
-s, --startTime     FLOAT               渲染开始时间
-e, --endTime       FLOAT               渲染结束时间
-z, --zoom          FLOAT:POSITIVE      缩放比例
-m, --musicVolume   FLOAT               音乐音量
-n, --notesVolume   FLOAT               音符音效音量
-W, --width         INT                 输出视频宽度
-H, --height        INT                 输出视频高度
-a, --aas           FLOAT               反锯齿比例
    --FPS           INT:POSITIVE        输出视频帧率
    --CPU           INT:INT in [1 32]   使用的CPU核心数
\   固定参数        STRING:FILE_PATH    谱面文件路径

```
#### UI

##### UI.json
```json
{
    "hitFx": ["[x]", "[y]"],                // 打击特效长宽数量
    "holdAtlas": ["[head]", "[tail]"],      // Hold 音符头尾切割位置
    "holdAtlasMH": ["[head]", "[tail]"],    // Hold 双押音符头尾切割位置
    "title": "Title",                       // 标题文本
    "title2": "DEBUGTitle",                 // 调试标题文本
    "combo": "Combo",                       // 连击文本
    "info": "Info"                          // 水印文本
}
```

##### 图像与音频
位于 `resources` 目录下，有需要自行替换

### 自行编译
1. 安装 [`Visual Studio 2019`](https://visualstudio.microsoft.com/zh-hans/downloads/) 或更高版本
2. 安装 [CMake 3.18](https://cmake.org/download/) 或更高版本
3. 克隆或下载项目到本地
4. 使用CMake生成Visual Studio解决方案
5. 在Visual Studio中打开生成的解决方案并编译

## 技术栈
- 语言：C++
- 构建工具：CMake
- 平台：Windows
- 依赖：
  - [FFmpeg](https://ffmpeg.org/download.html)
  - [CLI11](https://github.com/CLIUtils/CLI11)
  - [cJSON](https://github.com/DaveGamble/cJSON)

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=phigrostl/PhiVideo&type=date)](https://www.star-history.com/#phigrostl/PhiVideo&type=date)

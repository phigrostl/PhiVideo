# PhiVideo

> 一个基于 `C++` 的 `Phigros` 谱面渲染器 <br> 本工具仅用于学习交流，请勿用于商业用途

## 使用方法

### Release
- 下载 [`Release`](https://github.com/phigrostl/PhiVideo/releases) 页面中的 `PhiVideo.zip` 文件并解压

-  安装 [`FFmpeg`](https://ffmpeg.org/download.html) 并将其添加到系统环境变量中

#### 命令行参数
``` args

PhiVideo [FILE_PATH] [OPTIONS]

                  TEXT:FILE_PATH                    谱面文件路径

-h, --help                                          输出帮助信息并退出
-d, --debug                                         开启谱面调试模式
-y, --overwrite                                     是否总是覆盖输出文件
-v, --video              BOOLEAN        true        渲染视频
-c, --cover              BOOLEAN        true        渲染封面
-o, --output                TEXT     Unnamed        输出文件名
-p, --picTime              FLOAT         0.0        渲染图片时间
-s, --startTime            FLOAT         0.0        渲染开始时间
-e, --endTime              FLOAT        -1.0        渲染结束时间
-z, --zoom        FLOAT:POSITIVE         1.0        缩放比例
-m, --musicVolume          FLOAT         1.0        音乐音量
-n, --notesVolume          FLOAT         0.5        音符音效音量
-W, --width                  INT        1920        输出视频宽度
-H, --height                 INT        1080        输出视频高度
-a, --aas                    INT           1        抗锯齿比例
-b, --bitrate              FLOAT        10.0        视频码率(Mbps)
-l, --logLevel              TEXT        info        日志等级
    --FPS           INT:POSITIVE          60        视频帧率
    --CPU      INT:INT in [1 32]           4        使用的CPU核心数
```
#### UI

##### UI.json
```json
{
    "HitFx": ["[x]", "[y]"],                // 打击特效长宽数量
    "HoldAtlas": ["[head]", "[tail]"],      // Hold 音符头尾切割位置
    "HoldAtlasMH": ["[head]", "[tail]"],    // Hold 双押音符头尾切割位置
    "Title": "Title",                       // 标题文本
    "Title2": "DEBUGTitle",                 // 调试标题文本
    "Info": "Info"                          // 水印文本
    "Combo": "Combo",                       // 连击文本
}
{
	"HitFx": [[x], [y]],                    // 打击特效长宽数量
	"HoldAtlas": [[head], [tail]],          // Hold 音符头尾切割位置
	"HoldAtlasMH": [[head], [tail]],        // Hold 双押音符头尾切割位置

	"Title": "Title",                       // 标题文本
	"Title2": "DEBUGTitle",                 // 调试标题文本
	"Info": "Info",                         // 水印文本
	"Combo": "Combo"                        // 连击文本

	"NoteDelay": [                          // 音符提前时间
		[Tap/Hold],
		[Drag],
		[Flick]
	]
}
```

##### 图像与音频
位于 `resources` 目录下，有需要自行替换

### 注意事项
 - 该程序会占用大量内存与 CPU 资源，请确保计算机性能足够
 - 可能会替换删除掉某些文件，请提前备份重要数据
     

### 自行编译
1. 安装 [`Visual Studio 2019`](https://visualstudio.microsoft.com/zh-hans/downloads/) 或更高版本
2. 安装 [`CMake 3.18`](https://cmake.org/download/) 或更高版本
3. 克隆或下载项目到本地
4. 使用 `CMake `生成 `Visual Studio` 解决方案
5. 在 `Visual Studio` 中打开生成的解决方案并编译

## 技术栈
- 语言：`C++`
- 构建工具：`CMake`
- 平台：`Windows`
- 依赖：
  - [`FFmpeg`](https://ffmpeg.org/download.html)
  - [`CLI11`](https://github.com/CLIUtils/CLI11)
  - [`cJSON`](https://github.com/DaveGamble/cJSON)

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=phigrostl/PhiVideo&type=date)](https://www.star-history.com/#phigrostl/PhiVideo&type=date)

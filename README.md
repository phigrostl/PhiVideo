# PhiVideo

> 一个基于 `C++` 的 `Phigros` 谱面渲染器 <br> 本工具仅用于学习交流，请勿用于商业用途 <br> 项目资源文件均属于 `南京鸽游网络有限公司` ，本作者不拥有任何版权

> 注意：该项目的代码质量较低，很多错误处理不完善，若在使用时遇到问题请先检查输入是否正确，如果问题仍然存在请提交 [issue](https://github.com/phigrostl/PhiVideo/issues) <br> 部分[特殊字符](#特殊字符)会引发程序问题，请避免在输入文件路径或输出文件名中使用特殊字符

## 使用方法

### Release
- 下载 [`Release`](https://github.com/phigrostl/PhiVideo/releases) 页面中的 `PhiVideo.zip` 文件并解压
- 将该文件夹添加至系统环境变量 `PATH` 中

#### 命令行参数
``` args

PhiVideo [FILE_PATH] [OPTIONS]

                  TEXT:FILE_PATH                    谱面文件路径

-h, --help                                          输出帮助信息并退出
-d, --debug                                         开启谱面调试模式
-y, --overwrite                                     是否总是覆盖输出文件
-c, --cover              BOOLEAN        true        渲染封面
-p, --picTime              FLOAT         0.0        渲染图片时间
-s, --startTime            FLOAT         0.0        渲染开始时间
-e, --endTime              FLOAT        -1.0        渲染结束时间
-o, --output                TEXT     Unnamed        输出文件名
-A, --alphaMode                                     输出视频是否为ALPHA通道
-v, --video              BOOLEAN        true        渲染视频
-z, --zoom        FLOAT:POSITIVE         1.0        缩放比例
-m, --musicVolume          FLOAT         1.0        音乐音量
-n, --notesVolume          FLOAT         0.5        Note 音效音量
-W, --width                  INT        1920        输出视频宽度
-H, --height                 INT        1080        输出视频高度
-a, --aas                    INT           1        抗锯齿比例
-b, --bitrate              FLOAT        10.0        视频码率(Mbps)
-l, --logLevel              TEXT        info        日志等级
-L, --language               INT           0        日志语言
-F, --FPS           INT:POSITIVE          60        视频帧率
-G, --GPU           INT:POSITIVE        true        GPU线程数
-C, --CPU      INT:INT in [1 32]           4        使用的CPU核心数
    --seed                 FLOAT      [rand]        打击特效粒子随机种子
```
#### UI

##### UI.json
```json
{
    "HitFx": [5, 6],                                                // 打击特效长宽数量
    "HoldAtlas": [50, 50],                                          // Hold Note 头尾切割位置
    "HoldAtlasMH": [100, 100],                                      // Hold 双押Note 头尾切割位置

    "ParticleNum": 4,					                        	// 打击特效粒子数量

    "RenderJudgeLines": true,			                        	// 是否渲染判定线
    "RenderNotes": true,				                        	// 是否渲染 Note
    "RenderEffects": true,				                            // 是否渲染打击特效
    "RenderUI": true,					                        	// 是否渲染 UI
    "RenderMainInfo": true,			                            	// 是否渲染主信息
    "RenderSubInfo": true,			                            	// 是否渲染次信息
    "RenderDebugInfo": [true,			                        	// 是否渲染调试信息
    "RenderBack": true,												// 是否渲染背景

    "Title": "谱面演示",												// 标题文本
    "Title2": "谱面揭秘",                                            // 调试标题文本
    "Info": "Code & File by たおりん on Bilibili | - Phigros - ",    // 水印文本
    "Combo": "AUTOPLAY"                                             // 连击文本

    "SYM": [						                            	// 进度条符号列表
        "\u001B[38;2;180;0;0m%",
        "\u001B[38;2;173;10;0ma",
        "\u001B[38;2;166;20;0mb",
        "\u001B[38;2;159;30;0mc",
        "\u001B[38;2;152;40;0md",
        "\u001B[38;2;145;50;0me",
        "\u001B[38;2;138;60;0mf",
        "\u001B[38;2;131;70;0mg",
        "\u001B[38;2;124;80;0mh",
        "\u001B[38;2;117;90;0mi",
        "\u001B[38;2;110;100;0mj",
        "\u001B[38;2;103;110;0mk",
        "\u001B[38;2;96;120;0ml",
        "\u001B[38;2;89;130;0mm",
        "\u001B[38;2;82;140;0mn",
        "\u001B[38;2;75;150;0mo",
        "\u001B[38;2;68;160;0mp",
        "\u001B[38;2;61;170;0mq",
        "\u001B[38;2;54;180;0mr",
        "\u001B[38;2;47;190;0ms",
        "\u001B[38;2;40;200;0mt",
        "\u001B[38;2;33;210;0mu",
        "\u001B[38;2;26;220;0mv",
        "\u001B[38;2;19;230;0mw",
        "\u001B[38;2;12;240;0mx",
        "\u001B[38;2;5;250;0my",
        "\u001B[38;2;0;255;0mz",
        "\u001B[38;2;0;255;0m#"
    ],

    "NoteDelay": [                                                  // Note 音频提前时间
        0.00061239101845989,
        0.03229180050666951,
        0.00183683717307712
    ]
}
```

##### 图像与音频
位于 `resources` 目录下，有需要自行替换

### 注意事项
 - 该程序会占用大量内存与 CPU 资源，请确保计算机性能足够
 - 可能会替换删除掉某些文件，请提前备份重要数据
 - 若电脑不支持显卡，请尝试将 `-g` 选项降低
     

### 自行编译
1. 安装 [`Visual Studio 2026`](https://visualstudio.microsoft.com/zh-hans/downloads/) 或更高版本
2. 安装 [`CMake 3.18`](https://cmake.org/download/) 或更高版本
3. 克隆或下载项目到本地
4. 使用 `CMake` 生成 `Visual Studio` 解决方案
5. 在 `Visual Studio` 中打开生成的解决方案并编译
6. 按 [`Release`](#release) 部分的使用方法运行编译后的程序

## 技术栈
- 语言：`C++`
- 构建工具：`CMake`
- 平台：`Windows`
- 依赖：
  - [`FFmpeg`](https://ffmpeg.org/download.html)
  - [`CLI11`](https://github.com/CLIUtils/CLI11)
  - [`cJSON`](https://github.com/DaveGamble/cJSON)
  - [`stbtt`](https://github.com/nothings/stb)

## 特殊字符
 - `中文字符`，`空格`，`终端特殊字元(不含下文特殊字符)`，可正常使用
 - 某些特殊语言中的字符
 - `\` `/` `:` `*` `?` `"` `<` `>` `|`

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=phigrostl/PhiVideo&type=date)](https://www.star-history.com/#phigrostl/PhiVideo&type=date)

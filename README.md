# Introduction of DeskMaze

**一款可自定义地图的命令行伪3D迷宫游戏**

**A command line pseudo 3D maze game with customizable maps**

### 重要： 

1. 目前该程序能在VScode的Terminal环境下运行流畅，但是直接打开exe文件会有长宽适配和帧刷新问题。默认的适配显示长宽是Windows环境，1080p，100%缩放的VScode中Terminal。如果在其他环境测试请务必调试'config.txt'中的字符画长宽（单位：字符数量） 

2. 原版与SP版本的唯一区别是SP版本在尝试全屏化后会有一个手动提示需要全屏化的页面，防止全屏化失败后显示出撕裂的主菜单。若该程序无法正确自动全屏化窗口，可以使用SP版本。

#### Important: 

1. Currently, the program can run smoothly in the VScode Terminal environment, but there may be issues with length and width adaptation and frame refresh when directly opening .exe file. The default adaptation display length and width is Windows environment, 1080p, and Terminal in VScode with 100% scaling. If testing in other environments, please be sure to debug the displaying length and width (unit: number of characters) in 'config. txt'. 
2. The only difference between the original version and the SP version is that the SP version will have a manual prompt for full-screening after attempting full screen, to prevent the main menu from tearing after full screen failure. If the program cannot automatically full screen the window correctly, the SP version can be used.

## CN

#### 项目

**DESKMAZE**

#### 开发者

**LabLeonOfficial**

#### 核心工作

一款可自定义地图的命令行伪3D迷宫游戏

#### 使用说明

若开始时程序没有全屏显示，请手动按F11全屏化窗口

其它使用说明在程序内亦有介绍：

在菜单界面，用W/S进行选择，Enter进行确认

在游戏中，使用W/A/S/D进行移动，按住Shift奔跑，左右移动鼠标调整视角，L离开游戏

关于导入地图：将自定义关卡的地图文件放入/map文件夹中，并在/map/maplist文件中更新地图总数与新地图的文件名和关卡名

#### 注意事项

如果直接打开程序发生屏幕刷新问题，请在VScode的terminal环境下尝试打开！

ACSLL画的默认适应分辨率为1080p屏幕+VScode的独立Terminal

若屏幕撕裂即为字符画长宽未对准，请调整config.txt中的长宽信息（单位：横向字符数量，纵向字符数量）

## EN

#### Project Name

**DESKMAZE**

#### Developer

**LabLeonOfficial**

#### Core Work

A command line pseudo 3D maze game with customizable maps

#### Instructions

If the program does not display in full screen at the beginning, please manually press F11 to make the window full screen 
Other usage instructions are also introduced in the program: 
On the menu interface, use ‘W/S’ to select and ‘Enter’ to confirm 
In the game, use ‘W/A/S/D’ to move, hold down ‘Shift’ to run, move the mouse left and right to adjust the perspective, and leave the game with ’L’
Regarding importing maps: Place the custom level map files in the ‘/map’ folder and update the total number of maps, the file name of the new map, and the level name in the ‘/map/maplist’ file

#### Precautions

If there is a screen refresh issue when opening the program directly, please try opening it in the VScode terminal environment! 
The default adaptive resolution for ACSLL drawing is 1080p screen+VScode independent terminal 
If the screen tears, it means that the character drawing is not aligned in length and width. Please adjust the length and width information in the ‘config.txt’ file (unit: horizontal character count, vertical character count)

Note: The English introduction is generated using translation software and personally proofread, verified, and modified. There may be inaccuracies.

#include <bits/stdc++.h>
#include <algorithm>
#include <cmath>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>
#include <conio.h>
// #include<Mmsystem.h>
// #pragma comment(lib,"winmm.lib")

#define KEY_DOWN(vKey) ((GetAsyncKeyState(vKey) & 0x8000) ? 1 : 0)
#define KEY_UP(vKey) ((GetAsyncKeyState(vKey) & 0x8000) ? 0 : 1)

#define M_PI 3.14159265358979323846

using namespace std;

// === 全局变量区 ===

int width = 237;
int height = 67;
const int max_width = 300;
const int max_height = 80;
char frame[max_width][max_height];
char frameRaw[max_width * max_height + 1];

double angle, posX, posY; // 玩家视角、位置
double angleInit = 0, posXInit = 0, posYInit = 0;

const double rotSpeed = 100; // 旋转速度（角度每秒）
double moveSpeed = 0.5;      // 移动速度（格每秒）
const double moveSpeedWalk = 0.5, moveSpeedRun = 1;
const double moveScale = 0.5; // 左右后移动要比前慢一点
const double deltaTime = 30;  // 帧间隔（毫秒）

const double wallHeight = 0.2; // 由于上下对称显示，墙高常数制定的是墙高的一半
const double angularField = 100;
const double verticalZooming = 0.5; // 单个字符不是正方形而是长方形，垂直比率需要进行缩放

int mapWidth, mapHeight;                           // 游戏地图在第一象限，迷宫左下角即为(0,0)，此为迷宫长方形的长宽
vector<vector<bool>> wallVertical, wallHorizontal; // 水平墙数组与垂直墙数组;Vertical数组应为[mapWidth+1,mapHeight]，Horizontal数组应为[mapWidth,mapHeight+1]
vector<pair<string, string>> mapList;              // 地图列表

const double wallThick = 0.08; // 墙厚度的一半(单位：格)
const double wallLength = 1;

int screenWidth = 1920;
int screenHeight = 1080;

const double rotateScale = 0.05; // 旋转系数越大，鼠标控制旋转越灵敏

enum Page
{
    MAIN_SCREEN,
    MAP_SELECT,
    GAME,
    CONTROLS
};
Page pageSelect; // 下一个切换的页面

string mainScreenOpts[3] = {"Select Map", "Controls", "Exit"};

// === 通用函数 ===
int clamp(int x, int bot, int top){
    if(x < bot){
        return bot;
    }
    if(x > top){
        return top;
    }
    return x;
}

void swap(double &a, double &b)
{
    double temp = a;
    a = b;
    b = temp;
}

double RADtoDEG(double rad)
{
    return rad / M_PI * 180;
}

double DEGtoRAD(double deg)
{
    return deg * M_PI / 180;
}

void HideCursor() // 隐藏光标
{
    CONSOLE_CURSOR_INFO cur = {1, 0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cur);
}

// === 光标调整 ===

void GotoXY(int x, int y)
{ // 移动光标

    COORD position;
    position.X = x;
    position.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
}

void GotoHead()
{ // 光标移回开头
    GotoXY(0, 0);
}

// === 屏幕打印函数 ===

void color() // 自定义函根据参数改变颜色
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void color(short x) // 自定义函根据参数改变颜色
{
    if (x >= 0 && x <= 15)                                           // 参数在0-15的范围颜色
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), x); // 只有一个参数，改变字体颜色
    else                                                             // 默认的颜色白色
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

class ASCIIart
{
public:
    int widthASCII, heightASCII;
    vector<string> content;

    void read(string path)
    {
        // 创建流对象
        ifstream ifs;
        ifs.open(path, ios::in);

        // 判断文件是否成功打开成功则读取数据
        if (!ifs.is_open())
        {
            ifs.close();
            return;
        }

        ifs >> this->widthASCII >> this->heightASCII;
        ifs.get();

        string buf;
        for (int i = 0; i < this->heightASCII; i++)
        {
            getline(ifs, buf);
            this->content.push_back(buf);
        }
        // 关闭文件
        ifs.close();
    }

    void PrintASCII(int posX, int posY) // 以屏幕的XY（中心为0,0）为中心点打印图像
    {
        for (int i = 0; i < this->heightASCII; i++)
        {
            GotoXY(width / 2 - this->widthASCII / 2 + posX, height / 2 - this->heightASCII / 2 + i + posY);
            printf(this->content[i].data());
        }
        GotoXY(0, 0);
    }

    void PrintASCII(int posX, int posY, int clr) // 带颜色的PrintASCII
    {
        color(clr);
        PrintASCII(posX, posY);
        color();
    }
};

void PrintStr(string str, int posX, int posY) // 以屏幕的XY（中心为0,0）为中心点打印string
{
    int len = str.length();
    GotoXY(width / 2 - len / 2 + posX, height / 2 + posY);
    printf(str.data());
    GotoXY(0, 0);
}

void PrintStr(string str, int posX, int posY, int clr) // 带颜色的PrintStr
{
    color(clr);
    PrintStr(str, posX, posY);
    color();
}

// === 字符画 ===

ASCIIart title, version, map_select, congratulation_text, controls, controls_text;

// === 刷新帧函数 ===

double AngToLength(double ang)
{ // 将角视场转化为显示长度（单位：像素）
    return width * (ang / angularField);
}

double LengthToAng(double pix)
{ // 将显示长度（单位：像素）转化为角视场
    return angularField * (pix / width);
}

double AngToLengthVertical(double ang)
{ // 将角视场转化为纵向显示长度（单位：像素）
    return width * (ang / angularField) * verticalZooming;
}

double LengthToAngVertica(double pix)
{ // 将纵向显示长度（单位：像素）转化为角视场
    return angularField / verticalZooming * (pix / width);
}

int RoundSP(double index)
{ // 把浮点的屏幕像素位置数据转化为四舍五入的像素值
    return int(round(index - 0.5));
}

int CeilSP(double index)
{ // 把浮点的屏幕像素位置数据转化为向右边取整的像素值
    return int(ceil(index - 0.5));
}

int FloorSP(double index)
{ // 把浮点的屏幕像素位置数据转化为向左边取整的像素值
    return int(floor(index - 0.5));
}

int ClampWidth(int index)
{ // clamp屏幕宽度
    return clamp(index, 0, width - 1);
}

int ClampHeight(int index)
{ // clamp屏幕高度
    return clamp(index, 0, height - 1);
}

void FillRow(int index, int bottom, int top, char ch)
{
    // 向frame中填充一列
    // 四个参数：在frame中的列数下标，填充起始下标，填充结束下标，填充符号
    for (int i = bottom; i <= top; i++)
    {
        frame[index][i] = ch;
    }
}

void FillRectangleRaw(double lIndex, double rIndex, double lHeight, double rHeight, char ch)
{
    // 向frame中填充一个三维视图中的长方形
    // 五个参数：左边在frame中的列数下标，右边在frame中的列数下标，左边墙的显示高度（的一半），右边墙的显示高度（的一半），填充符号
    // 列数下标不是整数，每个像素点的中心（.5,.5处)若在墙图形的内部则被填充
    int lPix = ClampWidth(CeilSP(lIndex)), rPix = ClampWidth(FloorSP(rIndex));
    for (int i = lPix; i <= rPix; i++)
    {
        double iHeight = lHeight + (rHeight - lHeight) * ((i - lIndex) / (rIndex - lIndex));
        FillRow(i, ClampHeight(CeilSP(height * 0.5 - iHeight)), ClampHeight(FloorSP(height * 0.5 + iHeight)), ch);
    }
}

void FillRectangle(double lAng, double rAng, double lDis, double rDis, char ch)
{ // 向frame中填充一个三维视图中的长方形
  // 五个参数：左边相对于屏幕中央的角度差，左边相对于屏幕中央的角度差（左移为负右移为正），左边到原点距离，右边到原点距离，填充符号

    if (rAng < lAng)
    {
        swap(lAng, rAng);
        swap(lDis, rDis);
    }
    if (rAng - lAng > 180)
    {
        return; // 墙在身后;
    }
    if (rAng < -angularField / 2 || lAng > angularField / 2)
    {
        return; // 墙不在视野内
    }

    FillRectangleRaw(width * 0.5 + AngToLength(lAng), width * 0.5 + AngToLength(rAng), AngToLengthVertical(RADtoDEG(atan(wallHeight / lDis))), AngToLengthVertical(RADtoDEG(atan(wallHeight / rDis))), ch);
}

double DisToHeight(double dis)
{ // 输入墙的距离计算视野中像素高度
    return AngToLengthVertical(RADtoDEG(atan(wallHeight / dis)));
}

double AngToIndex(double ang)
{ // 输入视野相对角返回画面列数下标
    return width * 0.5 + AngToLength(ang);
}

double IndexToAng(double index)
{ // 输入画面列数下标返回视野相对角
    return LengthToAng(index - width * 0.5);
}

void FillRectangleRenewedLen(double lAng, double rAng, double lDis, double rDis, char ch, double len)
{
    // 考虑到视野变形，长方形不会在视野中呈现长方形，需要做优化
    // 向frame中填充一个三维视图中的长方形
    // 六个参数：左边相对于屏幕中央的角度差，左边相对于屏幕中央的角度差（左移为负右移为正），左边到原点距离，右边到原点距离，填充符号，墙长度（其实直接算也可以）

    if (rAng < lAng)
    {
        swap(lAng, rAng);
        swap(lDis, rDis);
    }
    if (rAng - lAng > 180)
    {
        // 墙在身后;
        if (lAng < -angularField / 2 && rAng > angularField / 2)
        {
            return; // 墙不在视野内
        }
        if (lAng > -angularField / 2)
        {
            rAng -= 360; // 确保墙被向正确的方向渲染
            swap(lAng, rAng);
            swap(lDis, rDis);
        }
        else if (rAng < angularField / 2)
        {
            lAng += 360; // 确保墙被向正确的方向渲染
            swap(lAng, rAng);
            swap(lDis, rDis);
        }
    }
    if (rAng < -angularField / 2 || lAng > angularField / 2)
    {
        return; // 墙不在视野内
    }

    double lIndex = AngToIndex(lAng), rIndex = AngToIndex(rAng);
    int lPix = ClampWidth(CeilSP(lIndex)), rPix = ClampWidth(FloorSP(rIndex));

    double sinlSideAng = sin(DEGtoRAD(rAng - lAng)) * (rDis / len);                                         // 提前求出，优化算法用
    double lSideAng = rDis * rDis > lDis * lDis + len * len ? M_PI - asin(sinlSideAng) : asin(sinlSideAng); // 墙两头与玩家所成三角形靠左的角，弧度制，之后解三角形要用

    for (int i = lPix; i <= rPix; i++)
    {
        double rSideAng = DEGtoRAD(180 - RADtoDEG(lSideAng) - (IndexToAng(i) - lAng)); // 正在渲染该列的注视点、墙左边与玩家所成三角形的右边角，弧度制
        double iHeight = DisToHeight(lDis * (sinlSideAng / sin(rSideAng)));
        // double iHeight = lHeight + (rHeight - lHeight) * ((i - lIndex) / (rIndex - lIndex));
        FillRow(i, ClampHeight(CeilSP(height * 0.5 - iHeight)), ClampHeight(FloorSP(height * 0.5 + iHeight)), ch);
    }
}

void FillRectangleRenewed(double lAng, double rAng, double lDis, double rDis, char ch) // 考虑到视野变形，长方形不会在视野中呈现长方形，需要做优化
{                                                                                      // 向frame中填充一个三维视图中的长方形
                                                                                       // 五个参数：左边相对于屏幕中央的角度差，左边相对于屏幕中央的角度差（左移为负右移为正），左边到原点距离，右边到原点距离，填充符号

    if (rAng < lAng)
    {
        swap(lAng, rAng);
        swap(lDis, rDis);
    }
    if (rAng - lAng > 180)
    {
        return; // 墙在身后;
    }
    if (rAng < -angularField / 2 || lAng > angularField / 2)
    {
        return; // 墙不在视野内
    }

    double lIndex = AngToIndex(lAng), rIndex = AngToIndex(rAng);
    int lPix = ClampWidth(CeilSP(lIndex)), rPix = ClampWidth(FloorSP(rIndex));
    double len = sqrt(lDis * lDis + rDis * rDis - 2 * lDis * rDis * sin(DEGtoRAD(rAng - lAng)));

    double sinlSideAng = sin(DEGtoRAD(rAng - lAng)) * (rDis / len);                                         // 提前求出，优化算法用
    double lSideAng = rDis * rDis > lDis * lDis + len * len ? M_PI - asin(sinlSideAng) : asin(sinlSideAng); // 墙两头与玩家所成三角形靠左的角，弧度制，之后解三角形要用

    for (int i = lPix; i <= rPix; i++)
    {
        double rSideAng = DEGtoRAD(180 - RADtoDEG(lSideAng) - (IndexToAng(i) - lAng)); // 正在渲染该列的注视点、墙左边与玩家所成三角形的右边角，弧度制
        double iHeight = DisToHeight(lDis * (sinlSideAng / sin(rSideAng)));
        // double iHeight = lHeight + (rHeight - lHeight) * ((i - lIndex) / (rIndex - lIndex));
        FillRow(i, ClampHeight(CeilSP(height * 0.5 - iHeight)), ClampHeight(FloorSP(height * 0.5 + iHeight)), ch);
    }
}

double formatAngle(double rawAng)
{ // 把角度变到[0,360)
    return rawAng - floor(rawAng / 360) * 360;
}

double formatAngle180(double rawAng)
{ // 把角度变到[-180,180)
    return formatAngle(rawAng + 180) - 180;
}

double GetAngle(double rawX, double rawY)
{ // 返回目标物体相对于玩家视角中心线的角度值，范围[-180,180]
    double X = rawX - posX, Y = rawY - posY;
    double angleRaw;
    if (Y == 0)
    {
        angleRaw = X >= 0 ? 90 : -90;
    }
    else
    {
        angleRaw = RADtoDEG(Y >= 0 ? atan(X / Y) : atan(X / Y) + M_PI);
    }
    return formatAngle180(angleRaw - angle);
}

double getDis(double rawX, double rawY)
{ // 返回目标物体相对于玩家视角中心线的距离
    return sqrt((rawX - posX) * (rawX - posX) + (rawY - posY) * (rawY - posY));
}

double getLen(double pos1X, double pos1Y, double pos2X, double pos2Y)
{ // 返回两点距离
    return sqrt((pos1X - pos2X) * (pos1X - pos2X) + (pos1Y - pos2Y) * (pos1Y - pos2Y));
}

void FillRectanglePos(double pos1X, double pos1Y, double pos2X, double pos2Y, char ch)
{ // 向frame中填充一个三维视图中的长方形
  // 五个参数：墙角两根柱子的XY值，填充符号
    FillRectangle(GetAngle(pos1X, pos1Y), GetAngle(pos2X, pos2Y), getDis(pos1X, pos1Y), getDis(pos2X, pos2Y), ch);
}

void FillRectangleRenewedPos(double pos1X, double pos1Y, double pos2X, double pos2Y, char ch)
{ // 向frame中填充一个三维视图中的长方形
  // 五个参数：墙角两根柱子的XY值，填充符号
    FillRectangleRenewedLen(GetAngle(pos1X, pos1Y), GetAngle(pos2X, pos2Y), getDis(pos1X, pos1Y), getDis(pos2X, pos2Y), ch, getLen(pos1X, pos1Y, pos2X, pos2Y));
}

void FillRectangleRenewedPosLen(double pos1X, double pos1Y, double pos2X, double pos2Y, char ch, double len)
{ // 向frame中填充一个三维视图中的长方形
  // 五个参数：墙角两根柱子的XY值，填充符号
    FillRectangleRenewedLen(GetAngle(pos1X, pos1Y), GetAngle(pos2X, pos2Y), getDis(pos1X, pos1Y), getDis(pos2X, pos2Y), ch, len);
}

// === 视觉函数 ===

void FillScreen(char ch)
{ // 全屏填充，仅供测试
    GotoHead();
    char fillstr[height * width + 1];
    for (int i = 0; i < height * width; i++)
    {
        fillstr[i] = ch;
    }
    fillstr[height * width] = 0;
    puts(fillstr);
    GotoHead();
}

void FillFrame(char ch)
{ // 用同一种符号填充frame
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            frame[i][j] = ch;
        }
    }
}

void FillGround(char ch)
{ // 用同一种符号填充地板(frame的下半部分)
    int halfHeight = FloorSP(height * 0.5);
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j <= halfHeight; j++)
        {
            frame[i][j] = ch;
        }
        for (int j = halfHeight + 1; j < height; j++)
        {
            frame[i][j] = ' ';
        }
    }
}

void FrameToRaw()
{ // 将二维的Frame数组同步到一维的FrameRaw数组
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            frameRaw[width * (height - 1 - j) + i] = frame[i][j];
        }
    }
    frameRaw[width * height] = 0;
}

void PushFrame()
{ // 帧上屏
    FrameToRaw();
    GotoHead();
    puts(frameRaw);
    GotoHead();
}

void PushFrameOri()
{ // 不转换，帧上屏

    GotoHead();
    for (int i = height - 1; i >= 0; i--)
    {
        for (int j = 0; j < width; j++)
        {
            putchar(frame[j][i]);
        }
    }
    GotoHead();
}

void ShowStat()
{
    printf("----------------\nStats:          \nX:%lf\t\nY:%lf\t\nAng:%lf\t\n----------------\n", posX, posY, angle);
}

void Congratulate()
{
    printf("-----------------------------\n| Congratulations! You win! |\n|    Press 'L' to leave.    |\n-----------------------------");
}

void CongratulateCenter()
{
    congratulation_text.PrintASCII(0, 0, 14);
}

// === 墙渲染函数 ===

int GetXGrid()
{ // 获取玩家所在小方格X值（即X坐标向下取整）
    return floor(posX);
}

int GetYGrid()
{ // 获取玩家所在小方格Y值（即Y坐标向下取整）
    return floor(posY);
}

void RenderWall(int type, int indexX, int indexY)
{ // type：0竖直1水平，渲染该类中下标处的墙壁
    switch (type)
    {
    case 0:
        if (wallVertical[indexX][indexY])
        // if(1)
        {
            FillRectangleRenewedPosLen(indexX, indexY, indexX, indexY + 1, '@', wallLength);
            // FillRectanglePos(indexX, indexY, indexX, indexY + 1, '@');
        }
        break;
    case 1:
        if (wallHorizontal[indexX][indexY])
        // if(1)
        {
            FillRectangleRenewedPosLen(indexX, indexY, indexX + 1, indexY, '+', wallLength);
            // FillRectanglePos(indexX, indexY, indexX + 1, indexY, '+');
        }
        break;
    }
}

void RenderGrid(int cooX, int cooY, int type)
{ // 渲染一个小方格的两面墙。type取1~4表示渲染1~4象限的方向
    /*
        RenderWall(0,cooX+1,cooY); //右侧
        RenderWall(0,cooX,cooY); //左侧
        RenderWall(1,cooX,cooY+1); //上侧
        RenderWall(1,cooX,cooY); //下侧
    */
    switch (type)
    {
    case 1:
        RenderWall(0, cooX + 1, cooY); // 右侧
        RenderWall(1, cooX, cooY + 1); // 上侧
        break;
    case 2:
        RenderWall(0, cooX, cooY);     // 左侧
        RenderWall(1, cooX, cooY + 1); // 上侧
        break;
    case 3:
        RenderWall(0, cooX, cooY); // 左侧
        RenderWall(1, cooX, cooY); // 下侧
        break;
    case 4:
        RenderWall(0, cooX + 1, cooY); // 右侧
        RenderWall(1, cooX, cooY);     // 下侧
        break;
    }
}
void UpdateFrame()
{ // 依次渲染所有墙
    FillGround('.');
    int cooX = clamp(GetXGrid(), 0, mapWidth - 1), cooY = clamp(GetYGrid(), 0, mapHeight - 1);
    for (int x = 0; x <= cooX; x++)
    { // 依次渲染玩家左下方的墙
        for (int y = 0; y <= cooY; y++)
        {
            RenderGrid(x, y, 3);
        }
    }
    for (int x = 0; x <= cooX; x++)
    { // 依次渲染玩家左上方的墙
        for (int y = mapHeight - 1; y >= cooY; y--)
        {
            RenderGrid(x, y, 2);
        }
    }
    for (int x = mapWidth - 1; x >= cooX; x--)
    { // 依次渲染玩家右下方的墙
        for (int y = 0; y <= cooY; y++)
        {
            RenderGrid(x, y, 4);
        }
    }
    for (int x = mapWidth - 1; x >= cooX; x--)
    { // 依次渲染玩家右上方的墙
        for (int y = mapHeight - 1; y >= cooY; y--)
        {
            RenderGrid(x, y, 1);
        }
    }
    PushFrame();
}

// === 光标检测函数 ===

void ResetMouse()
{
    SetCursorPos(screenWidth / 2, screenHeight);
}

double GetMouse()
{
    POINT mousePos;
    GetCursorPos(&mousePos);
    return mousePos.x - screenWidth / 2;
}

double GetAndResetMouse()
{
    double mousePos = GetMouse();
    ResetMouse();
    return mousePos;
}

// === 移动函数 ===

void RotateR()
{
    angle += rotSpeed * (deltaTime / 1000);
    angle = formatAngle(angle);
}

void RotateL()
{
    angle -= rotSpeed * (deltaTime / 1000);
    angle = formatAngle(angle);
}

bool Rotate(int arg)
{
    angle += arg * rotateScale;
    angle = formatAngle(angle);
    return arg;
}

void MoveFowardRaw()
{
    posX += moveSpeed * sin(DEGtoRAD(angle)) * (deltaTime / 1000);
    posY += moveSpeed * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
}

void MoveFoward()
{
    double NposX = posX + moveSpeed * sin(DEGtoRAD(angle)) * (deltaTime / 1000), NposY = posY + moveSpeed * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
    if (floor(NposX) == floor(posX) && abs(NposX - round(NposX)) > abs(posX - round(posX)))
    {

        posX = NposX;
    }
    else if (-wallThick <= NposX - round(NposX) && NposX - round(NposX) <= wallThick)
    {
        if (wallVertical[(int)round(NposX)][(int)floor(posY)] == 1)
        {
            NposX = posX;
        }
    }
    posX = NposX;
    if (floor(NposY) == floor(posY) && abs(NposY - round(NposY)) > abs(posY - round(posY)))
    {

        posY = NposY;
    }
    else if (-wallThick <= NposY - round(NposY) && NposY - round(NposY) <= wallThick)
    {
        if (wallHorizontal[(int)floor(posX)][(int)round(NposY)] == 1)
        {
            NposY = posY;
        }
    }
    posY = NposY;
}

void MoveBackRaw()
{
    posX -= moveSpeed * moveScale * sin(DEGtoRAD(angle)) * (deltaTime / 1000);
    posY -= moveSpeed * moveScale * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
}

void MoveBack()
{
    double NposX = posX - moveSpeed * moveScale * sin(DEGtoRAD(angle)) * (deltaTime / 1000), NposY = posY - moveSpeed * moveScale * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
    if (floor(NposX) == floor(posX) && abs(NposX - round(NposX)) > abs(posX - round(posX)))
    {

        posX = NposX;
    }
    else if (-wallThick <= NposX - round(NposX) && NposX - round(NposX) <= wallThick)
    {
        if (wallVertical[(int)round(NposX)][(int)floor(posY)] == 1)
        {
            NposX = posX;
        }
    }
    posX = NposX;
    if (floor(NposY) == floor(posY) && abs(NposY - round(NposY)) > abs(posY - round(posY)))
    {

        posY = NposY;
    }
    else if (-wallThick <= NposY - round(NposY) && NposY - round(NposY) <= wallThick)
    {
        if (wallHorizontal[(int)floor(posX)][(int)round(NposY)] == 1)
        {
            NposY = posY;
        }
    }
    posY = NposY;
}

void MoveRight()
{
    double NposX = posX + moveSpeed * moveScale * sin(DEGtoRAD(angle + 90)) * (deltaTime / 1000), NposY = posY + moveSpeed * moveScale * cos(DEGtoRAD(angle + 90)) * (deltaTime / 1000);
    if (floor(NposX) == floor(posX) && abs(NposX - round(NposX)) > abs(posX - round(posX)))
    {

        posX = NposX;
    }
    else if (-wallThick <= NposX - round(NposX) && NposX - round(NposX) <= wallThick)
    {
        if (wallVertical[(int)round(NposX)][(int)floor(posY)] == 1)
        {
            NposX = posX;
        }
    }
    posX = NposX;
    if (floor(NposY) == floor(posY) && abs(NposY - round(NposY)) > abs(posY - round(posY)))
    {

        posY = NposY;
    }
    else if (-wallThick <= NposY - round(NposY) && NposY - round(NposY) <= wallThick)
    {
        if (wallHorizontal[(int)floor(posX)][(int)round(NposY)] == 1)
        {
            NposY = posY;
        }
    }
    posY = NposY;
}

void MoveLeft()
{
    double NposX = posX + moveSpeed * moveScale * sin(DEGtoRAD(angle - 90)) * (deltaTime / 1000), NposY = posY + moveSpeed * moveScale * cos(DEGtoRAD(angle - 90)) * (deltaTime / 1000);
    if (floor(NposX) == floor(posX) && abs(NposX - round(NposX)) > abs(posX - round(posX)))
    {

        posX = NposX;
    }
    else if (-wallThick <= NposX - round(NposX) && NposX - round(NposX) <= wallThick)
    {
        if (wallVertical[(int)round(NposX)][(int)floor(posY)] == 1)
        {
            NposX = posX;
        }
    }
    posX = NposX;
    if (floor(NposY) == floor(posY) && abs(NposY - round(NposY)) > abs(posY - round(posY)))
    {

        posY = NposY;
    }
    else if (-wallThick <= NposY - round(NposY) && NposY - round(NposY) <= wallThick)
    {
        if (wallHorizontal[(int)floor(posX)][(int)round(NposY)] == 1)
        {
            NposY = posY;
        }
    }
    posY = NposY;
}

bool Move()
{
    double NposX = posX, NposY = posY;
    int ifMoveX = -2, ifMoveY = -2;
    if (KEY_DOWN(65))
    {
        NposX += moveSpeed * moveScale * sin(DEGtoRAD(angle - 90)) * (deltaTime / 1000);
        NposY += moveSpeed * moveScale * cos(DEGtoRAD(angle - 90)) * (deltaTime / 1000);
    }
    if (KEY_DOWN(68))
    {
        NposX += moveSpeed * moveScale * sin(DEGtoRAD(angle + 90)) * (deltaTime / 1000);
        NposY += moveSpeed * moveScale * cos(DEGtoRAD(angle + 90)) * (deltaTime / 1000);
    }
    if (KEY_DOWN(87))
    {
        NposX += moveSpeed * sin(DEGtoRAD(angle)) * (deltaTime / 1000);
        NposY += moveSpeed * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
    }
    if (KEY_DOWN(83))
    {
        NposX -= moveSpeed * moveScale * sin(DEGtoRAD(angle)) * (deltaTime / 1000);
        NposY -= moveSpeed * moveScale * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
    }
    if (NposX != posX)
    {
        ifMoveX++;
    }
    if (NposY != posY)
    {
        ifMoveY++;
    }
    if (floor(NposX) == floor(posX) && abs(NposX - round(NposX)) > abs(posX - round(posX)))
    {
        posX = NposX;
        ifMoveX++;
    }
    else if (-wallThick <= NposX - round(NposX) && NposX - round(NposX) <= wallThick && wallVertical[(int)round(NposX)][(int)floor(posY)] == 1)
    {
        NposX = posX;
    }
    else
    {
        posX = NposX;
        ifMoveX++;
    }
    if (floor(NposY) == floor(posY) && abs(NposY - round(NposY)) > abs(posY - round(posY)))
    {
        posY = NposY;
        ifMoveY++;
    }
    else if (-wallThick <= NposY - round(NposY) && NposY - round(NposY) <= wallThick && wallHorizontal[(int)floor(posX)][(int)round(NposY)] == 1)
    {
        NposY = posY;
    }
    else
    {
        posY = NposY;
        ifMoveY++;
    }
    return !ifMoveX || !ifMoveY;
}

bool MoveRaw()
{
    double NposX = posX, NposY = posY;
    int ifMoveX = -1, ifMoveY = -1;
    if (KEY_DOWN(65))
    {
        NposX += moveSpeed * moveScale * sin(DEGtoRAD(angle - 90)) * (deltaTime / 1000);
        NposY += moveSpeed * moveScale * cos(DEGtoRAD(angle - 90)) * (deltaTime / 1000);
    }
    if (KEY_DOWN(68))
    {
        NposX += moveSpeed * moveScale * sin(DEGtoRAD(angle + 90)) * (deltaTime / 1000);
        NposY += moveSpeed * moveScale * cos(DEGtoRAD(angle + 90)) * (deltaTime / 1000);
    }
    if (KEY_DOWN(87))
    {
        NposX += moveSpeed * sin(DEGtoRAD(angle)) * (deltaTime / 1000);
        NposY += moveSpeed * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
    }
    if (KEY_DOWN(83))
    {
        NposX -= moveSpeed * moveScale * sin(DEGtoRAD(angle)) * (deltaTime / 1000);
        NposY -= moveSpeed * moveScale * cos(DEGtoRAD(angle)) * (deltaTime / 1000);
    }
    if (NposX != posX)
    {
        ifMoveX++;
    }
    if (NposY != posY)
    {
        ifMoveY++;
    }
    posX = NposX, posY = NposY;
    return !ifMoveX || !ifMoveY;
}

bool ifWin()
{
    return !(posX >= 0 && posY >= 0 && posX <= mapWidth && posY <= mapHeight);
}

// === 全屏函数 ===

void FullScreen()
{
    HWND hwnd = GetForegroundWindow();
    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);
    LONG l_WinStyle = GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, (l_WinStyle | WS_MAXIMIZE | WS_POPUP) & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_BORDER);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, cx, cy, 0);
}

// === 读取地图信息 ===
bool ReadMap(string path)
{ // 读取地图文件
    // 创建流对象
    ifstream ifs;
    ifs.open(path, ios::in);

    // 判断文件是否成功打开成功则读取数据
    if (!ifs.is_open())
    {
        ifs.close();
        return false;
    }
    ifs >> mapWidth >> mapHeight >> posXInit >> posYInit >> angleInit;
    ifs.get();

    vector<vector<bool>> sampleVertical(mapWidth + 1, vector<bool>(mapHeight, 0)), sampleHorizontal(mapWidth, vector<bool>(mapHeight + 1, 0));
    wallVertical = sampleVertical, wallHorizontal = sampleHorizontal;

    string buf;
    getline(ifs, buf);
    for (int j = 0; j < mapWidth; j++)
    {
        wallHorizontal[j][mapHeight] = (buf[2 * j + 1] == '_' ? 1 : 0);
    }
    for (int i = mapHeight - 1; i >= 0; i--)
    {
        getline(ifs, buf);
        for (int j = 0; j < mapWidth; j++)
        {
            wallHorizontal[j][i] = (buf[2 * j + 1] == '_' ? 1 : 0);
        }
        for (int j = 0; j <= mapWidth; j++)
        {
            wallVertical[j][i] = (buf[2 * j] == '|' ? 1 : 0);
        }
    }
    // cout << "Map file loaded." << endl;
    //  关闭文件
    ifs.close();
    return true;
}

// === 各界面函数 ===

bool InitResolution()
{
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    ifstream ifs;
    ifs.open("configs\\config.txt", ios::in);
    // 判断文件是否成功打开成功则读取数据
    if (!ifs.is_open())
    {
        ifs.close();
        return false;
    }
    ifs >> width >> height;
    ifs.close();
    return true;
}

void Init()
{
    HideCursor();
    FillFrame(' ');
    InitResolution();
    title.read("arts\\title.txt");
    version.read("arts\\v2_0.txt");
    map_select.read("arts\\map_select.txt");
    congratulation_text.read(("arts\\congratulation_text.txt"));
    controls.read(("arts\\controls.txt"));
    controls_text.read(("arts\\controls_text.txt"));
    frameRaw[width * height] = 0;
}

/*
void Init()
{
    FillFrame(' ');
    frameRaw[width * height] = 0;
    moveSpeed = moveSpeedWalk;
    angle = angleInit = 0, posX = posXInit = 1.5, posY = posYInit = 1.5;
    mapWidth = mapHeight = 3;
    wallVertical = {{1, 1, 1}, {1, 1, 0}, {0, 1, 1}, {1, 1, 0}};
    wallHorizontal = {{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}};
}
*/

void MainScreen()
{
    FillScreen(' ');
    title.PrintASCII(0, -18, 11);
    version.PrintASCII(0, -8, 3);
    PrintStr("W/S: Switch Options   Enter: Select", 0, 5);

    int len = 3;
    int posMain0 = 15;
    char ch;
    int opt = 0, optL = 0;
    for (int i = 0; i < len; i++)
    {
        PrintStr(mainScreenOpts[i], 0, posMain0 + 2 * i);
    }
    PrintStr(mainScreenOpts[0], 0, posMain0, 14);
    while ((ch = _getch()) != '\r')
    {
        if (ch == 's' || ch == 'S')
        {
            opt++;
        }
        if (ch == 'w' || ch == 'W')
        {
            opt--;
        }
        opt = (opt + len) % len;
        if (opt != optL)
        {
            PrintStr(mainScreenOpts[opt], 0, posMain0 + 2 * opt, 14);
            PrintStr(mainScreenOpts[optL], 0, posMain0 + 2 * optL);
        }
        optL = opt;
    }
    switch (opt)
    {
    case 0:
        pageSelect = MAP_SELECT;
        break;
    case 1:
        pageSelect = CONTROLS;
        break;
    case 2:
        exit(0);
    default:
        break;
    }
}

bool GetMapList()
{ // 从 "maps\\maplist.txt 读取地图文件列表

    mapList.clear();

    // 创建流对象
    ifstream ifs;
    ifs.open("maps\\maplist.txt", ios::in);

    // 判断文件是否成功打开成功则读取数据
    if (!ifs.is_open())
    {
        ifs.close();
        return false;
        exit(0);
    }

    int listLength;
    ifs >> listLength;
    ifs.get();

    for (int i = 0; i < listLength; i++)
    {
        pair<string, string> newLevel;
        getline(ifs, newLevel.first, ',');
        getline(ifs, newLevel.second);
        mapList.push_back(newLevel);
    }
    // 关闭文件
    ifs.close();
    return true;
}

int ShowMaps()
{
    int len = mapList.size();
    int posMap0 = 3;
    char ch;
    int opt = 0, optL = 0;
    for (int i = 0; i < len; i++)
    {
        PrintStr(mapList[i].second, 0, posMap0 + 2 * i);
    }
    PrintStr(mapList[0].second, 0, posMap0, 14);
    while ((ch = _getch()) != '\r')
    {
        if (ch == 's' || ch == 'S')
        {
            opt++;
        }
        if (ch == 'w' || ch == 'W')
        {
            opt--;
        }
        opt = (opt + len) % len;
        if (opt != optL)
        {
            PrintStr(mapList[opt].second, 0, posMap0 + 2 * opt, 14);
            PrintStr(mapList[optL].second, 0, posMap0 + 2 * optL);
        }
        optL = opt;
    }
    return opt;
}

void MapSelect()
{
    FillScreen(' ');
    if (!GetMapList())
    {
        PrintStr("'maps//maplist.txt' doesn't exist! Press 'Enter' to continue.", 0, 0, 4);
        pageSelect = MAIN_SCREEN;
        while (_getch() != '\r')
            ;
        return;
    }
    if (mapList.empty())
    {
        PrintStr("There is no map in 'maps\\'! Press 'Enter' to continue.", 0, 0, 4);
        pageSelect = MAIN_SCREEN;
        while (_getch() != '\r')
            ;
        return;
    }
    map_select.PrintASCII(0, -18, 11);
    int opt = ShowMaps();
    FillScreen(' ');
    if (!ReadMap("maps\\" + mapList[opt].first))
    {
        PrintStr("'maps\\" + mapList[opt].first + "' doesn't exist! Press 'Enter' to continue.", 0, 0, 4);
        pageSelect = MAIN_SCREEN;
        while (_getch() != '\r')
            ;
        return;
    }
    pageSelect = GAME;
    return;
}

void Controls()
{
    FillScreen(' ');
    controls.PrintASCII(0, -18, 11);
    controls_text.PrintASCII(0, -2);
    while (_getch() != '\r')
        ;
    pageSelect = MAIN_SCREEN;
    return;
}

/*
int main()
{
    getchar();
    FillGround('.');
    FillRectangle(-20, 20, 1.5, 2, '@');
    PushFrame();
    return 0;
}
*/

/*
int main()
{
    Init();
    FullScreen();
    printf("Please fullscreen before playing!\n\nW:Move Forward\nS:Move Backword\nA/D:Rotate Left/Right\nHolding Shift:Run\nL:Leave The Game\n\nPress 'Enter' to continue.");
    while (_getch() != '\r' && _getch() != '\n');
    bool flag = 1;
    while (1)
    {
        if (KEY_DOWN(16))
        {
            moveSpeed = moveSpeedRun;
        }
        else
        {
            moveSpeed = moveSpeedWalk;
        }
        if (KEY_DOWN(65) && KEY_UP(68))
        {
            RotateL();
            flag = 1;
        }
        if (KEY_DOWN(68) && KEY_UP(65))
        {
            RotateR();
            flag = 1;
        }
        if (KEY_DOWN(87) && KEY_UP(83))
        {
            MoveRaw();
            flag = 1;
        }
        if (KEY_DOWN(83) && KEY_UP(87))
        {
            MoveBackRaw();
            flag = 1;
        }
        if (KEY_DOWN(76))
        {
            FillScreen(' ');
            break;
        }
        if (flag)
        {
            FillGround('.');
            FillRectanglePos(2, 2, 2, 3, '@');
            PushFrame();
            ShowStat();
            flag = 0;
        }
        Sleep(deltaTime);
    }
    while (_getch() != 'l')
        ;
    GotoHead();
    printf("You have left the Game. Press Any key to continue.");
    getchar();
}
*/

void Game()
{
    moveSpeed = moveSpeedWalk;
    angle = angleInit, posX = posXInit, posY = posYInit;
    // PlaySound(TEXT("musics\\(icicles.wav"),NULL,SND_FILENAME | SND_ASYNC);
    bool flag = 1, win = 0;
    do
    {
        if (KEY_DOWN(16))
        {
            moveSpeed = moveSpeedRun;
        }
        else
        {
            moveSpeed = moveSpeedWalk;
        }
        if (Move())
        {
            flag = 1;
        }
        if (Rotate(GetAndResetMouse()))
        {
            flag = 1;
        }
        if (KEY_DOWN(76))
        {
            FillScreen(' ');
            break;
        }
        if (flag)
        {
            UpdateFrame();
            ShowStat();
            flag = 0;
        }
        if (ifWin())
        {
            win = 1;
            break;
        }
        Sleep(deltaTime);
    } while (1);
    if (win)
    {
        CongratulateCenter();
        while (_getch() != '\r')
            ;
    }
    else
    {
        while (_getch() != 'l')
            ;
    }
    GotoHead();
    FillScreen(' ');
    pageSelect = MAIN_SCREEN;
    // PlaySound(TEXT("musics\\the_lava_dwellers.wav"),NULL,SND_FILENAME | SND_ASYNC);
}

// === 主函数 ===

/*
int main()
{
    Init();
    FullScreen();
    printf("Please fullscreen before playing!\n\nW:Move Forward\nS:Move Backword\nA/D:Rotate Left/Right\nHolding Shift:Run\nL:Leave The Game\n\nPress 'Enter' to continue.");
    while (_getch() != '\r' && _getch() != '\n')
        ;
    bool flag = 1, win = 0;
    while (1)
    {
        if (KEY_DOWN(16))
        {
            moveSpeed = moveSpeedRun;
        }
        else
        {
            moveSpeed = moveSpeedWalk;
        }
        if (KEY_DOWN(65) && KEY_UP(68))
        {
            RotateL();
            flag = 1;
        }
        if (KEY_DOWN(68) && KEY_UP(65))
        {
            RotateR();
            flag = 1;
        }
        if (KEY_DOWN(87) && KEY_UP(83))
        {
            Move();
            flag = 1;
        }
        if (KEY_DOWN(83) && KEY_UP(87))
        {
            MoveBack();
            flag = 1;
        }
        if (KEY_DOWN(76))
        {
            FillScreen(' ');
            break;
        }
        if (flag)
        {
            UpdateFrame();
            ShowStat();
            flag = 0;
        }
        if (ifWin())
        {
            win = 1;
            break;
        }
        Sleep(deltaTime);
    }
    if (win)
    {
        CongratulateCenter();
    }
    while (_getch() != 'l')
        ;
    GotoHead();
    FillScreen(' ');
    printf("You have left the Game. Press Any key to continue.");
    getchar();
}
*/

void WaitFullScreen(){
    GotoHead();
    printf("Please FullScreen Before Playing!\nPress 'F11' to FullScreen if the terminal didn't FullScreen automatically.\n\n[ Press any key ]");
    getchar();
}

int main()
{
    Init();
    FullScreen();
    WaitFullScreen();
    pageSelect = MAIN_SCREEN;
    // PlaySound(TEXT("musics\\the_lava_dwellers.wav"),NULL,SND_FILENAME | SND_ASYNC);
    while (1)
    {
        switch (pageSelect)
        {
        case MAIN_SCREEN:
            MainScreen();
            break;
        case MAP_SELECT:
            MapSelect();
            break;
        case GAME:
            Game();
            break;
        case CONTROLS:
            Controls();
            break;
        default:
            break;
        }
    }
}
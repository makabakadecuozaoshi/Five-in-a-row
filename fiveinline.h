#ifndef FIVEINLINE_H
#define FIVEINLINE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class FiveInLine; }
QT_END_NAMESPACE
//界面 580 * 580 固定
//外边距
#define FIL_MARGIN_HEIGHT   (50)
#define FIL_MARGIN_WIDTH    (50)
//行列数 15* 15
#define FIL_COLS            (15)
#define FIL_ROWS            (15)
//边距离
#define FIL_SPACE           (30)
//棋子大小
#define FIL_PIECE_SIZE      (26)
//棋盘边缘缩进距离
#define FIL_DISTANCE        (10)
//1.绘制棋盘  网格线  棋子
//2.鼠标点击出现棋子，棋子跟随按下的鼠标移动，松开鼠标棋子落下（鼠标点击 移动 释放 事件）
//3.定时器 定时刷新界面 1秒 25帧
//4.落子 出界放弃 合法抛出信号
//槽函数 更新数组，切换回合
//5.判断输赢
//在 条线的一组相邻 方便写为循环
//1.根据当前点,依次査看 左右,上下,左上右下，左下右上,四条直线上的棋子个数
//2.初始棋子个数为1,然后根据方向换算新的坐标,查看是否出界,出界,break
    //,不然看是否和当前棋子同色,如果是继续看,个数+1,不然break如果count到5,结束,没有到5 则继续看其他直线
//3.所有都看完没到5 则游戏没结束.

#include<vector>
#include<QPaintEvent>
#include<QTimer>
#include<vector>
#include<unordered_set>
#include<QPainter>
#include<QMessageBox>
#include<qDebug>
using namespace std;
///
/// \brief The FiveInLine class
///     AI 赢法结构体
#define DEFAULT_COUNTER 30

struct stru_win
{
    stru_win():board(FIL_COLS ,vector<int>( FIL_ROWS , 0 ))
      ,playerCount(0),cpuCount(0)
    {
        //clear();
    }
    void clear()
    {
        playerCount = 0;
        cpuCount = 0;
    }
    vector<vector<int>> board;//胜利棋子布局

    int playerCount;//该赢法玩家棋子个数

    int cpuCount;//该赢法电脑棋子个数
};
class FiveInLine : public QWidget
{
    Q_OBJECT

signals:
    //落子信号：鼠标释放捕捉落子位置(颜色 位置)
    void SIG_pieceDown(int blackorwhite , int x ,int y);
    //胜利信号
    void SIG_playerWin(int blackorwhite );
public slots:
    //落子槽函数：鼠标释放捕捉落子位置(颜色 位置)
    void slot_pieceDown(int blackorwhite , int x ,int y);
    //开始游戏
    void slot_startGame();
    //回合定时器
    void slot_countTimer();
public:
    FiveInLine(QWidget *parent = nullptr);
    ~FiveInLine();
    //绘图 重绘事件 绘制背景 棋盘 棋子
    void paintEvent(QPaintEvent *event);
    //鼠标点击  出现棋子
    void mousePressEvent(QMouseEvent *event);
    //移动 棋子跟随按下的鼠标移动
    void mouseMoveEvent(QMouseEvent *event);
    //释放 棋子落下
    void mouseReleaseEvent(QMouseEvent *event);
    //判断是否出界
    bool isCrossLie(int x,int y);
    //获取当前回合
    int getBlackOrWhite() const;
    //切换当前回合
    void changeBlackAndWhite();
    //是否获胜
    bool isWin(int x,int y);
    //清空
    void clear();
    //设置状态函数
    void setSelfStatus(int _status);
    //棋子颜色 0空 黑1 白2
    enum ENUM_BLACK_OR_WHITE{None = 0 , Black , White};
    //初始化电脑AI所有赢法 棋子布局
    void InitAiVector();
    //AI电脑落子
    void pieceDownByCpu();
    //设置电脑回合颜色
    void setCpuColor(int newCpuColor);
private:
    Ui::FiveInLine *ui;
    //记录黑白子 二维数组
    vector<vector<int>> m_board;
    //画刷数组 黑白棋子颜色
    QBrush m_pieceColor[3];
    //鼠标移动中的位置
    QPoint m_movePoint;
    //移动标志 鼠标是否松开 点击为真true
    bool m_moveFlag;
    //记录当前回合
    int m_blackOrWhite;
    //重绘定时器
    QTimer m_timer;
    //方向
    enum enum_direction{d_z,d_y,d_s,d_x,d_zs,d_yx,d_zx,d_ys,d_count};
    //根据方向对坐标的偏移 每次是一个单位
    int dx[d_count] ={-1 , 1 , 0 , 0 , -1 , 1 , -1 ,  1};
    int dy[d_count] ={ 0 , 0 , -1, 1 , -1 , 1 ,  1 , -1};
    //结束标志
    bool m_isOver;
    //网络版本 玩家身份 不是自己回合不可以动
    int m_status;
    /*电脑 AI 赢法数组*/
    vector<stru_win> m_vecWin;
    //电脑回合颜色
    int m_cpuColor;
    //回合定时
    //每秒的定时
    QTimer m_countTimer;
    //剩余时间计数器
    int m_colorCount;   //初始30s DEFAULT_COUNTER 30
    //AI maxmin搜索博弈树+α-β剪枝
private:
    //8个方向
    vector<pair<int,int>> directions=
    {
        {0,-1},{0,1},  //上下
        {-1,1},{1,-1}, //左上右下
        {1,0} ,{-1,0},  //左右
        {1,1} ,{-1,-1}  //左下右上
    };
    //设置最大深度
    const int MAX_DEPTH=4;
    QPoint m_lastPos;
    vector<pair<int,int>> m_everyStepPos;
    //为了方便后续判断棋盘是否结束，这里实现重载新的isWin判断
    bool isWin(int x,int y,vector<vector<int>> &board);
    //添加 新ai的接口
    //AI电脑落子
    void pieceDownByBetterCpu();
    //获取最佳位置，设置maxmin搜索深度
    void findBestMove(int &bestX, int &bestY, int player, int depth);
    //获得后选点 这是要遍历查看 可能下棋的点 因为黑子先下棋，电脑后下，所以一定能找到
    void getNeedHandlePos(vector<pair<int,int>> &copyEveryStep,
                          vector<pair<int,int>> &candidates,
                          vector<vector<int>> &board);
    //实现maxmin搜索+剪枝
    int minmax(vector<vector<int>> &copyBoard,vector<pair<int,int>>& copyEveryStep,
               int depth,int alpha,int beta,bool isMaximizing,int player);
    //评估棋盘算法
    //分析一次白棋的得分 分析一次黑骑的得分，然后做差
    int evaluateBoard(int color,vector<vector<int>>& board);
    //总体的棋面分析
    int evaluateBoard(vector<vector<int>>& board);
};
#define DEF_ALPHA_BETA 1
#endif // FIVEINLINE_H

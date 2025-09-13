#include "fiveinline.h"
#include "ui_fiveinline.h"



FiveInLine::FiveInLine(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FiveInLine)
    , m_board( FIL_COLS , vector<int>(FIL_ROWS , 0))//vector 初始化 有参构造 多长 初值多少
    , m_movePoint(0,0)
    , m_moveFlag(false)
    , m_blackOrWhite(Black)
    , m_isOver(false)
    , m_status(Black)
    , m_colorCount(DEFAULT_COUNTER)
{
    ui->setupUi(this);

    //初始化AI棋谱//初始化电脑AI所有赢法 棋子布局
    InitAiVector();
    //设置AI棋子颜色
    setCpuColor(White);
    //棋子颜色
    m_pieceColor[None] = QBrush(QColor(0 , 0 , 0 , 0)); //最后一个0为全透明
    m_pieceColor[Black] = QBrush(QColor(0 , 0 , 0));
    m_pieceColor[White] = QBrush(QColor(255 , 255 , 255));

//    //测试
//    m_board[1][0] = Black;
//    m_board[2][2] = White;

    //定时器
    connect(&m_timer , SIGNAL(timeout()) , this , SLOT(repaint()));
    //repaint触发重绘
    m_timer.start(1000/25); //1秒25次

    //通过Kernel转发，这个要禁止
//    //落子处理
//    //联网Ckernl接,
    connect(this , SIGNAL(SIG_pieceDown(int,int,int)),
            this , SLOT(slot_pieceDown(int,int,int)));
//    //测试开始游戏
//    slot_startGame();

    //回合定时器
    //clear();
    //定时器
    //修改9.12
    //connect(&m_countTimer , SIGNAL(timeout()) , this , SLOT(slot_countTimer()));
    //开始禁止操作
    slot_startGame();
    for(int i=0;i<8;i++)
    {
        m_worker[i]=new MyWorker(this);
        connect(m_worker[i],&MyWorker::SIG_getScoreFinish,
                this,[this](int x,int y,int score)
                {
                m_taskCount--;
                m_vecScoreRes.push_back({x,y,score});
                if(m_taskCount==0)
                {
                    sort(m_vecScoreRes.begin(),m_vecScoreRes.end(),[&](vector<int>&a,vector<int>&b)
                    {
                        return a[2]>b[2];
                    });
                    int x=m_vecScoreRes[0][0];
                    int y=m_vecScoreRes[0][1];
                    if(x!=-1&&y!=-1)
                    emit SIG_pieceDown(getBlackOrWhite(),x,y);
                    m_vecScoreRes.clear();
                }

                });
    }
    connect(this,SIGNAL(SIG_getBetterScore0(int,int,int))
            ,m_worker[0],SLOT(slot_getBetterScore(int,int,int)));
    connect(this,SIGNAL(SIG_getBetterScore1(int,int,int))
            ,m_worker[1],SLOT(slot_getBetterScore(int,int,int)));
    connect(this,SIGNAL(SIG_getBetterScore2(int,int,int))
            ,m_worker[2],SLOT(slot_getBetterScore(int,int,int)));
    connect(this,SIGNAL(SIG_getBetterScore3(int,int,int))
            ,m_worker[3],SLOT(slot_getBetterScore(int,int,int)));
    connect(this,SIGNAL(SIG_getBetterScore4(int,int,int))
            ,m_worker[4],SLOT(slot_getBetterScore(int,int,int)));
    connect(this,SIGNAL(SIG_getBetterScore5(int,int,int))
            ,m_worker[5],SLOT(slot_getBetterScore(int,int,int)));
    connect(this,SIGNAL(SIG_getBetterScore6(int,int,int))
            ,m_worker[6],SLOT(slot_getBetterScore(int,int,int)));
    connect(this,SIGNAL(SIG_getBetterScore7(int,int,int))
            ,m_worker[7],SLOT(slot_getBetterScore(int,int,int)));
}

FiveInLine::~FiveInLine()
{
    m_timer.stop();
    delete ui;
}


//绘图 重绘事件 绘制背景 棋盘 棋子
void FiveInLine::paintEvent(QPaintEvent *event)
{
    //绘制棋盘
    QPainter painer(this);  //QPainter 用于绘图 使用有参构造 传入当前对象 得到当前控件得画布
    //设置颜色 通过画刷设置 000黑
    painer.setBrush(QBrush(QColor(255 , 160 , 0)));
    //画图 矩形
    painer.drawRect(FIL_MARGIN_WIDTH - FIL_DISTANCE ,
                    FIL_MARGIN_HEIGHT - FIL_DISTANCE ,
                    (FIL_COLS + 1) * FIL_SPACE + 2 * FIL_DISTANCE,
                    (FIL_ROWS + 1) * FIL_SPACE + 2 * FIL_DISTANCE);
    //绘制网格线 15*15
    //先竖 后横
    for(int i = 1 ; i<= FIL_COLS ; ++i)
    {
        //设置颜色 通过画刷设置 000黑
        painer.setBrush(QBrush(QColor(0 , 0 , 0)));
        //起点
        QPoint p1(FIL_MARGIN_WIDTH + FIL_SPACE * i , FIL_MARGIN_HEIGHT + FIL_SPACE);
        QPoint p2(FIL_MARGIN_WIDTH + FIL_SPACE * i , FIL_MARGIN_HEIGHT + (FIL_ROWS) * FIL_SPACE);
        painer.drawLine(p1 , p2);
    }

    for(int i = 1 ; i<= FIL_ROWS ; ++i)
    {
        //设置颜色 通过画刷设置 000黑
        painer.setBrush(QBrush(QColor(0 , 0 , 0)));
        //起点 终点
        QPoint p1(FIL_MARGIN_WIDTH + FIL_SPACE  , FIL_MARGIN_HEIGHT + FIL_SPACE * i);
        QPoint p2(FIL_MARGIN_WIDTH + (FIL_COLS) * FIL_SPACE  , FIL_MARGIN_HEIGHT + FIL_SPACE * i);
        painer.drawLine(p1 , p2);
    }

    //画棋子
    //x y 对应颜色 调用画刷数组 得到对应颜色
    for(int x = 0; x<FIL_COLS ; ++x)
    {
        for(int y =0 ; y < FIL_ROWS ; ++y)
        {
            if(m_board[x][y] != None)
            {
                painer.setBrush(m_pieceColor[m_board[x][y]]);   //通过画刷设置颜色
                //画圆形(圆心坐标 ， 半径x , 半径y)
                painer.drawEllipse(
                QPoint(FIL_MARGIN_WIDTH + FIL_SPACE + FIL_SPACE* x,FIL_MARGIN_HEIGHT + FIL_SPACE + FIL_SPACE * y)
                            , FIL_PIECE_SIZE / 2 , FIL_PIECE_SIZE / 2);
            }
        }
    }

    //标注中心点
    //设置颜色 通过画刷设置 000黑
    painer.setBrush(QBrush(QColor(0 , 0 , 0)));
    //画圆形(圆心坐标 ， 半径x , 半径y)
    painer.drawEllipse( QPoint( 290 , 290) , 6/2 , 6/2 );


    //显示移动棋子
    if(m_moveFlag)
    {
        //黑白子（当前回合是谁）
        //画点
        painer.setBrush(m_pieceColor[getBlackOrWhite()]);   //通过画刷设置颜色
        //画圆形(圆心坐标 ， 半径x , 半径y)
        painer.drawEllipse(
        QPoint( m_movePoint.x()
               ,m_movePoint.y())
                    , FIL_PIECE_SIZE / 2 , FIL_PIECE_SIZE / 2);
    }
    //绘制上一次落子的位置
    if(!m_lastPos.isNull())
    {
        painer.setBrush(QBrush(QColor(0 , 250 , 0)));
        //左上
        int x1=FIL_MARGIN_WIDTH + FIL_SPACE + FIL_SPACE* m_lastPos.x()-FIL_PIECE_SIZE / 2;
        int y1=FIL_MARGIN_HEIGHT + FIL_SPACE + FIL_SPACE * m_lastPos.y()-FIL_PIECE_SIZE / 2;
        //右上
        int x2=x1+FIL_PIECE_SIZE;
        int y2=y1;
        //左下
        int x3=x1;
        int y3=y1+FIL_PIECE_SIZE;
        //右下
        int x4=x2;
        int y4=y3;

        painer.drawLine(QPoint(x1,y1),QPoint(x2,y2));
        painer.drawLine(QPoint(x1,y1),QPoint(x3,y3));
        painer.drawLine(QPoint(x2,y2),QPoint(x4,y4));
        painer.drawLine(QPoint(x3,y3),QPoint(x4,y4));
    }
    //事件执行
    event->accept();
}

//鼠标点击  出现棋子
void FiveInLine::mousePressEvent(QMouseEvent *event)
{
    m_movePoint = event->pos();

    //捕捉点 ：相对坐标（相对窗口）
    //相对坐标（相对窗口）QMouseEvent::pos() ， 绝对坐标（相对桌面）QMouseEvent::globalPos()  QCursor::pos()鼠标位置
    //如果结束就不捕捉
    if(m_isOver)goto quit;
    //添加 玩家只能下黑子
    if(getBlackOrWhite()==White) goto quit;
    if(m_status != getBlackOrWhite()) goto quit;
    //点击状态
    m_moveFlag = true;

quit:
    event->accept();
}

//移动 棋子跟随按下的鼠标移动
void FiveInLine::mouseMoveEvent(QMouseEvent *event)
{
    //更新相对坐标
    if(m_moveFlag)
    {
        m_movePoint = event->pos();
    }
    event->accept();
}

//释放 棋子落下
void FiveInLine::mouseReleaseEvent(QMouseEvent *event)
{
    m_moveFlag = false;

    //停止捕捉
    //将相对坐标 换算为棋盘坐标 换算坐标存在四舍五入 取整问题
    //event->pos();
    //四舍五入
    int x = 0;
    int y = 0;
    float fx = (float)event->pos().x();
    float fy = (float)event->pos().y();

    if(m_isOver) goto quit;
    if(getBlackOrWhite()==White) goto quit;
    if(m_status != getBlackOrWhite())goto quit;
    fx = (fx - FIL_MARGIN_WIDTH - FIL_SPACE) / FIL_SPACE;
    fy = (fy - FIL_MARGIN_HEIGHT - FIL_SPACE) / FIL_SPACE;
    //根据小于0.5 大于0.5分两个情况
    x = (fx - (int)fx ) > 0.5 ? ((int)fx + 1) : (int) fx;
    y = (fy - (int)fy ) > 0.5 ? ((int)fy + 1) : (int) fy;
    //是否越界
    if(isCrossLie(x,y))
    {
        return;
    }
    Q_EMIT SIG_pieceDown(getBlackOrWhite() , x, y);
    //4.落子 出界放弃 合法抛出信号
quit:
    event->accept();
}

//判断是否出界
bool FiveInLine::isCrossLie(int x, int y)
{
    if( x<0 || x>=15 || y<0 || y>=15 )
    {
        return true;
    }
    return false;
}

//获取当前回合
int FiveInLine::getBlackOrWhite() const
{
    return m_blackOrWhite;
}

//切换当前回合
void FiveInLine::changeBlackAndWhite()
{
    //qDebug()<<__func__;
    // 0 1 2
    m_blackOrWhite = (m_blackOrWhite + 1);
    if(m_blackOrWhite == 3)
    {
        m_blackOrWhite = Black;
    }
    if(m_blackOrWhite == Black)
    {
        ui->lb_color->setText("黑方回合");
    }
    else
    {
        ui->lb_color->setText("白方回合");
    }

}

//是否获胜
bool FiveInLine::isWin(int x, int y)
{
    //看四条直线 8个方向同色棋子个数 有一个到5就结束
    int count = 1;
    //循环 4条
    //左
    for(int dr = d_z ; dr < d_count ; dr += 2)
    {
        for(int i = 1; i<=4 ;++i)
        {
            //获取偏移后棋子坐标
            int ix = dx[dr]*i + x;
            int iy = dy[dr]*i + y;
            //判断是否出界
            if(isCrossLie(ix,iy))
            {
                break;
            }
            //是否同色
            if(m_board[ix][iy] == m_board[x][y])
            {
                count++;
            }
            else
            {
                break;
            }
        }
        //右
        for(int i = 1; i<=4 ;++i)
        {
            //获取偏移后棋子坐标
            int ix = dx[dr + 1]*i + x;
            int iy = dy[dr + 1]*i + y;
            //判断是否出界
            if(isCrossLie(ix,iy))
            {
                break;
            }
            //是否同色
            if(m_board[ix][iy] == m_board[x][y])
            {
                count++;
            }
            else
            {
                break;
            }
        }
        if(count >= 5)//5个不在看其他直线
        {
            break;
        }
        else    //不够看其他直线
        {
            count = 1;
        }
    }
    if(count >= 5)
    {
        m_isOver = true;
        return true;
    }
    return false;
}
//清空
void FiveInLine::clear()
{
    //默认AI颜色空(不开AI)
    //setCpuColor(None);
    //清空赢法棋子个数统计
    for(int i = 0; i< m_vecWin.size() ;++i)
    {
        m_vecWin[i].clear();
    }
    //状态位置
    m_isOver = true;
    m_blackOrWhite = Black;
    m_moveFlag = false;
    //界面
    for(int x = 0 ; x < FIL_COLS ; ++x)
    {
        for(int y = 0 ; y < FIL_ROWS ;++y)
        {
            m_board[x][y] = None;
        }
    }
    //清空胜利方
    ui->lb_winner->setText("");
    ui->lb_color->setText("黑方回合");

    ui->lb_timer->hide();
}

//设置状态函数
void FiveInLine::setSelfStatus(int _status)
{
    m_status = _status;
}

//落子槽函数：鼠标释放捕捉落子位置(颜色 位置)
void FiveInLine::slot_pieceDown(int blackorwhite , int x ,int y)
{
    //槽函数 更新数组，切换回合
    //判断是否是同一个回合
    if(blackorwhite != getBlackOrWhite())return;
    //落子 没有子才能落
    if(m_board[x][y] == None)
    {
        //更新数组
        m_board[x][y] = blackorwhite;
        m_lastPos=QPoint(x,y);
        //下棋的过程保存
        m_everyStepPos.push_back({x,y});
        //更新棋子颜色后判断输赢
        if(isWin(x,y))
        {
            QString str = getBlackOrWhite() == Black?"黑方胜":"白方胜";
            //胜利方
            ui->lb_winner->setText(str);

            //结束回合定时
            m_countTimer.stop();
            //隐藏
            ui->lb_timer->hide();

            //发送信号
            Q_EMIT SIG_playerWin(getBlackOrWhite());
            //QMessageBox::about(this ,"提示",str);
            //修改1
            //clear();
        }
        else
        {
            //判断不可以是电脑回合
            if(m_cpuColor != getBlackOrWhite())
            {
                //更新 玩家 每种赢法 玩家棋子个数
                for(int i = 0 ; i < m_vecWin.size() ; i++)
                {
                    if(m_vecWin[i].board[x][y] == 1)
                    {
                        m_vecWin[i].playerCount += 1;
                        m_vecWin[i].cpuCount = 100;     //电脑可行得位置+1 从100开始加 100以上称为无效值
                    }
                }
            }
            else
            {
                //更新 电脑 每种赢法 玩家棋子个数统计个数更新
                for(int k = 0 ; k < m_vecWin.size() ; ++k)
                {
                    if(m_vecWin[k].board[x][y] == 1)
                    {
                        m_vecWin[k].cpuCount += 1;
                        m_vecWin[k].playerCount = 100;
                    }
                }
            }
            //回合时间恢复为默认值
            m_colorCount = DEFAULT_COUNTER;
            //切换回合
            changeBlackAndWhite();
            //qDebug() << "AI颜色:" << m_cpuColor << "当前回合:" << getBlackOrWhite();
            //电脑下棋：判断是否是电脑回合
            if(m_cpuColor == getBlackOrWhite())
            {
#ifndef DEF_ALPHA_BETA
                pieceDownByCpu();
#else
                pieceDownByBetterCpu();
#endif
            }
        }
    }
}

//开始游戏
void FiveInLine::slot_startGame()
{
    clear();
    //setCpuColor(White);
    m_isOver = false;
    m_colorCount=DEFAULT_COUNTER;
    //显示
    ui->lb_timer->show();
    //定时器使用
    m_countTimer.start(1000);
}

//------------------AIAIAIAI---------------------------------------------------
//AIAIAIAI-------------------------------------------------------------------
//初始化电脑AI所有赢法 棋子布局
void FiveInLine::InitAiVector()
{
    //横
    for(int y = 0 ; y < FIL_ROWS ; y++)
    {
        for(int x = 0 ; x < FIL_COLS - 4 ;x++)//起点位置
        {
            stru_win w;
            for(int k = 0 ;k < 5 ;k++)
            {
                //(x+k) y
                w.board[x+k][y] = 1;
            }
            m_vecWin.push_back(w);
        }
    }

    //竖
    for(int x = 0 ; x < FIL_COLS ; x++)
    {
        for(int y = 0 ; y <  FIL_ROWS - 4 ; y++)//起点位置
        {
            stru_win w;
            for(int k = 0 ;k < 5 ;k++)
            {
                //(x) y+k
                w.board[x][y+k] = 1;
            }
            m_vecWin.push_back(w);
        }
    }
    //左
    for(int x = FIL_COLS - 1 ; x >= 4 ; x--)
    {
        for(int y = 0 ; y <  FIL_ROWS - 4 ; y++ )//起点位置
        {
            stru_win w;
            for(int k = 0 ;k < 5 ;k++)
            {
                //(x) y+k
                w.board[x-k][y+k] = 1;
            }
            m_vecWin.push_back(w);
        }
    }

    //右
    for(int x = 0 ; x < FIL_COLS - 4 ; x++)
    {
        for(int y = 0 ; y <  FIL_ROWS - 4 ; y++ )//起点位置
        {
            stru_win w;
            for(int k = 0 ;k < 5 ;k++)
            {
                //(x) y+k
                w.board[x + k][y + k] = 1;
            }
            m_vecWin.push_back(w);
        }
    }

}
//AI电脑落子：根据每种赢法 棋子个数 给没有子的位置评分 得到胜算最大的坐标
void FiveInLine::pieceDownByCpu()
{
    if(m_isOver) return;

    //是这个回合再下
    if(m_cpuColor != getBlackOrWhite()) return;


    int x=0,y=0,k=0;
//------------------------------------------------------------------------------------
    //方法1
    //目标坐标
    int u = 0;
    int v = 0;
    int max = 0;//最优位置
//----------------------------------------------------------------------------------
    int MyScore[FIL_COLS][FIL_ROWS] ={};    //玩家分数
    int CpuScore[FIL_COLS][FIL_ROWS] ={};   //电脑分数
//--------------------------------------------------------------------------------------
    //方法2
    int a = 0;
    int b = 0;
    int maxCpu = 0;
//-----------------------------------------------------------------------------------------
    //估分：找没有子的点，看每种赢法个数，对应棋子个数，进行对点的分数计算
    for(x = 0 ; x < FIL_COLS ;++x)
    {
        for( y = 0 ; y < FIL_ROWS ; ++y)
        {
            if(m_board[x][y] == None)
            {
                //遍历所有赢法
                for(k = 0 ; k < m_vecWin.size() ; ++k)
                {
                    //判断点有没有
                    //评估玩家
                    if(m_vecWin[k].board[x][y] == 1)
                    {
                        //根据该赢法棋子个数
                        switch(m_vecWin[k].playerCount)
                        {
                        case 1:
                            MyScore[x][y] += 200;
                            break;
                        case 2:
                            MyScore[x][y] += 400;
                            break;
                        case 3:
                            MyScore[x][y] += 2000;
                            break;
                        case 4:
                            MyScore[x][y] += 10000;
                            break;
                        }
                    }
                    //评估电脑
                    if(m_vecWin[k].board[x][y] == 1)
                    {
                        //根据该赢法棋子个数
                        switch(m_vecWin[k].cpuCount)
                        {
                        case 1:
                            CpuScore[x][y] += 220;
                            break;
                        case 2:
                            CpuScore[x][y] += 420;
                            break;
                        case 3:
                            CpuScore[x][y] += 4100;
                            break;
                        case 4:
                            CpuScore[x][y] += 20000;
                            break;
                        }
                    }
                }
            }
        }
    }
//-------------------------------------------------------------------------------------------------
//    //方法1
//    //找最优值：无子的点位
//    for(x = 0 ; x < FIL_COLS; ++x)
//    {
//        for( y = 0 ; y < FIL_ROWS ; ++y)
//        {
//            if(m_board[x][y] == None)
//            {
//                //玩家 防守方
//                if(MyScore[x][y] > max)
//                {
//                    max = MyScore[x][y];
//                    u = x;
//                    v = y;
//                }
//                else if(MyScore[x][y] == max)
//                {
//                    if(CpuScore[x][y] > CpuScore[u][v])
//                    {
//                        u = x;
//                        v = y;
//                    }
//                }
//                //电脑 进攻方
//                if(CpuScore[x][y] > max)
//                {
//                    max = CpuScore[x][y];
//                    u = x;
//                    v = y;
//                }
//                else if(CpuScore[x][y] == max)
//                {
//                    if(MyScore[x][y] > MyScore[u][v])
//                    {
//                        u = x;
//                        v = y;
//                    }
//                }
//            }
//        }
//    }
//----------------------------------------------------------------------------------
    //方法2
    //找最优值：无子的点位
    for(x = 0 ; x < FIL_COLS; ++x)
    {
        for( y = 0 ; y < FIL_ROWS ; ++y)
        {
            if(m_board[x][y] == None)
            {
                //玩家 防守方
                if(MyScore[x][y] > max)
                {
                    max = MyScore[x][y];
                    u = x;
                    v = y;
                }
                if(CpuScore[x][y] > max)
                {
                    maxCpu = CpuScore[x][y];
                    a = x;
                    b = y;
                }

            }
        }
    }
    if(max < maxCpu)
    {
        u = a;
        v = b;
    }
//-------------------------------------------------------------------------------
    //落子

    //得到最优目标坐标

    Q_EMIT SIG_pieceDown(getBlackOrWhite(),u,v);
}
//电脑回合颜色
void FiveInLine::setCpuColor(int newCpuColor)
{
    m_cpuColor = newCpuColor;
}
//不使用现在的棋盘 使用临时棋盘
bool FiveInLine::isWin(int x, int y, vector<vector<int> > &board)
{
    //看四条直线 8个方向同色棋子个数 有一个到5就结束
    int count = 1;
    //循环 4条
    //左
    for(int dr = d_z ; dr < d_count ; dr += 2)
    {
        for(int i = 1; i<=4 ;++i)
        {
            //获取偏移后棋子坐标
            int ix = dx[dr]*i + x;
            int iy = dy[dr]*i + y;
            //判断是否出界
            if(isCrossLie(ix,iy))
            {
                break;
            }
            //是否同色
            if(board[ix][iy] == board[x][y])
            {
                count++;
            }
            else
            {
                break;
            }
        }
        //右
        for(int i = 1; i<=4 ;++i)
        {
            //获取偏移后棋子坐标
            int ix = dx[dr + 1]*i + x;
            int iy = dy[dr + 1]*i + y;
            //判断是否出界
            if(isCrossLie(ix,iy))
            {
                break;
            }
            //是否同色
            if(board[ix][iy] == board[x][y])
            {
                count++;
            }
            else
            {
                break;
            }
        }
        if(count >= 5)//5个不在看其他直线
        {
            break;
        }
        else    //不够看其他直线
        {
            count = 1;
        }
    }
    if(count >= 5)
    {
        return true;
    }
    return false;
}

void FiveInLine::pieceDownByBetterCpu()
{
    //qDebug()<<__func__;
    if(m_isOver) return;
    int bestX=-1;int bestY=-1;
    findBestMove(bestX,bestY,getBlackOrWhite(),MAX_DEPTH);
    if(bestX!=-1&&bestY!=-1)
    {
        emit SIG_pieceDown(getBlackOrWhite(),bestX,bestY);
    }

}

void FiveInLine::findBestMove(int &bestX, int &bestY, int player, int depth)
{
    //一上来就是max层
    int BestValue=INT_MIN;
    bestX=-1;
    bestY=-1;
    vector<pair<int,int>> candidates;
    vector<pair<int,int>> &copyEveryStep=m_everyStepPos;
    vector<vector<int>> &copyBoard=m_board;
    //获取要看的点
    getNeedHandlePos(copyEveryStep,candidates,copyBoard);
    /*sort(candidates.begin(),candidates.end(),[&](pair<int,int>&a,pair<int,int>&b)
    {
        copyBoard[a.first][a.second]=player;
        int scoreA=evaluateBoard(copyBoard);
        copyBoard[a.first][a.second]=None;

        copyBoard[b.first][b.second]=player;
        int scoreB=evaluateBoard(copyBoard);
        copyBoard[b.first][b.second]=None;
        return scoreA>scoreB;
    });*/
    int count=candidates.size();
    m_taskCount =count;
    for(int i=0;i<count;++i)
    {
        switch (i%8) {
        case 0:emit SIG_getBetterScore0(candidates[i].first,candidates[i].second,player);
            break;
        case 1:emit SIG_getBetterScore1(candidates[i].first,candidates[i].second,player);
            break;
        case 2:emit SIG_getBetterScore2(candidates[i].first,candidates[i].second,player);
            break;
        case 3:emit SIG_getBetterScore3(candidates[i].first,candidates[i].second,player);
            break;
        case 4:emit SIG_getBetterScore4(candidates[i].first,candidates[i].second,player);
            break;
        case 5:emit SIG_getBetterScore5(candidates[i].first,candidates[i].second,player);
            break;
        case 6:emit SIG_getBetterScore6(candidates[i].first,candidates[i].second,player);
            break;
        case 7:emit SIG_getBetterScore7(candidates[i].first,candidates[i].second,player);
            break;
        }
    }
    /*for(auto& pos:candidates)
    {
        int x=pos.first;
        int y=pos.second;
        //尝试在该位置下子 如果能直接获得胜利 那就直接返回
        copyBoard[x][y]=player;
        if(isWin(x,y,copyBoard))
        {
            bestX=x;
            bestY=y;
            return;
        }
        //尝试在该位置 敌人瞎子 如果能直接获得胜利 那么也返回
        copyBoard[x][y]=(player ==Black)?White:Black;
        if(isWin(x,y,copyBoard))
        {
            bestX=x;
            bestY=y;
            return;
        }
        //切换回来 开始搜索
        copyBoard[x][y]=player;
        copyEveryStep.push_back({x,y});
        //使用极大极小值搜索 配合剪枝
        int moveValue=minmax(copyBoard,copyEveryStep,depth-1,INT_MIN,INT_MAX,false,player);
        //dfs撤销该点落子
        copyBoard[x][y]=None;
        copyEveryStep.pop_back();

        if(moveValue>BestValue)
        {
            BestValue=moveValue;
            bestX=x;
            bestY=y;
        }
    }*/
}

void FiveInLine::getNeedHandlePos(vector<pair<int, int> > &copyEveryStep,
                                  vector<pair<int, int> > &candidates,
                                  vector<vector<int> > &board)
{
    //根据历史下棋位置 找附近一个位置 也就是3*3的位置 去重新添加到数组中 最好的方式是最近下的先添加
    //去重采取的方式 是使用unordered_set
    unordered_set<int> unique;
    //使用反向迭代器
    for(auto ite=copyEveryStep.rbegin();ite!=copyEveryStep.rend();++ite)
    {
        auto & pos=*ite;
        int x=pos.first;
        int y=pos.second;
        for(int i=-1;i<=1;++i)
        {
            for(int j=-1;j<=1;++j)
            {
                if(i==0&&j==0)continue;
                int nx=x+i;
                int ny=y+j;
                if(nx<0||nx>=FIL_COLS||ny<0||ny>=FIL_ROWS) continue;
                if(board[nx][ny]!=None) continue;
                int key=nx*100+ny;
                if(unique.count(key)==0)
                {
                    unique.insert(key);
                    candidates.push_back({nx,ny});
                }
            }
        }
    }
}

int FiveInLine::minmax(vector<vector<int> > &copyBoard, vector<pair<int, int> > &copyEveryStep,
                       int depth, int alpha, int beta, bool isMaximizing, int player)
{
    //其实本质上是dfs 只是max层 取子节点的max值 min层取子节点的min值
    //分别实现ai最大化自己的得分 以及玩家最小化ai得分
    //一会实现剪枝
    //递归终点评估棋盘
    if(depth==0)
    {

        return evaluateBoard(copyBoard);
    }
    int bestValue=isMaximizing?INT_MIN:INT_MAX;//max层取最小 便于更新
    int opponent=(player==Black)?White:Black;
    vector<pair<int,int>> candidates;
    getNeedHandlePos(copyEveryStep,candidates,copyBoard);
    if(candidates.empty()) return 0;
    if(isMaximizing)
    {
        sort(candidates.begin(),candidates.end(),[&](pair<int,int>&a,pair<int,int>&b)
        {
            copyBoard[a.first][a.second]=player;
            int scoreA=evaluateBoard(copyBoard);
            copyBoard[a.first][a.second]=None;

            copyBoard[b.first][b.second]=player;
            int scoreB=evaluateBoard(copyBoard);
            copyBoard[b.first][b.second]=None;
            return scoreA>scoreB;
        });
    }else
    {
        sort(candidates.begin(),candidates.end(),[&](pair<int,int>&a,pair<int,int>&b)
        {
            copyBoard[a.first][a.second]=player;
            int scoreA=evaluateBoard(Black,copyBoard);
            copyBoard[a.first][a.second]=None;

            copyBoard[b.first][b.second]=player;
            int scoreB=evaluateBoard(Black,copyBoard);
            copyBoard[b.first][b.second]=None;
            return scoreA>scoreB;
        });
    }


    for(auto& pos:candidates)
    {
        int x=pos.first;
        int y=pos.second;
        //逻辑和findBestMove类似
        //尝试落子
        copyBoard[x][y]=isMaximizing?player:opponent;
        copyEveryStep.push_back({x,y});
        //检查是否获胜
        bool win=isWin(x,y,copyBoard);
        if(win)
        {
            copyBoard[x][y]=None;
            copyEveryStep.pop_back();
            return isMaximizing?1000000:-1000000;
        }
            int value=minmax(copyBoard,copyEveryStep,depth-1,alpha,beta,!isMaximizing,player);
            //撤销
            copyBoard[x][y]=None;
            copyEveryStep.pop_back();
            //更新最佳
            if(isMaximizing)
            {
                if(value>bestValue)
                {
                    bestValue=value;
                }
                //α剪枝
                if(bestValue>alpha)
                {
                    alpha=bestValue;
                    if(alpha>=beta)break;
                }
            }else
            {
                if(value<bestValue)
                {
                    bestValue=value;
                }
                //β剪枝
                if(bestValue<beta)
                {
                    beta=bestValue;
                    if(beta<=alpha)break;
                }
            }
    }
    return bestValue;
}

int FiveInLine::evaluateBoard(int color, vector<vector<int> > &board)
{
    int values=0;
    //遍历棋盘所有位置 不是想要的颜色跳过
    for(int x=0;x<FIL_COLS;++x)
    {
        for(int y=0;y<FIL_ROWS;++y)
        {
            if(board[x][y]!=color)
            {
                continue;
            }
            //处理八个方向
            int j=0;
            while(j<directions.size())
            {
                int count=1;
                vector<int> record;
                for(int a=0;a<2;a++)
                {
                    int curX=x;
                    int curY=y;
                    for(int i=0;i<4;++i)//查看延伸的两个方向ge2四个点 ye就是看九个点
                    {
                        if(curX+directions[j].first<0||curX+directions[j].first>=FIL_COLS
                           ||curY+directions[j].second<0||curY+directions[j].second>=FIL_ROWS)
                        {
                            record.push_back(3-color);//认为超出边界也是对方堵死
                            break;
                        }
                        //移动到下一位置
                        curX=curX+directions[j].first;
                        curY=curY+directions[j].second;
                        if(board[curX][curY]==color)
                        {
                            count++;
                        }else{
                            record.push_back(board[curX][curY]);
                            break;
                        }
                    }
                     j++;
                }
               //看完一组的个数 可以看是棋子得分
               //得分表：
               //五连200000 活四10000 死四1000 活三900 死三100 活二90 死二10
                if(count>=5)
                {
                    values+=200000;
                }
                else if(count==4)
                {
                    if(record[0]==0&&record[1]==0)
                    {
                        values+=10000;
                    }else if((record[0]==0 && record[1]==3-color)||
                             (record[1]==0 && record[0]==3-color))
                    {
                        values+=1000;
                    }
                }
                else if(count==3)
                {
                    if(record[0]==0&&record[1]==0)
                    {
                        values+=900;
                    }else if((record[0]==0 && record[1]==3-color)||
                             (record[1]==0 && record[0]==3-color))
                    {
                        values+=100;
                    }
                }
                else if(count==2)
                {
                    if(record[0]==0&&record[1]==0)
                    {
                        values+=90;
                    }else if((record[0]==0 && record[1]==3-color)||
                             (record[1]==0 && record[0]==3-color))
                    {
                        values+=10;
                    }
                }
            }
        }
    }
    return values;
}
//获得后选点
//---------------------------回合定时器---------------------------------------------------
//回合定时器：每秒更新一次
void FiveInLine::slot_countTimer()
{

    m_colorCount--;
    if(m_colorCount <= 0)
    {
        //界面文本更新
        ui->lb_timer->setText(QString("%1s").arg(m_colorCount,2,10,QChar('0')));
        changeBlackAndWhite();
        m_colorCount = DEFAULT_COUNTER;
        return;
    }
    //界面文本更新
    ui->lb_timer->setText(QString("%1s").arg(m_colorCount,2,10,QChar('0')));
}

int FiveInLine::evaluateBoard(vector<vector<int> > &board)
{
    //先计算棋盘hash
    //查看是否有 有就直接返回
    //没有 计算并添加到缓存
    //先查看是否要淘汰 如果是就先淘汰
    string hash=getBoardHash(board);
    if(evaluateCache.count(hash)>0)
    {
        return evaluateCache[hash];
    }
    int white=evaluateBoard(White,board);
    int black=evaluateBoard(Black,board);
    //这里多线程访问 因为访问共享的资源 所以与线程安全问题 需要加锁
    {
        lock_guard<mutex> lck(m_mtx);
        if(CacheQueue.size()>10000)
        {
            evaluateCache.erase(CacheQueue.front());//根据key值删除，队列中添加的是字符串key值
            CacheQueue.pop_front();
        }
        evaluateCache[hash]=white-black;
        CacheQueue.push_back(hash);
    }

    return white-black;
}

string FiveInLine::getBoardHash(vector<vector<int> > &board)
{
    string hash;
    for(int i=0;i<FIL_COLS;++i)
    {
        for(int j=0;j<FIL_ROWS;++j)
        {
            hash+=to_string(board[i][j]);
        }
    }
    return hash;
}

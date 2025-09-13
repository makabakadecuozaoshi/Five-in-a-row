#include "myworker.h"
#include"fiveinline.h"
MyWorker::MyWorker(FiveInLine *_fil)
{
    m_fil=_fil;
    m_thread=new QThread;
    m_thread->start();
    this->moveToThread(m_thread);

}

MyWorker::~MyWorker()
{
    m_thread->quit();
    m_thread->wait();
    if(m_thread->isRunning())
    {
        m_thread->terminate();
        m_thread->wait();
    }
    m_thread->deleteLater();
}

void MyWorker::slot_getBetterScore(int x, int y, int player)
{

    int BestValue=INT_MIN;
    int bestX=-1;
    int bestY=-1;

    vector<pair<int,int>> copyEveryStep=m_fil->m_everyStepPos;
    vector<vector<int>> copyBoard=m_fil->m_board;
    //尝试在该位置下子 如果能直接获得胜利 那就直接返回
    copyBoard[x][y]=player;
    if(m_fil->isWin(x,y,copyBoard))
    {
        BestValue=200000;
        bestX=x;
        bestY=y;
        emit SIG_getScoreFinish(bestX,bestY,BestValue);
        return;
    }
    //尝试在该位置 敌人瞎子 如果能直接获得胜利 那么也返回
    copyBoard[x][y]=(player == FiveInLine::Black)?FiveInLine::White:FiveInLine::Black;
    if(m_fil->isWin(x,y,copyBoard))
    {
        bestX=x;
        bestY=y;
        BestValue=100000;
        emit SIG_getScoreFinish(bestX,bestY,BestValue);
        return;
    }
    //切换回来 开始搜索
    copyBoard[x][y]=player;
    copyEveryStep.push_back({x,y});
    //使用极大极小值搜索 配合剪枝
    int moveValue=m_fil->minmax(copyBoard,copyEveryStep,m_fil->MAX_DEPTH-1,INT_MIN,INT_MAX,false,player);
    //dfs撤销该点落子
    copyBoard[x][y]=FiveInLine::None;
    copyEveryStep.pop_back();

    if(moveValue>BestValue)
    {
        BestValue=moveValue;
        bestX=x;
        bestY=y;
    }
    emit SIG_getScoreFinish(bestX,bestY,BestValue);
}

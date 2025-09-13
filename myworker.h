#ifndef MYWORKER_H
#define MYWORKER_H

#include <QObject>
#include <QThread>
class FiveInLine;
class MyWorker : public QObject
{
    Q_OBJECT
signals:
    void SIG_getScoreFinish(int x,int y,int score);
public:
    MyWorker(FiveInLine* _fil);
    ~MyWorker();
private:
    FiveInLine * m_fil;//因为是要用五子棋类的方法 所以要加成员
    QThread *m_thread;
public slots:
    void slot_getBetterScore(int x,int y,int player);
};

#endif // MYWORKER_H

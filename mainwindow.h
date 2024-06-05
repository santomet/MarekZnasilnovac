#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "requestsmodel.h"
#include <QtNetwork/QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


protected slots:
    void getReady();

    void start();
    void burst();
    void stopEverything();

    void checkTime();

private slots:
    void on_plusButton_clicked();

    void on_minusButton_clicked();

    void on_readyPushButton_clicked();

    void on_pushButton_clicked();

private:
    enum State {
        Resting,
        Ready,
        Bursting
    };

    State mState{Resting};

    Ui::MainWindow *ui;

    //timing
    QTimer burstTimer;
    QTimer startTimer;
    QTime startTime;
    QTime stopTime;

    //network
    QNetworkAccessManager mNetwork;
    QNetworkCookieJar mCookieJar;
    QList<QNetworkRequest> mNetworkRequests;

    //model
    RequestsModel mModel;
};

#endif // MAINWINDOW_H

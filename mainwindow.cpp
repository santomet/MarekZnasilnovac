#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Marek the Raper");

    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setModel(&mModel);

    connect(&startTimer, &QTimer::timeout, this, &MainWindow::start);
    connect(&burstTimer, &QTimer::timeout, this, &MainWindow::burst);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getReady()
{
    ui->statusLabel->setText("STATUS: getting ready");

    mNetworkRequests.clear();
    QNetworkCookieJar *cookieJar = new QNetworkCookieJar(this);

    QNetworkCookie iscreds("iscreds", QByteArray::fromBase64(ui->iscredsLineEdit->text().toUtf8().toBase64()));
    QNetworkCookie issession("issession", QByteArray::fromBase64(ui->issessionLineEdit->text().toUtf8().toBase64()));

    iscreds.setDomain("is.muni.cz");
    issession.setDomain("is.muni.cz");

    cookieJar->insertCookie(iscreds);
    cookieJar->insertCookie(issession);

    mNetwork.setCookieJar(cookieJar);

    burstTimer.setInterval(ui->askFrequencyMs->value());

    for(QString url : mModel.getRequests())
    {
        QNetworkRequest req(url);

        if(!ui->userAgentEdit->text().isEmpty())
            req.setRawHeader("User-Agent", ui->userAgentEdit->text().toUtf8());

        mNetworkRequests.append(req);
    }

    startTime = ui->timeEdit->time();

    stopTime = startTime;
    stopTime.addMSecs(ui->lastAfterMs->value());
    //TODO need to use returned value
    startTime.addMSecs(-(ui->earlyStartMs->value()));

    startTimer.setInterval(1);
    startTimer.start();

    ui->statusLabel->setText("STATUS: Waiting!!");
    ui->readyPushButton->setText("UNLOAD!");
    mState = Ready;
}

void MainWindow::start()
{
    if(QTime::currentTime() >= startTime)
    {
        startTimer.stop();
        burstTimer.start();
        ui->statusLabel->setText("STATUS: BURSTING!");
        ui->readyPushButton->setText("STOP!");
        mState = Bursting;
    }
}

void MainWindow::burst()
{
    for(QNetworkRequest req : mNetworkRequests)
    {
        mNetwork.get(req);
    }

    if(QTime::currentTime() >= stopTime)
    {
        stopEverything();
    }
}

void MainWindow::stopEverything()
{
    burstTimer.stop();
    startTimer.stop();
    ui->statusLabel->setText("STATUS: resting");
    ui->readyPushButton->setText("LOAD!");
    mState = Resting;
}

void MainWindow::on_plusButton_clicked()
{
    mModel.insertRow(mModel.rowCount());
}

void MainWindow::on_minusButton_clicked()
{
    QModelIndexList slist = ui->tableView->selectionModel()->selectedRows();
    if(slist.isEmpty())
        return;
    mModel.removeRow(slist.at(0).row());
}

void MainWindow::on_readyPushButton_clicked()
{
    switch(mState)
    {
    case Resting :
        getReady();
        break;
    case Ready :
    case Bursting :
        stopEverything();
        break;
    }
}

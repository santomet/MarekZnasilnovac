#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

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

    QNetworkCookie iscreds("__Host-iscreds", QByteArray::fromBase64(ui->iscredsLineEdit->text().toUtf8().toBase64()));
    QNetworkCookie issession("__Host-issession", QByteArray::fromBase64(ui->issessionLineEdit->text().toUtf8().toBase64()));

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
    stopTime = startTime.addMSecs(ui->lastAfterMs->value());
    //TODO need to use returned value
    startTime = startTime.addMSecs(-(ui->earlyStartMs->value()));

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

void MainWindow::checkTime()
{
    QUdpSocket udpSocket;
    QByteArray packet(48, 0);
    packet[0] = 0x1B; //client mode

    udpSocket.connectToHost("time.fi.muni.cz", 123);

    QElapsedTimer elt;

    QDateTime local;

    if (udpSocket.waitForConnected()) {
        local = QDateTime::currentDateTime();
        elt.start();
        udpSocket.write(packet);
        if (udpSocket.waitForReadyRead(3000)) { // wait for response up to 3 seconds
            int msPing = elt.elapsed();
            QByteArray ntpResponse = udpSocket.readAll();

            if (ntpResponse.size() == 48) {
                QDataStream dataStream(ntpResponse);
                dataStream.setByteOrder(QDataStream::BigEndian);

                quint32 seconds;
                quint32 fraction;
                dataStream.skipRawData(40);
                dataStream >> seconds >> fraction;

                quint64 milliseconds = (seconds - 2208988800UL) * 1000 + (static_cast<quint64>(fraction) * 1000) / 0x100000000ULL;

                // Convert to QDateTime
                QDateTime currentTime = QDateTime::fromMSecsSinceEpoch(milliseconds, Qt::UTC).toLocalTime().addMSecs(- msPing/2);
                int diff = local.msecsTo(currentTime);
                qDebug() << "NTP Time:" << currentTime.toString() << "Local time" << local << "Difference" << diff << msPing;
                QString message;
                message.append("Results of time test:\ntime.fi.muni.cz:\t");
                message.append(currentTime.toString());
                message.append("\nLocal time:\t\t");
                message.append(local.toString());
                message.append("\nFI minus Local ms\t");
                message.append(QString::number(diff));
                message.append("\nPing ms:\t\t");
                message.append(QString::number(msPing));
                QMessageBox::information(this, "Time Test", message);
            }
        } else {
            qWarning() << "No response from NTP server.";
        }
    } else {
        qWarning() << "Could not connect to NTP server.";
    }
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

void MainWindow::on_pushButton_clicked()
{
    checkTime();
}


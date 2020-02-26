#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QDateTime>
#include <QTextStream>

#define APP_VERSION tr("Correcter v0.1 by 菟裘")

#define HALF_INTERVAL 2.5
#define MAX_HS 2000.0

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(800,480);
//    ui->tw_lrc->setColumnWidth(3, 10000);
    ui->hs_A->setMaximum(MAX_HS);
    ui->hs_B->setMaximum(MAX_HS);
    ui->hs_A->setValue(MAX_HS/2);
    ui->hs_B->setValue(MAX_HS/2);

//    ui->tw_lrc->setSelectionBehavior ( QAbstractItemView::SelectRows);
//    ui->tw_lrc->setSelectionMode(QAbstractItemView::SingleSelection);
//    ui->tw_lrc->horizontalHeader()->setStretchLastSection(true);

    settings = new QSettings("settings.ini", QSettings::IniFormat, this);
    this->move(settings->value("pos").toPoint());
    this->resize(settings->value("size").toSize());
    this->file_name = settings->value("file name").toString();

    this->player = new QMediaPlayer(this);
    connect(this->player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadTXT(QString txtName)
{
    QFile txtFile(txtName);
    if(!txtFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this, tr("Warning"), tr("Cannot open TXT file,\nChoose it yourself!"));
        return;
    }
    QString txtContent = txtFile.readAll();
    QStringList txtLine = txtContent.split(QRegExp("[\r\n]"));

    ui->tw_lrc->setRowCount(0);
    for(int i=0;i<txtLine.count();i++){
        QString tmp = txtLine.at(i).trimmed();
        if (tmp.isEmpty())continue;

        QTableWidgetItem *check = new QTableWidgetItem();
        check->setCheckState(Qt::Checked);

        ui->tw_lrc->insertRow(i);
        ui->tw_lrc->setItem(i,0, check);
        ui->tw_lrc->setItem(i,1,new QTableWidgetItem("[\t]"));
        ui->tw_lrc->setItem(i,2,new QTableWidgetItem("[\t]"));
        ui->tw_lrc->setItem(i,3,new QTableWidgetItem(tmp));
    }
    ui->tw_lrc->resizeColumnsToContents();
}

void MainWindow::saveToLrc()
{
    QString content = tr("");
    int rows = ui->tw_lrc->rowCount();
    if (rows <= 0) return;

    QString lrcPath = file_name;
    QString saveFilePath = QFileDialog::getSaveFileName(this, "Save Lrc File", lrcPath.replace(".mp3", ".lrc"), "Lrc File (*.lrc)");
    QFile file(saveFilePath);
//    file.resize(0);
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this, "Warning", "Cannot save file!");
        return;
    }
    QTextStream stream(&file);
    stream.seek(0);

    for(int i=0;i<rows;i++){
        if (ui->tw_lrc->item(i,0)->checkState() == Qt::Checked){
            int timeA = ui->tw_lrc->item(i,1)->text().toInt();
            int prevB = timeA;
            if (i > 0){
                prevB = ui->tw_lrc->item(i-1,2)->text().toInt();
            }
            timeA = (timeA+prevB)/2;
            content.append(tr("[%1:%2.%3]%4\r\n")
                           .arg(timeA/(60*1000),2,10,QLatin1Char('0'))
                           .arg((timeA/1000)%60,2,10,QLatin1Char('0'))
                           .arg((timeA/10)%100,2,10,QLatin1Char('0'))
                           .arg(ui->tw_lrc->item(i,3)->text()));
        }
    }
    stream << content;
    file.close();
    log(tr("Save LRC file successfully!"), "blue");
}

void MainWindow::log(QString str, QString color)
{
    QString cur_time = QDateTime::currentDateTime().time().toString();
    ui->textEdit->insertHtml(tr("<font color=\"%2\">[%3] %1</font><p>").arg(str).arg(color).arg(cur_time));
    ui->textEdit->moveCursor(QTextCursor::End);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    settings->setValue("file name", QVariant(this->file_name));
    settings->setValue("pos", QVariant(this->pos()).toPoint());
    settings->setValue("size", QVariant(this->size()).toSize());
}


void MainWindow::on_btn_loadmp3_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Load MP3"), file_name, tr("MP3 File (*.mp3)"));
    if(filePath.isEmpty())return;

    this->player->setMedia(QUrl::fromLocalFile(filePath));
    this->player->setVolume(50);

    file_name = filePath;

    QFile file(filePath);
    QFileInfo info(file);
    file_name_no_extension = info.fileName();
    this->setWindowTitle(file_name_no_extension + "  -  " + APP_VERSION);

    QString txtName =  filePath.replace(".mp3", ".txt");
    loadTXT(txtName);
}

void MainWindow::on_positionChanged(qint64 pos)
{
    if (player->duration()!=ui->hs_progress->maximum()){
        ui->hs_progress->setMaximum(player->duration());
        this->media_total_secs = player->duration()/1000;
    }
    ui->hs_progress->setValue(pos);

    int escaped_secs = pos/1000;
    ui->lbl_lrc->setText(tr("%1:%2/%3:%4 ---- %5/%6")
                         .arg(escaped_secs/60,2)
                         .arg(escaped_secs%60,2)
                         .arg(media_total_secs/60,2)
                         .arg(media_total_secs%60,2)
                         .arg(pos)
                         .arg(player->duration()));

    if (A_B_status){
        if (pos < pos_A || pos > pos_B){
            player->setPosition(pos_A);
            player->play();
        }
    }
}

void MainWindow::on_hs_progress_sliderMoved(int position)
{
    player->setPosition(position);
}

void MainWindow::on_btn_play_clicked()
{
    if (!is_playing) {
        player->play();
        is_playing = true;
        ui->btn_play->setText(tr("Pause"));
    }else {
        player->pause();
        is_playing = false;
        ui->btn_play->setText(tr("Play"));
    }
}

void MainWindow::on_btn_opentxt_clicked()
{
    QString txtName = QFileDialog::getOpenFileName(this, tr("Open TXT File"), file_name, tr("TXT File (*.txt)"));
    loadTXT(txtName);
}

void MainWindow::on_btn_markA_clicked()
{
    this->pos_A = player->position();
    this->original_pos_A = this->pos_A;
    A_B_status = false;
    log(tr("Point A: %1.").arg(this->pos_A), "green");
}

void MainWindow::on_btn_markB_clicked()
{
    this->pos_B = player->position();
    this->original_pos_B = this->pos_B;
    A_B_status = true;
    log(tr("Point B: %1.").arg(this->pos_B), "green");
}


void MainWindow::on_hs_A_sliderMoved(int position)
{
    qint64 tmp = this->original_pos_A + (position / MAX_HS - 0.5) * HALF_INTERVAL * 1000;
    this->pos_A = tmp > 0 ? tmp : 0;
}

void MainWindow::on_hs_B_sliderMoved(int position)
{
    qint64 tmp = this->original_pos_B + (position / MAX_HS - 0.5) * HALF_INTERVAL * 1000;
    qint64 maxt = player->duration();
    this->pos_B = tmp < maxt ? tmp : maxt;
}

void MainWindow::on_btn_mark_clicked()
{
    int cur_row = ui->tw_lrc->currentRow();
    QString timeA = tr("%1").arg(this->pos_A);
    QString timeB = tr("%1").arg(this->pos_B);
    ui->tw_lrc->setItem(cur_row,1, new QTableWidgetItem(timeA));
    ui->tw_lrc->setItem(cur_row, 2, new QTableWidgetItem(timeB));

//    this->pos_A = this->pos_B;
//    this->pos_B = player->duration();

    ui->hs_A->setValue(MAX_HS/2);
    ui->hs_B->setValue(MAX_HS/2);
}

void MainWindow::on_btn_save_clicked()
{
    saveToLrc();
}

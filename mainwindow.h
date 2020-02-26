#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia/QMediaPlayer>
#include <QCloseEvent>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadTXT(QString txtName);

    void saveToLrc();

    void log(QString str, QString color);

protected:
    void closeEvent(QCloseEvent *) override;
private slots:
    void on_btn_loadmp3_clicked();

    void on_positionChanged(qint64);
    void on_hs_progress_sliderMoved(int position);

    void on_btn_play_clicked();

    void on_btn_opentxt_clicked();

    void on_btn_markA_clicked();

    void on_btn_markB_clicked();

    void on_hs_A_sliderMoved(int position);

    void on_hs_B_sliderMoved(int position);

    void on_btn_mark_clicked();

    void on_btn_save_clicked();

private:
    Ui::MainWindow *ui;

    QMediaPlayer *player = nullptr;
    int media_total_secs = 0;
    bool is_playing = false;
    QString file_name;
    QString file_name_no_extension;
    QSettings *settings;

    qint64 pos_A, pos_B, original_pos_A, original_pos_B;
    bool A_B_status = false;
};
#endif // MAINWINDOW_H

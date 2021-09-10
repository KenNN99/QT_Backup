#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QFileInfoList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void CopyFile(QFileInfoList file_list, QString dir_name);

private slots:
    void on_OpenDir_clicked();

    void on_backupButton_clicked();

private:
    QString backup_dir_name="./Backup_Dir";
    Ui::MainWindow *ui;
    QGridLayout* scoll_pointer=NULL;
    QFileInfoList file_info;
    QString Dir_name;
};
#endif // MAINWINDOW_H

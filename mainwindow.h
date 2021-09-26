#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <CFileSystemModel.h>
#include <QDebug>
#include <vector>
#include "operation.h"
#include <QDateTime>
#include "overwritedialog.h"

#define DATA_FILE_NAME "/.b_data.txt"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef enum enum_operate_type
{
    _append=0,
    overwrite
}backup_operate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString getBackupDir();
    QString getRawDir();

signals:
    void sendOverwriteList(QString);

private slots:
    void on_pushButton_clicked();

    void on_OpenDir_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_backupButton_clicked();

    void get_overwrite_dialog_signal(bool flag);

private:
    Ui::MainWindow *ui;
    CFileSystemModel* model = new CFileSystemModel;
    bool raw_selected = false;
    bool backup_selected = false;
    QString backup_dir_name;
    QString raw_dir_name;
    backup_operate current_operate = _append;

    QStringList overwrite_file_list;
    overWriteDialog* o_dialog;

    int refreshRawCheckbox();
    int checkAppendBackup(std::vector<DIR_LIST_STRUCT>& dir_list_vector);
};

#endif // MAINWINDOW_H

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
#include "appenddialog.h"
#include "update_a_dialog.h"
#include "overwrte_r_dialog.h"
#include "copydonedialog.h"
#include <QProcess>
#include <QFileSystemWatcher>
#include "overwriterecovery.h"

#define DATA_FILE_NAME "/.b_data.txt"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef enum enum_operate_type
{
    _append=0,
    overwrite
}backup_operate;

typedef enum enum_r_operate_type
{
    _update=0,
    r_overwrite
}recovery_operate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString getBackupDir();
    QString getRawDir();

signals:
    void sendOverwritePath(QString);
    void sendROverwritePath(QString);
    void sendOverwriteList(QStringList);
    void sendROverwriteList(QStringList);
    void sendWOverwritePath(QString);

private slots:
    void on_pushButton_clicked();

    void on_OpenDir_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_backupButton_clicked();

    void get_overwrite_dialog_signal(bool flag);

    void get_append_dialog_signal(bool flag);

    void get_r_overwrite_signal(bool flag);

    void get_r_t_overwrite_signal(bool flag);

    void on_pushButton_2_clicked();

    void on_OpenDir_2_clicked();

    void on_backupButton_2_clicked();

    void on_comboBox_2_currentTextChanged(const QString &arg1);

    void on_tabWidget_currentChanged(int index);

    void get_w_overwrite_dialog_signal(bool flag);

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_OpenDir_3_clicked();

    void on_file_changed(const QString &file);

    void on_directory_changed(const QString &file);

private:
    Ui::MainWindow *ui;
    CFileSystemModel* model = new CFileSystemModel;
    CFileSystemModel* model1 = new CFileSystemModel;
    bool raw_selected = false;
    bool backup_selected = false;
    bool recovery_selected = false;
    QString backup_dir_name;
    QString raw_dir_name;
    QString recovery_dir_name;
    backup_operate current_operate = _append;
    recovery_operate current_r_operate = _update;

    QString target_dir_name;
    QString watch_dir_name;
    bool target_selected = false;
    bool watch_selected = false;
    bool watching = false;

    QStringList overwrite_file_list;
    QStringList r_overwrite_file_list;
    overWriteDialog* o_dialog;
    overwriteRecovery* o_w_dialog;
    AppendDialog* a_dialog;
    Update_a_Dialog* u_dialog;
    overwrte_R_Dialog* o_r_dialog;
    CopyDoneDialog* c_d_dialog;
    QFileSystemWatcher* watcher;

    std::vector<DIR_LIST_STRUCT> fresh_backup_info;
    std::vector<DIR_LIST_STRUCT> recovery_info;
    std::vector<DIR_LIST_STRUCT> watched_info;

    int refreshRawCheckbox();
    int checkAppendBackup(std::vector<DIR_LIST_STRUCT>& dir_list_vector);
    int buildFreshList(QFileInfoList file_info_list);
    int checkDirinFreshList(QFileInfo checking);
    int checkFileinFreshList(QFileInfo checking, int index);
    int appendCopyFile();
    int makeDFile();
    int refreshRecoveryCheckbox();
    int checkRecovery();
    int sortRecoveryInfo();
    int recoveryCopyFile();
    int overwriteRecovery_f();
    int sortOverwriteInfo();
    int makeTFile();
};

#endif // MAINWINDOW_H

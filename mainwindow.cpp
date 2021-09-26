#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDirModel>
#include <QFileDialog>
#include <QFile>
#include <QStack>
#include "operation.h"
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    o_dialog = new overWriteDialog;
    connect(this,SIGNAL(sendOverwriteList(QString)),o_dialog,SLOT(get_file_list(QString)));
    connect(o_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_overwrite_dialog_signal(bool)));
    ui->tab1_label->setText("Please select raw directory and backup directory");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked() //backup_selected
{
    backup_dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"),"/home/ken/backup_dir");
    ui->lineEdit_2->setText(backup_dir_name);
    backup_selected = true;
    if(raw_selected == false)
        ui->tab1_label->setText("Please select raw directory");
    else
        ui->tab1_label->setText("Please check the file you want to backup");

    if(raw_selected && backup_selected)
    {
        ui->treeView->setModel(model);
        ui->treeView->setRootIndex(model->index(raw_dir_name));
        ui->treeView->header()->resizeSection(0,400);

        refreshRawCheckbox();
    }
}

void MainWindow::on_OpenDir_clicked()
{
    raw_dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"),"/home/ken/Backup");
    ui->dir_display->setText(raw_dir_name);
    raw_selected = true;
    if(backup_dir_name == false)
        ui->tab1_label->setText("Please select backup directory");
    else
        ui->tab1_label->setText("Please check the file you want to backup");

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(raw_dir_name));
    ui->treeView->header()->resizeSection(0,400);

    if(raw_selected && backup_selected)
        refreshRawCheckbox();
}

QString MainWindow::getRawDir()
{
    return raw_dir_name;
}

QString MainWindow::getBackupDir()
{
    return backup_dir_name;
}

int MainWindow::refreshRawCheckbox()
{
    QFile data_file(backup_dir_name + DATA_FILE_NAME);
    if(!data_file.exists())//rm all file
    {
        model->setData(model->index(raw_dir_name),Qt::Checked,Qt::CheckStateRole);
        return 0;
    }
    if(!data_file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug()<<"Data file open fail";
        return -1;
    }
    std::vector<DIR_LIST_STRUCT> dir_list;
    if(check_src_file_info(backup_dir_name,data_file,dir_list)!=0)
    {
        return -1;
    }
    checkAppendBackup(dir_list);
    return 0;
}

int MainWindow::checkAppendBackup(std::vector<DIR_LIST_STRUCT>& dir_list_vector)
{
    model->setData(model->index(raw_dir_name),Qt::Checked,Qt::CheckStateRole);
    foreach (auto dir_iter, dir_list_vector)
    {
        qDebug()<<raw_dir_name + dir_iter.dir_name;
        if(!model->index(raw_dir_name + dir_iter.dir_name).isValid())
        {
            //foreach(auto c_iter, )
            continue;
        }
        qDebug()<<model->index(raw_dir_name + dir_iter.dir_name);
        foreach (auto file_iter, dir_iter.info_list)
        {
            if(file_iter.name != dir_iter.dir_name)
            {
                if(model->index(raw_dir_name + dir_iter.dir_name + file_iter.name).isValid())
                {
                    QFileInfo checking_file(raw_dir_name + dir_iter.dir_name + file_iter.name);

                    uint ttime = checking_file.lastModified().toTime_t();
                    uint stime = file_iter.m_time.toTime_t();
                    if(
                            checking_file.groupId()==file_iter.gid &&
                            checking_file.ownerId()==file_iter.uid &&
                            check_file_type(checking_file,file_iter.type) &&
                            (int)checking_file.permissions()==file_iter.permission &&
                            ttime<=stime
                            )
                            model->setData(model->index(raw_dir_name + dir_iter.dir_name + file_iter.name),Qt::Unchecked,Qt::CheckStateRole);
                    else
                        overwrite_file_list.push_back(backup_dir_name + dir_iter.dir_name + file_iter.name);
                }
            }
        }
    }
    return 0;
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "Add File")
        current_operate = _append;
    else
        current_operate = overwrite;

    qDebug()<<current_operate;
    return;
}

void MainWindow::on_backupButton_clicked()
{
    if(!(backup_selected && raw_selected))
    {
        ui->tab1_label->setText("Please select raw directory and backup directory");
        return;
    }
    if(current_operate == _append)
        qDebug()<<"_append";
    else
    {
        emit sendOverwriteList(backup_dir_name);
        o_dialog->show();
        qDebug()<<overwrite_file_list;
    }
}

void MainWindow::get_overwrite_dialog_signal(bool flag)
{
    if(flag)
    {
        overwriteBackup(backup_dir_name,raw_dir_name);
    }
    return;
}

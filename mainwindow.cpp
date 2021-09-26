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
    a_dialog = new AppendDialog;
    connect(this,SIGNAL(sendOverwritePath(QString)),o_dialog,SLOT(get_file_list(QString)));
    connect(this,SIGNAL(sendOverwriteList(QStringList)),a_dialog,SLOT(get_file_list(QStringList)));
    connect(o_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_overwrite_dialog_signal(bool)));
    connect(a_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_append_dialog_signal(bool)));
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
    if(!data_file.exists())
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
    fresh_backup_info = dir_list;
    return 0;
}

int MainWindow::checkAppendBackup(std::vector<DIR_LIST_STRUCT>& dir_list_vector)
{
    model->setData(model->index(raw_dir_name),Qt::Checked,Qt::CheckStateRole);
    foreach (auto dir_iter, dir_list_vector)
    {
        if(!model->index(raw_dir_name + dir_iter.dir_name).isValid())
        {
            continue;
        }
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
                            )//replace
                            model->setData(model->index(raw_dir_name + dir_iter.dir_name + file_iter.name),Qt::Unchecked,Qt::CheckStateRole);
                    else
                    {
                        overwrite_file_list.push_back(backup_dir_name + dir_iter.dir_name + file_iter.name);
                        continue;
                    }
                }

            }
        }
    }
    return 0;
}

int MainWindow::buildFreshList(QFileInfoList file_info_list)
{
    QFileInfoList::iterator iter=file_info_list.begin();
    while(iter->fileName()!=".")
        iter++;
    int i = checkDirinFreshList(*iter);
    if(i>=0)//already had
    {
        fresh_backup_info.at(i).need_copy = false;
        foreach(auto file_info_iter, file_info_list)
        {
            if(file_info_iter.fileName()!="." && file_info_iter.fileName()!="..")
            {
                auto index = model->index(file_info_iter.absoluteFilePath());
                if(model->data(index,Qt::CheckStateRole)==Qt::Checked)
                {
                    int d=checkFileinFreshList(file_info_iter,i);
                    if(d <= 0)//new file
                    {
                        BACKUP_INFO_STRUCT temp_backup_info;
                        temp_backup_info.name = file_info_iter.fileName();
                        temp_backup_info.gid = file_info_iter.groupId();
                        temp_backup_info.uid = file_info_iter.ownerId();
                        int c = 1;
                        if(file_info_iter.isDir())
                            c=0;
                        else if(file_info_iter.isFile())
                            c=1;
                        else if(file_info_iter.isSymLink())
                            c=2;
                        temp_backup_info.type = (FILE_TYPE_ENUM)c;
                        temp_backup_info.permission = file_info_iter.permissions();
                        temp_backup_info.m_time = file_info_iter.lastModified();
                        temp_backup_info.need_copy = true;
                        fresh_backup_info.at(i).info_list.push_back(temp_backup_info);
                        fresh_backup_info.at(i).need_copy = true;
                    }
                    else //replace
                    {
                        fresh_backup_info.at(i).info_list.at(d).name = file_info_iter.fileName();
                        fresh_backup_info.at(i).info_list.at(d).gid = file_info_iter.groupId();
                        fresh_backup_info.at(i).info_list.at(d).uid = file_info_iter.ownerId();
                        int c = 1;
                        if(file_info_iter.isDir())
                            c=0;
                        else if(file_info_iter.isFile())
                            c=1;
                        else if(file_info_iter.isSymLink())
                            c=2;
                        fresh_backup_info.at(i).info_list.at(d).type = (FILE_TYPE_ENUM)c;
                        fresh_backup_info.at(i).info_list.at(d).permission = file_info_iter.permissions();
                        fresh_backup_info.at(i).info_list.at(d).m_time = file_info_iter.lastModified();
                        fresh_backup_info.at(i).info_list.at(d).need_copy = true;
                        fresh_backup_info.at(i).need_copy = true;
                    }
                }
                if(file_info_iter.isDir())
                {
                    QDir d(file_info_iter.absoluteFilePath());
                    QFileInfoList sub_info_list = d.entryInfoList();
                    buildFreshList(sub_info_list);
                }
            }
        }
    }
    else
    {
        DIR_LIST_STRUCT temp_dir_list;
        temp_dir_list.dir_name = iter->absoluteFilePath().mid(raw_dir_name.length())+"/";
        foreach(auto file_info_iter, file_info_list)
        {
            if(file_info_iter.fileName()!="." && file_info_iter.fileName()!="..")
            {
                auto index = model->index(file_info_iter.absoluteFilePath());
                if(model->data(index,Qt::CheckStateRole)==Qt::Checked)
                {
                    BACKUP_INFO_STRUCT temp_backup_info;
                    temp_backup_info.name = file_info_iter.fileName();
                    temp_backup_info.gid = file_info_iter.groupId();
                    temp_backup_info.uid = file_info_iter.ownerId();
                    int c = 1;
                    if(file_info_iter.isDir())
                        c=0;
                    else if(file_info_iter.isFile())
                        c=1;
                    else if(file_info_iter.isSymLink())
                        c=2;
                    temp_backup_info.type = (FILE_TYPE_ENUM)c;
                    temp_backup_info.permission = file_info_iter.permissions();
                    temp_backup_info.m_time = file_info_iter.lastModified();
                    temp_backup_info.need_copy = true;
                    temp_dir_list.info_list.push_back(temp_backup_info);
                }
                if(file_info_iter.isDir())
                {
                    QDir d(file_info_iter.absoluteFilePath());
                    QFileInfoList sub_info_list = d.entryInfoList();
                    buildFreshList(sub_info_list);
                }
            }
        }
        temp_dir_list.need_copy=true;
        fresh_backup_info.push_back(temp_dir_list);
    }
    return 0;
}

int MainWindow::checkFileinFreshList(QFileInfo checking, int index)
{
    int i=0;
    foreach(auto iter, fresh_backup_info.at(index).info_list)
    {
        if(checking.fileName()==iter.name)
        {
            return i;
        }
        i++;
    }
    return -1;
}

int MainWindow::appendCopyFile()
{
    QDir raw_d(raw_dir_name);
    auto raw_file_info_list = raw_d.entryInfoList();
    foreach(auto dir_iter, fresh_backup_info)
    {
        if(!dir_iter.need_copy)
        {
            continue;
        }
        QString dir_path = backup_dir_name+dir_iter.dir_name;
        QDir d(dir_path);
        qDebug()<<backup_dir_name+dir_iter.dir_name;
        if(!d.exists())
        {
            bool flag = d.mkdir(dir_path);
            qDebug()<<"Make backup diretory \""<<flag<<"\" ";
        }
        foreach(auto file_iter, dir_iter.info_list)
        {
            if(file_iter.type == DIR || !file_iter.need_copy)
                continue;
            QString file_raw_path = raw_dir_name+dir_iter.dir_name+file_iter.name;
            QString file_backup_path = backup_dir_name+dir_iter.dir_name+file_iter.name;
                QProcess q;
                if(file_iter.type == Sym_link)
                {
                    q.execute("cp -d \"" + file_raw_path + "\" \"" + file_backup_path + "\"");
                    if(q.exitCode()!=0)
                        qDebug()<<"cp fail";
                }
                else if(file_iter.type == C_FILE)
                {
                    q.execute("cp \"" + file_raw_path + "\" \"" + file_backup_path + "\"");
                    if(q.exitCode()!=0)
                        qDebug()<<"cp fail";
                }
                q.close();
        }
    }
    return 0;
}

int MainWindow::checkDirinFreshList(QFileInfo checking)
{
    bool flag = false;
    int i=0;
    QString dir=checking.absoluteFilePath().mid(raw_dir_name.length())+"/";
    foreach(auto iter, fresh_backup_info)
    {
        if(iter.dir_name == dir)
        {
            flag = true;
            break;
        }
        i++;
    }
    if(flag == true)
        return i;
    return -1;
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
    {
        qDebug()<<"_append";
        QDir d(raw_dir_name);
        QFileInfoList file_info_list = d.entryInfoList();
        buildFreshList(file_info_list);
        emit sendOverwriteList(overwrite_file_list);
        a_dialog->show();
    }
    else
    {
        emit sendOverwritePath(backup_dir_name);
        o_dialog->show();
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

void MainWindow::get_append_dialog_signal(bool flag)
{
    if(flag)
    {
        if(makeDFile()!=0)
        {
            qDebug()<<"Make log file fail.";
            return;
        }
        appendCopyFile();
    }
    return;
}

int MainWindow::makeDFile()
{
    QFile data_file(backup_dir_name + "/.b_data.txt");
    if(!data_file.open(QIODevice::Truncate|QIODevice::ReadWrite))
    {
        qDebug()<<"Open fail";
        return -1;
    }

    foreach(auto dir_iter, fresh_backup_info)
    {
        std::vector<BACKUP_INFO_STRUCT>::iterator file_iter;
        for(file_iter = dir_iter.info_list.begin(); file_iter != dir_iter.info_list.end();file_iter++)
        {
            QString text;
            text = "\n";
            if(file_iter==dir_iter.info_list.begin())
            {
                text += dir_iter.dir_name;
            }
            else
            {
                text += "\"";
                text += file_iter->name;
                text += "\"";
            }
            if(dir_iter.info_list.begin() == file_iter)
            text += " ";
            text += QString::number(file_iter->gid);
            text += " ";
            text += QString::number(file_iter->uid);
            text += " ";
            text += QString::number(file_iter->type);
            text += " ";
            text += QString::number(file_iter->permission);
            text += file_iter->m_time.toString("yyyyMMddhhmm.ss");
            QTextStream in(&data_file);
            in<<text;
        }
    }
    return 0;
}

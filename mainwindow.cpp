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
    u_dialog = new Update_a_Dialog;
    o_r_dialog = new overwrte_R_Dialog;
    c_d_dialog = new CopyDoneDialog;
    o_w_dialog = new overwriteRecovery;
    watcher = new QFileSystemWatcher;
    connect(this,SIGNAL(sendOverwritePath(QString)),o_dialog,SLOT(get_file_list(QString)));
    connect(this,SIGNAL(sendROverwriteList(QStringList)),u_dialog,SLOT(get_file_list(QStringList)));
    connect(this,SIGNAL(sendOverwriteList(QStringList)),a_dialog,SLOT(get_file_list(QStringList)));
    connect(this,SIGNAL(sendROverwritePath(QString)),o_r_dialog,SLOT(get_file_list(QString)));
    connect(this,SIGNAL(sendWOverwritePath(QString)),o_w_dialog,SLOT(get_file_list(QString)));
    connect(o_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_overwrite_dialog_signal(bool)));
    connect(a_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_append_dialog_signal(bool)));
    connect(u_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_r_overwrite_signal(bool)));
    connect(o_r_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_r_t_overwrite_signal(bool)));
    connect(o_w_dialog,SIGNAL(confirmInfo(bool)),this,SLOT(get_w_overwrite_dialog_signal(bool)));
    connect(watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(on_file_changed(const QString &)));
    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(on_directory_changed(const QString &)));


    ui->tab1_label->setText("Please select raw directory and backup directory");
    ui->tab1_label_2->setText("Please select recovery directory and backup directory");
    ui->label_2->setText("Please select target directory and watched directory");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked() //backup_selected
{
    backup_dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"),"/home/ken/backup_dir");
    ui->lineEdit_2->setText(backup_dir_name);
    ui->lineEdit_3->setText(backup_dir_name);
    backup_selected = true;
    if(raw_selected == false)
        ui->tab1_label->setText("Please select raw directory");
    else
        ui->tab1_label->setText("Please check the file you want to backup");

    if(recovery_selected == false)
        ui->tab1_label_2->setText("Please select recovery directory");
    else
        ui->tab1_label_2->setText("Please check the file you want to recovery");

    ui->treeView_2->setModel(model1);
    ui->treeView_2->setRootIndex(model1->index(backup_dir_name));
    ui->treeView_2->header()->resizeSection(0,400);

    if(raw_selected && backup_selected)
    {
        ui->treeView->setModel(model);
        ui->treeView->setRootIndex(model->index(raw_dir_name));
        ui->treeView->header()->resizeSection(0,400);

        refreshRawCheckbox();
    }
    if(recovery_selected && backup_selected)
    {
        refreshRecoveryCheckbox();
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
                    qDebug()<<checking_file.groupId()<<" "<<file_iter.gid;
                    qDebug()<<checking_file.ownerId()<<" "<<file_iter.uid;
                    qDebug()<<checking_file.permissions()<<" "<<file_iter.permission;
                    qDebug()<<ttime<<" "<<stime;
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
                        overwrite_file_list.push_back(fresh_backup_info.at(i).dir_name + file_info_iter.fileName());
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
        BACKUP_INFO_STRUCT temp_dir_info;
        temp_dir_info.name = iter->absoluteFilePath().mid(raw_dir_name.length())+"/";
        temp_dir_info.gid = iter->groupId();
        temp_dir_info.uid = iter->ownerId();
        int c = 1;
        if(iter->isDir())
            c=0;
        else if(iter->isFile())
            c=1;
        else if(iter->isSymLink())
            c=2;
        temp_dir_info.type = (FILE_TYPE_ENUM)c;
        temp_dir_info.permission = iter->permissions();
        temp_dir_info.m_time = iter->lastModified();
        temp_dir_info.need_copy = true;
        temp_dir_list.info_list.push_back(temp_dir_info);
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
                    temp_dir_list.need_copy = true;
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
            bool flag = d.mkpath(dir_path);
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
        overwrite_file_list.clear();
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
        c_d_dialog->show();
        if(raw_selected && backup_selected)
        {
            refreshRawCheckbox();
        }
        if(recovery_selected && backup_selected)
        {
            refreshRecoveryCheckbox();
        }
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
        overwrite_file_list.clear();
        c_d_dialog->show();
        if(raw_selected && backup_selected)
        {
            refreshRawCheckbox();
        }
        if(recovery_selected && backup_selected)
        {
            refreshRecoveryCheckbox();
        }
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
            text += " ";
            text += QString::number(file_iter->gid);
            text += " ";
            text += QString::number(file_iter->uid);
            text += " ";
            text += QString::number(file_iter->type);
            text += " ";
            text += QString::number(file_iter->permission);
            text += " ";
            text += file_iter->m_time.toString("yyyyMMddhhmm.ss");
            QTextStream in(&data_file);
            in<<text;
        }
    }
    return 0;
}

int MainWindow::makeTFile()
{
    QFile data_file(target_dir_name + "/.b_data.txt");
    if(!data_file.open(QIODevice::Truncate|QIODevice::ReadWrite))
    {
        qDebug()<<"Open fail";
        return -1;
    }

    foreach(auto dir_iter, watched_info)
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
            text += " ";
            text += QString::number(file_iter->gid);
            text += " ";
            text += QString::number(file_iter->uid);
            text += " ";
            text += QString::number(file_iter->type);
            text += " ";
            text += QString::number(file_iter->permission);
            text += " ";
            text += file_iter->m_time.toString("yyyyMMddhhmm.ss");
            QTextStream in(&data_file);
            in<<text;
        }
    }
    return 0;
}

void MainWindow::on_pushButton_2_clicked()
{
    backup_dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"),"/home/ken/backup_dir");
    ui->lineEdit_2->setText(backup_dir_name);
    ui->lineEdit_3->setText(backup_dir_name);
    backup_selected = true;
    if(raw_selected == false)
        ui->tab1_label->setText("Please select raw directory");
    else
        ui->tab1_label->setText("Please check the file you want to backup");

    if(recovery_selected == false)
        ui->tab1_label_2->setText("Please select recovery directory");
    else
        ui->tab1_label_2->setText("Please check the file you want to recovery");

    ui->treeView_2->setModel(model1);
    ui->treeView_2->setRootIndex(model1->index(backup_dir_name));
    ui->treeView_2->header()->resizeSection(0,400);

    if(raw_selected && backup_selected)
    {
        ui->treeView->setModel(model);
        ui->treeView->setRootIndex(model->index(raw_dir_name));
        ui->treeView->header()->resizeSection(0,400);

        refreshRawCheckbox();
    }
    if(recovery_selected && backup_selected)
    {
        refreshRecoveryCheckbox();
    }
}

void MainWindow::on_OpenDir_2_clicked()
{
    recovery_dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"),"/home/ken/backup_test");
    ui->dir_display_2->setText(recovery_dir_name);
    recovery_selected = true;
    if(backup_dir_name == false)
        ui->tab1_label_2->setText("Please select backup directory");
    else
        ui->tab1_label_2->setText("Please check the file you want to recovery");

    if(recovery_selected && backup_selected)
        refreshRecoveryCheckbox();
}

int MainWindow::refreshRecoveryCheckbox()
{
    QFile data_file(backup_dir_name + DATA_FILE_NAME);
    if(!data_file.exists())
    {
        qDebug()<<"Data file Missing!";
        return -1;
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
    recovery_info=dir_list;
    checkRecovery();
    return 0;
}

int MainWindow::checkRecovery()
{
    model1->setData(model1->index(backup_dir_name),Qt::Unchecked,Qt::CheckStateRole);
    foreach(auto dir_iter, recovery_info)
    {
        QDir target(recovery_dir_name + dir_iter.dir_name.left(dir_iter.dir_name.length()-1));
        QStringList target_name_list = target.entryList();
        dir_iter.need_copy = false;
        foreach(auto info_iter, dir_iter.info_list)
        {
            if(info_iter.type == DIR)
                continue;
            if(target_name_list.indexOf(info_iter.name)<0)
            {
                model1->setData(model1->index(backup_dir_name + dir_iter.dir_name + info_iter.name),Qt::Checked,Qt::CheckStateRole);
            }
            else
            {
                QFileInfo checking_file(recovery_dir_name + dir_iter.dir_name + info_iter.name);

                uint ttime = checking_file.lastModified().toTime_t();
                uint stime = info_iter.m_time.toTime_t();

                qDebug()<<checking_file.groupId()<<" "<<info_iter.gid;
                qDebug()<<checking_file.ownerId()<<" "<<info_iter.uid;
                qDebug()<<(int)checking_file.permissions()<<" "<<info_iter.permission;
                qDebug()<<check_file_type(checking_file,info_iter.type);
                qDebug()<<checking_file.lastModified()<<" "<<info_iter.m_time;
                qDebug()<<ttime<<" "<<stime;
                if(
                        !(checking_file.groupId()==info_iter.gid &&
                        checking_file.ownerId()==info_iter.uid &&
                        check_file_type(checking_file,info_iter.type) &&
                        (int)checking_file.permissions()==info_iter.permission &&
                        ttime>=stime)
                        )
                {
                    qDebug()<<backup_dir_name + dir_iter.dir_name + info_iter.name;
                    model1->setData(model1->index(backup_dir_name + dir_iter.dir_name + info_iter.name),Qt::Checked,Qt::CheckStateRole);
                }
            }
        }
    }
    return 0;
}

int MainWindow::sortRecoveryInfo()
{
    r_overwrite_file_list.clear();

    for(std::vector<DIR_LIST_STRUCT>::iterator dir_iter = recovery_info.begin(); dir_iter != recovery_info.end(); dir_iter++)
    {
        dir_iter->need_copy = false;
        for(std::vector<BACKUP_INFO_STRUCT>::iterator file_iter = dir_iter->info_list.begin(); file_iter != dir_iter->info_list.end(); file_iter++)
        {
            if(file_iter->type == DIR)
                continue;
            QString temp_s = backup_dir_name + dir_iter->dir_name + file_iter->name;
            if(model1->data(model1->index(temp_s),Qt::CheckStateRole) == Qt::Checked)
            {
                file_iter->need_copy = true;
                dir_iter->need_copy = true;
                QString temp_r = recovery_dir_name + dir_iter->dir_name + file_iter->name;
                QFile d_r(temp_r);
                if(d_r.exists())
                    r_overwrite_file_list.push_back(dir_iter->dir_name + file_iter->name);
            }
            else
            {
                file_iter->need_copy = false;
            }
        }
    }
    return 0;
}

int MainWindow::sortOverwriteInfo()
{
    for(std::vector<DIR_LIST_STRUCT>::iterator dir_iter = recovery_info.begin(); dir_iter != recovery_info.end(); dir_iter++)
    {
        dir_iter->need_copy = true;
        for(std::vector<BACKUP_INFO_STRUCT>::iterator file_iter = dir_iter->info_list.begin(); file_iter != dir_iter->info_list.end(); file_iter++)
        {
            file_iter->need_copy = true;
        }
    }
    return 0;
}

int MainWindow::recoveryCopyFile()
{
    QStack<BACKUP_INFO_STRUCT> dir_info_stack;
    foreach(auto dir_iter, recovery_info)
    {
        if(!dir_iter.need_copy)
        {
            continue;
        }
        QString dir_path = recovery_dir_name+dir_iter.dir_name;
        QDir d(dir_path);
        if(!d.exists())
        {
            bool flag = d.mkpath(dir_path);
            qDebug()<<"Make recovery diretory \""<<flag<<"\" ";
        }
        dir_info_stack.push(*dir_iter.info_list.begin());
        foreach(auto file_iter, dir_iter.info_list)
        {
            if(file_iter.type == DIR || !file_iter.need_copy)
                continue;
            QString file_recovery_path = recovery_dir_name+dir_iter.dir_name+file_iter.name;
            QString file_backup_path = backup_dir_name+dir_iter.dir_name+file_iter.name;
            QProcess q;
            if(file_iter.type == Sym_link)
            {
                q.execute("cp -d \"" + file_backup_path + "\" \"" + file_recovery_path + "\"");
                if(q.exitCode()!=0)
                    qDebug()<<"cp fail";
                q.execute("chgrp " + QString::number(file_iter.gid) + " \"" +  recovery_dir_name + dir_iter.dir_name + file_iter.name + "\"");
                if(q.exitCode()!=0)
                {
                    qDebug()<<"chgrp fail";
                    return -1;
                }
                q.execute("chown " + QString::number(file_iter.uid) + " \"" +  recovery_dir_name + dir_iter.dir_name + file_iter.name + "\"");
                if(q.exitCode()!=0)
                {
                    qDebug()<<"chown fail";
                    return -1;
                }
                q.execute("touch -t " + file_iter.m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  recovery_dir_name + dir_iter.dir_name + file_iter.name + "\"");
                if(q.exitCode()!=0)
                {
                    qDebug()<<"touch fail";
                    return -1;
                }
            }
            else if(file_iter.type == C_FILE)
            {
                q.execute("cp \"" + file_backup_path + "\" \"" + file_recovery_path + "\"");
                if(q.exitCode()!=0)
                    qDebug()<<"cp fail";
                q.execute("chgrp " + QString::number(file_iter.gid) + " \"" +  recovery_dir_name + dir_iter.dir_name + file_iter.name + "\"");
                if(q.exitCode()!=0)
                {
                    qDebug()<<"chgrp fail";
                    return -1;
                }
                q.execute("chown " + QString::number(file_iter.uid) + " \"" +  recovery_dir_name + dir_iter.dir_name + file_iter.name + "\"");
                if(q.exitCode()!=0)
                {
                    qDebug()<<"chown fail";
                    return -1;
                }
                q.execute("touch -t " + file_iter.m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  recovery_dir_name + dir_iter.dir_name + file_iter.name + "\"");
                if(q.exitCode()!=0)
                {
                    qDebug()<<"touch fail";
                    return -1;
                }
            }
            q.close();
        }
    }
    QProcess q;
    BACKUP_INFO_STRUCT temp_struct;
    while(!dir_info_stack.empty())
    {
        temp_struct=dir_info_stack.top();
        if(temp_struct.name=="/")
        {
            dir_info_stack.pop();
            continue;
        }
        q.execute("chgrp " + QString::number(temp_struct.gid) + " \"" +  recovery_dir_name + temp_struct.name);
        if(q.exitCode()!=0)
        {
            qDebug()<<"chgrp fail";
            return -1;
        }
        q.execute("chown " + QString::number(temp_struct.uid) + " \"" +  recovery_dir_name + temp_struct.name);
        if(q.exitCode()!=0)
        {
            qDebug()<<"chown fail";
            return -1;
        }
        q.execute("touch -t " + temp_struct.m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  recovery_dir_name + temp_struct.name);
        if(q.exitCode()!=0)
        {
            qDebug()<<"touch fail";
            return -1;
        }
        dir_info_stack.pop();
    }
    return 0;
}

void MainWindow::on_backupButton_2_clicked()
{
    if(!(backup_selected && recovery_selected))
    {
        ui->tab1_label->setText("Please select recovery directory and backup directory");
        return;
    }
    qDebug()<<current_r_operate;
    if(current_r_operate == _update)
    {
        sortRecoveryInfo();
        emit sendROverwriteList(r_overwrite_file_list);
        u_dialog->show();
    }
    else
    {
        qDebug()<<"r_overwrite";
        sortOverwriteInfo();
        emit sendROverwritePath(recovery_dir_name);
        o_r_dialog->show();
    }
    return;
}

void MainWindow::get_r_overwrite_signal(bool flag)
{
    if(flag)
    {
        recoveryCopyFile();
        qDebug()<<"recoveryCopyFile Done";
        c_d_dialog->show();
        if(raw_selected && backup_selected)
        {
            refreshRawCheckbox();
        }
        if(recovery_selected && backup_selected)
        {
            refreshRecoveryCheckbox();
        }
    }
    return;
}

int MainWindow::overwriteRecovery_f()
{
    QDir r(recovery_dir_name);
    QFileInfoList b_info = r.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
    foreach(auto b_iter, b_info)
    {
        if(rmAllFile(b_iter)!=0)
        {
            qDebug()<<"Clear backup directory fail";
            return -1;
        }
    }
    recoveryCopyFile();
    return 0;
}

void MainWindow::get_r_t_overwrite_signal(bool flag)
{
    if(flag)
    {
        overwriteRecovery_f();
        c_d_dialog->show();
        if(raw_selected && backup_selected)
        {
            refreshRawCheckbox();
        }
        if(recovery_selected && backup_selected)
        {
            refreshRecoveryCheckbox();
        }
    }
    return;
}

void MainWindow::on_comboBox_2_currentTextChanged(const QString &arg1)
{
    qDebug()<<arg1;
    if(arg1 == "Update")
    {
        qDebug()<<"UPDATE";
        current_r_operate = _update;
    }
    else
        current_r_operate = r_overwrite;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 2)
    {
        if(target_selected && watch_selected)
        {
            emit sendOverwritePath(target_dir_name);
            o_w_dialog->show();
        }
    }
    else
    {
        watching = false;
    }
}

void MainWindow::get_w_overwrite_dialog_signal(bool flag)
{
    if(flag)
    {
        overwriteBackup(target_dir_name,watch_dir_name);

        QFile data_file(target_dir_name + DATA_FILE_NAME);
        if(!data_file.exists())
        {
            qDebug()<<"Data file missing";
            return;
        }
        if(!data_file.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            qDebug()<<"Data file open fail.";
        }
        if(check_src_file_info(target_dir_name,data_file,watched_info)!=0)
        {
            qDebug()<<"Target dir overwrite fail";
            return;
        }
        QDir w_d(watch_dir_name);
        QFileInfoList c_f_l = w_d.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        QVector<QFileInfoList> path_vector;
        path_vector.push_back(c_f_l);
        int i = 0;
        watcher->addPath(watch_dir_name);
        while(i!=path_vector.size())
        {

            foreach(auto iter, path_vector.at(i))
            {
                if(iter.isDir())
                {
                    QDir t_d(iter.absoluteFilePath());
                    QFileInfoList t_l = t_d.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
                    path_vector.push_back(t_l);
                }
                watcher->addPath(iter.absoluteFilePath());
            }
            i++;
        }
        c_d_dialog->show();
        ui->label->setText("Watching target diretory. Any change will be apply.");
        watching = true;
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    target_dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"),"/home/ken/taget_dir");
    target_selected = true;
    ui->lineEdit_4->setText(target_dir_name);
    if(!watch_selected)
        ui->label_2->setText("Please select watched directory");

    else
    {
        emit sendWOverwritePath(target_dir_name);
        o_w_dialog->show();
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    if(target_selected && watch_selected && !watching)
    {
        emit sendWOverwritePath(target_dir_name);
        o_w_dialog->show();
    }
}

void MainWindow::on_OpenDir_3_clicked()
{
    watcher->removePaths(watcher->files());
    watch_dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"),"/home/ken/watch_dir");
    ui->dir_display_3->setText(watch_dir_name);
    watch_selected = true;
    if(!target_selected)
        ui->label_2->setText("Please select target directory");
    else
    {
        emit sendWOverwritePath(target_dir_name);
        o_w_dialog->show();
    }
}

void MainWindow::on_directory_changed(const QString &file)
{
    /*
    QDir changed(file);
    QString changed_name = file.right(file.length() - watch_dir_name.length()) + "/";
    if(!watching)
        return;
    std::vector<DIR_LIST_STRUCT>::iterator dir_iter = watched_info.begin();
    for (;dir_iter!=watched_info.end();dir_iter++)
    {
        if(dir_iter->dir_name == changed_name)//exist dir
        {
            QStack<QString> new_dir_stack;
            QFileInfoList file_list = changed.entryInfoList(QDir::AllEntries | QDir::NoDotDot);
            foreach(auto info_iter, file_list)
            {
                std::vector<BACKUP_INFO_STRUCT>::iterator current_iter = dir_iter->info_list.begin();
                for (;current_iter!=dir_iter->info_list.end();current_iter++)
                {
                    if(current_iter->name == info_iter.fileName())//fonud
                    {
                       break;
                    }
                }
                if(current_iter == dir_iter->info_list.end())//new file
                {
                    if(!info_iter.isDir())
                    {
                        QProcess q;
                        QString file_target_path = target_dir_name+dir_iter->dir_name+info_iter.fileName();
                        if(info_iter.isSymLink())
                        {
                            q.execute("cp -d \"" + file + "\" \"" + file_target_path + "\"");
                            if(q.exitCode()!=0)
                                qDebug()<<"cp fail";
                        }
                        else if(info_iter.isSymLink())
                        {
                            q.execute("cp \"" + file + "\" \"" + file_target_path + "\"");
                            if(q.exitCode()!=0)
                                qDebug()<<"cp fail";
                        }
                        q.close();
                        BACKUP_INFO_STRUCT temp;
                        temp.name = info_iter.fileName();
                        temp.gid = info_iter.groupId();
                        temp.uid = info_iter.ownerId();
                        int c = 1;
                        if(info_iter.isDir())
                            c=0;
                        else if(info_iter.isFile())
                            c=1;
                        else if(info_iter.isSymLink())
                            c=2;
                        temp.type = (FILE_TYPE_ENUM)c;
                        temp.permission = info_iter.permissions();
                        temp.m_time = info_iter.lastModified();
                        dir_iter->info_list.push_back(temp);
                    }
                    else
                    {
                        BACKUP_INFO_STRUCT temp;
                        temp.name = info_iter.fileName();
                        temp.gid = info_iter.groupId();
                        temp.uid = info_iter.ownerId();
                        int c = 1;
                        if(info_iter.isDir())
                            c=0;
                        else if(info_iter.isFile())
                            c=1;
                        else if(info_iter.isSymLink())
                            c=2;
                        temp.type = (FILE_TYPE_ENUM)c;
                        temp.permission = info_iter.permissions();
                        temp.m_time = info_iter.lastModified();
                        dir_iter->info_list.push_back(temp);
                        QString file_target_path = target_dir_name + dir_iter->dir_name + info_iter.fileName();

                        new_dir_stack.push(file_target_path);
                    }
                    watcher->addPath(file);
                }
            }
            while(!new_dir_stack.empty())
            {
                QString new_file_path = new_dir_stack.top();
                new_dir_stack.pop();

                DIR_LIST_STRUCT temp_dir;
                temp_dir.dir_name = new_file_path.mid(target_dir_name.length()) + "/";
                std::vector<BACKUP_INFO_STRUCT> temp_info_vector;
                BACKUP_INFO_STRUCT temp_info;
                QDir d(new_file_path);
                QFileInfoList file_info_list = d.entryInfoList(QDir::AllEntries | QDir::NoDotDot);
                QFileInfoList::iterator info_iter = file_info_list.begin();
                while(info_iter->fileName() != ".")
                    info_iter++;
                temp_info.name = info_iter->fileName();
                temp_info.gid = info_iter->groupId();
                temp_info.uid = info_iter->ownerId();
                int c = 1;
                if(info_iter->isDir())
                    c=0;
                else if(info_iter->isFile())
                    c=1;
                else if(info_iter->isSymLink())
                    c=2;
                temp_info.type = (FILE_TYPE_ENUM)c;
                temp_info.permission = info_iter->permissions();
                temp_info.m_time = info_iter->lastModified();
                temp_info_vector.push_back(temp_info);
                info_iter = file_info_list.begin();
                for(;info_iter != file_info_list.end();info_iter++)
                {
                    temp_info.name = info_iter->fileName();
                    temp_info.gid = info_iter->groupId();
                    temp_info.uid = info_iter->ownerId();
                    int c = 1;
                    if(info_iter->isDir())
                        c=0;
                    else if(info_iter->isFile())
                        c=1;
                    else if(info_iter->isSymLink())
                        c=2;
                    temp_info.type = (FILE_TYPE_ENUM)c;
                    temp_info.permission = info_iter->permissions();
                    temp_info.m_time = info_iter->lastModified();
                    temp_info_vector.push_back(temp_info);
                    info_iter = file_info_list.begin();
                    if(!info_iter->isDir())
                    {
                        QProcess q;
                        QString file_target_path = target_dir_name + temp_dir.dir_name + info_iter->fileName();
                        if(info_iter->isSymLink())
                        {
                            q.execute("cp -d \"" + info_iter->absoluteFilePath() + "\" \"" + file_target_path + "\"");
                            if(q.exitCode()!=0)
                                qDebug()<<"cp fail";
                        }
                        else if(info_iter->isSymLink())
                        {
                            q.execute("cp \"" + file + "\" \"" + file_target_path + "\"");
                            if(q.exitCode()!=0)
                                qDebug()<<"cp fail";
                        }
                        q.close();
                        watcher->addPath(file_target_path);
                    }
                }
                temp_dir.info_list=temp_info_vector;
                watched_info.push_back(temp_dir);
            }
            break;
        }
    }
    makeTFile();
    */
    if(watching)
        overwriteBackup(target_dir_name,watch_dir_name);
}

void MainWindow::on_file_changed(const QString &file)
{
    /*
    QFile changed(file);
    QString file_name = file.right(file.size()-file.lastIndexOf('/')-1);
    QString file_dir = file.left(file.size()-file_name.size()).mid(watch_dir_name.size());
    if(!watching)
        return;
    if(!changed.exists())//delete condition
    {
        std::vector<DIR_LIST_STRUCT>::iterator dir_iter = watched_info.begin();
        for (;dir_iter!=watched_info.end();dir_iter++)
        {
            if(file_dir == dir_iter->dir_name)
            {
                if(file_name.isEmpty())
                {
                    watched_info.erase(dir_iter);
                    QProcess q;
                    q.execute("rm -R \"" + file + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"touch fail";
                        return;
                    }
                    q.close();
                }
                else
                {
                    std::vector<BACKUP_INFO_STRUCT>::iterator file_iter = dir_iter->info_list.begin();
                    for (;file_iter!=dir_iter->info_list.end();file_iter++)
                    {
                        if(file_iter->name == file_name)
                        {
                            QProcess q;
                            q.execute("rm  \"" + file + "\"");
                            if(q.exitCode()!=0)
                            {
                                qDebug()<<"touch fail";
                                return;
                            }
                            q.close();
                            dir_iter->info_list.erase(file_iter);
                            break;
                        }
                    }
                }
                makeTFile();
                break;
            }
        }
    }
    else
    {
        QFileInfo changed_info(file);
        std::vector<DIR_LIST_STRUCT>::iterator dir_iter = watched_info.begin();
        for (;dir_iter!=watched_info.end();dir_iter++)
        {
            if(file_dir == dir_iter->dir_name)
            {
                if(file_name.isEmpty())
                {
                    dir_iter->info_list.at(0).gid=changed_info.groupId();
                    dir_iter->info_list.at(0).uid=changed_info.ownerId();
                    int c = 1;
                    if(changed_info.isDir())
                        c=0;
                    else if(changed_info.isFile())
                        c=1;
                    else if(changed_info.isSymLink())
                        c=2;
                    dir_iter->info_list.at(0).type = (FILE_TYPE_ENUM)c;
                    dir_iter->info_list.at(0).permission = changed_info.permissions();
                    dir_iter->info_list.at(0).m_time = changed_info.lastModified();
                }
                else
                {
                    std::vector<BACKUP_INFO_STRUCT>::iterator file_iter = dir_iter->info_list.begin();
                    for (;file_iter!=dir_iter->info_list.end();file_iter++)
                    {
                        if(file_iter->name == file_name)
                        {
                            QProcess q;
                            QString file_target_path = target_dir_name+dir_iter->dir_name+file_iter->name;
                            if(file_iter->type == Sym_link)
                            {
                                q.execute("cp -d \"" + file + "\" \"" + file_target_path + "\"");
                                if(q.exitCode()!=0)
                                    qDebug()<<"cp fail";
                            }
                            else if(file_iter->type == C_FILE)
                            {
                                q.execute("cp \"" + file + "\" \"" + file_target_path + "\"");
                                if(q.exitCode()!=0)
                                    qDebug()<<"cp fail";
                            }
                            q.close();
                            file_iter->name = changed_info.fileName();
                            file_iter->gid = changed_info.groupId();
                            file_iter->uid = changed_info.ownerId();
                            int c = 1;
                            if(changed_info.isDir())
                                c=0;
                            else if(changed_info.isFile())
                                c=1;
                            else if(changed_info.isSymLink())
                                c=2;
                            file_iter->type = (FILE_TYPE_ENUM)c;
                            file_iter->permission = changed_info.permissions();
                            file_iter->m_time = changed_info.lastModified();
                            break;
                        }
                    }
                }
                makeTFile();
                break;
            }
        }
    }
    */
}

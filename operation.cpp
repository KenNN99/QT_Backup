#include <QDir>
#include <QApplication>
#include <QFileInfoList>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stack>
#include <QProcess>
#include <sstream>
#include <QStack>
#include "operation.h"

using namespace std;

int readFileInfo(fstream& data_file, QFileInfoList file_info, QString dir_name, QString raw_dir)
{
    data_file<<endl<<dir_name.toStdString();
    qDebug()<<dir_name;
    QString test;
    string test_s;
    int i;
    stack<QFileInfoList> subdir_stack;
    stack<QString> subdir_name_stack;
    QFileInfoList::iterator iter=file_info.begin();
    while(iter->fileName()!=".")
        iter++;
    QDateTime time = iter->lastModified();
    test+=" ";
    test+=QString::number(iter->groupId());
    test+=" ";
    test+=QString::number(iter->ownerId());
    test+=" ";
    i=-1;
    if(iter->isDir())
        i=0;
    else if(iter->isFile())
        i=1;
    else if(iter->isSymLink())
        i=2;
    test+=QString::number(i);
    test+=" ";
    test+=QString::number(iter->permissions());
    test+=" ";
    test+=time.toString("yyyyMMddhhmm.ss");
    test_s=test.toStdString();
    data_file<<test_s;
    iter=file_info.begin();
    for(;iter!=file_info.end();iter++)
    {
        if(iter->fileName()!="."&&iter->fileName()!="..")
        {
            if(iter->isDir())
            {
                QString subdir_name = dir_name +iter->fileName() + "/";
                QDir d(raw_dir + subdir_name);
                QFileInfoList subdir = d.entryInfoList();
                subdir_stack.push(subdir);
                subdir_name_stack.push(subdir_name);
                test="\n";
                //qDebug()<<iter->fileName()<<" "<<iter->isDir()<<" "<<iter->isFile()<<" "<<iter->isSymLink()<<" "<<iter->lastModified()<<" "<<iter->birthTime();
                QDateTime time = iter->lastModified();
                //qDebug()<<time.toString();
                test += "\"";
                test+=iter->fileName();
                test += "\"";
                test+=" ";
                test+=QString::number(iter->groupId());
                test+=" ";
                test+=QString::number(iter->ownerId());
                test+=" ";
                if(iter->isDir())
                    i=0;
                else if(iter->isFile())
                    i=1;
                else if(iter->isSymLink())
                    i=2;
                test+=QString::number(i);
                test+=" ";
                test+=QString::number(iter->permissions());
                test+=" ";
                test+=time.toString("yyyyMMddhhmm.ss");
                test_s=test.toStdString();
                data_file<<test_s;
                qDebug()<<test;
                test.clear();
            }
            else
            {
                QString test;
                test="\n";
                //qDebug()<<iter->fileName()<<" "<<iter->isDir()<<" "<<iter->isFile()<<" "<<iter->isSymLink()<<" "<<iter->lastModified()<<" "<<iter->birthTime();
                QDateTime time = iter->lastModified();
                //qDebug()<<time.toString();
                test += "\"";
                test+=iter->fileName();
                test += "\"";
                test+=" ";
                test+=QString::number(iter->groupId());
                test+=" ";
                test+=QString::number(iter->ownerId());
                test+=" ";
                if(iter->isDir())
                    i=0;
                else if(iter->isFile())
                    i=1;
                else if(iter->isSymLink())
                    i=2;
                test+=QString::number(i);
                test+=" ";
                test+=QString::number(iter->permissions());
                test+=" ";
                test+=time.toString("yyyyMMddhhmm.ss");
                test_s=test.toStdString();
                data_file<<test_s;
                qDebug()<<test;
                test.clear();
            }
        }

    }

    while(!subdir_stack.empty())
    {
        QFileInfoList subdir = subdir_stack.top();
        QString subdir_name = subdir_name_stack.top();
        readFileInfo(data_file,subdir,subdir_name,raw_dir);
        subdir_stack.pop();
        subdir_name_stack.pop();
    }
    return 0;
}

int CopyFile(QFileInfoList& file_list, QString dir_name, QString raw_dir, QString backup_dir)
{
    QFileInfoList::iterator iter=file_list.begin();
    for(;iter!=file_list.end();iter++)
    {
        if(iter->isDir())
        {
            if(iter->fileName()==".."||iter->fileName()==".")
                continue;
            QString subdir_name = dir_name + "/" +iter->fileName();
            QDir d(raw_dir + subdir_name);
            QDir subdir_d;
            QFileInfoList subdir = d.entryInfoList();
            if(!subdir_d.exists(backup_dir+subdir_name))
                    qDebug()<<"Make backup diretory \""<<subdir_d.mkdir(backup_dir+"/"+subdir_name)<<"\" "<<endl;
            qDebug()<<dir_name<<subdir_name;
            CopyFile(subdir,subdir_name,raw_dir,backup_dir);
        }
        else
        {
            QProcess q;
            if(iter->isSymLink())
            {
                q.execute("cp -d \"" + iter->absoluteFilePath() + "\" \"" + backup_dir + dir_name + "\"");
                if(q.exitCode()!=0)
                    qDebug()<<"cp fail";
            }
            else
            {
                q.execute("cp \"" + iter->absoluteFilePath() + "\" \"" + backup_dir + dir_name + "\"");
                if(q.exitCode()!=0)
                    qDebug()<<"cp fail";
            }
            q.close();
        }
    }
    return 0;
}

int recovery(QString src_path, QString target_path)
{
    QFile data_file(src_path + "/.b_data.txt");
    if(!data_file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug()<<"Loading copy file data fail."<<endl;
        return -1;
    }
    vector<DIR_LIST_STRUCT> dir_list_vector;

    if(check_src_file_info(src_path,data_file,dir_list_vector)!=0)
    {
        return -1;
    }

    check_copy_file(target_path, dir_list_vector);

    vector<DIR_LIST_STRUCT>::iterator dir_iter;
    QStack<BACKUP_INFO_STRUCT> dir_info_stack;

    for(dir_iter = dir_list_vector.begin();dir_iter < dir_list_vector.end();dir_iter++)
    {
        QDir d(target_path + dir_iter->dir_name.left(dir_iter->dir_name.length()-1));
        if(!d.exists())
        {
            qDebug()<<"Make recovery diretory \""<<d.mkdir(target_path+"/"+dir_iter->dir_name.left(dir_iter->dir_name.length()-1))<<"\" "<<endl;
        }
        vector<BACKUP_INFO_STRUCT>::iterator info_iter;
        info_iter=dir_iter->info_list.begin();
        dir_info_stack.push(*info_iter);
        for(info_iter=dir_iter->info_list.begin()+1;info_iter<dir_iter->info_list.end();info_iter++)
        {
            if(info_iter->need_copy == true)
            {
                if(info_iter->type == DIR)
                    continue;
                QProcess q;
                if(info_iter->type==Sym_link)
                {
                    qDebug()<<"cp -d \"" + src_path + dir_iter->dir_name + info_iter->name + "\" \"" + target_path + dir_iter->dir_name + "\"";
                    qDebug()<<"chgrp " + QString::number(info_iter->gid) + " \"" +  target_path + dir_iter->dir_name + "\"";
                    qDebug()<<"chown " + QString::number(info_iter->uid) + " \"" +  target_path + dir_iter->dir_name + "\"";
                    qDebug()<<"touch -t " + info_iter->m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  target_path + dir_iter->dir_name + "\"";


                    q.execute("cp -d \"" + src_path + dir_iter->dir_name + info_iter->name + "\" \"" + target_path + dir_iter->dir_name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"cp fail";
                        return -1;
                    }
                    q.execute("chgrp " + QString::number(info_iter->gid) + " \"" +  target_path + dir_iter->dir_name + info_iter->name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"chgrp fail";
                        return -1;
                    }
                    q.execute("chown " + QString::number(info_iter->uid) + " \"" +  target_path + dir_iter->dir_name + info_iter->name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"chown fail";
                        return -1;
                    }
                    q.execute("touch -t " + info_iter->m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  target_path + dir_iter->dir_name + info_iter->name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"touch fail";
                        return -1;
                    }

                }
                else
                {
                    qDebug()<<"cp -d \"" + src_path + dir_iter->dir_name + info_iter->name + "\" \"" + target_path + dir_iter->dir_name + "\"";
                    qDebug()<<"chgrp " + QString::number(info_iter->gid) + " \"" +  target_path + dir_iter->dir_name + "\"";
                    qDebug()<<"chown " + QString::number(info_iter->uid) + " \"" +  target_path + dir_iter->dir_name + "\"";
                    qDebug()<<"touch -t " + info_iter->m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  target_path + dir_iter->dir_name + "\"";


                    q.execute("cp \"" + src_path + dir_iter->dir_name + info_iter->name + "\" \"" + target_path + dir_iter->dir_name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"cp fail";
                        return -1;
                    }
                    q.execute("chgrp " + QString::number(info_iter->gid) + " \"" +  target_path + dir_iter->dir_name + info_iter->name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"chgrp fail";
                        return -1;
                    }
                    q.execute("chown " + QString::number(info_iter->uid) + " \"" +  target_path + dir_iter->dir_name + info_iter->name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"chown fail";
                        return -1;
                    }
                    q.execute("touch -t " + info_iter->m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  target_path + dir_iter->dir_name + info_iter->name + "\"");
                    if(q.exitCode()!=0)
                    {
                        qDebug()<<"touch fail";
                        return -1;
                    }

                }
            }
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
        q.execute("chgrp " + QString::number(temp_struct.gid) + " \"" +  target_path + temp_struct.name);
        if(q.exitCode()!=0)
        {
            qDebug()<<"chgrp fail";
            return -1;
        }
        q.execute("chown " + QString::number(temp_struct.uid) + " \"" +  target_path + temp_struct.name);
        if(q.exitCode()!=0)
        {
            qDebug()<<"chown fail";
            return -1;
        }
        q.execute("touch -t " + temp_struct.m_time.toString("yyyyMMddhhmm.ss") + " " + " \"" +  target_path + temp_struct.name);
        if(q.exitCode()!=0)
        {
            qDebug()<<"touch fail";
            return -1;
        }
        dir_info_stack.pop();
    }
    return 0;
}

int check_src_file_info(QString src_path, QFile& data_file, vector<DIR_LIST_STRUCT>& dir_list_vector)
{
    //QDir src(src_path);
    //QFileInfoList src_file_info = src.entryInfoList();

    vector<BACKUP_INFO_STRUCT> file_info_vector;
    //vector<DIR_LIST_STRUCT> dir_list_vector;

    QString test;
    QString dir_name_temp;
    QStringList src_file_info;
    DIR_LIST_STRUCT dir_list_temp;
    while(!data_file.atEnd())
    {
        BACKUP_INFO_STRUCT temp_struct;
        test = data_file.readLine();
        test=test.trimmed();
        if(test.isEmpty())
            continue;
        if(test.startsWith("/"))
        {
            if(!dir_name_temp.isEmpty())
            {
                dir_list_temp.dir_name = dir_name_temp;
                dir_list_temp.info_list = file_info_vector;
                dir_list_temp.need_copy = false;
                dir_list_vector.push_back(dir_list_temp);
                file_info_vector.clear();
            }
            int last = test.lastIndexOf('/');
            dir_name_temp = test.left(last + 1);
            QDir src(src_path + dir_name_temp.left(dir_name_temp.length()-1));
            if(!src.exists())
            {
                qDebug()<<"Backup file lost! "<<temp_struct.name<<" lost.";
                return -1;
            }
            test=test.mid(last+2);
            QStringList temp_string_list=test.split(" ");
            temp_struct.name = dir_name_temp;
            temp_struct.gid=temp_string_list.at(0).toUInt();
            temp_struct.uid=temp_string_list.at(1).toUInt();
            temp_struct.type=(FILE_TYPE_ENUM)temp_string_list.at(2).toInt();
            temp_struct.permission=temp_string_list.at(3).toInt();
            temp_struct.m_time=QDateTime::fromString(temp_string_list.at(4),"yyyyMMddhhmm.ss");
            temp_struct.need_copy=false;
            file_info_vector.push_back(temp_struct);
            src_file_info = src.entryList();
            continue;
        }
        int first = test.indexOf('\"');
        int last = test.lastIndexOf('\"');
        QString file_name = test.mid(first+1,last-first-1);
        temp_struct.name = file_name;
        test=test.mid(last+2);
        QStringList temp_string_list=test.split(" ");

        temp_struct.gid=temp_string_list.at(0).toUInt();
        temp_struct.uid=temp_string_list.at(1).toUInt();
        temp_struct.type=(FILE_TYPE_ENUM)temp_string_list.at(2).toInt();
        temp_struct.permission=temp_string_list.at(3).toInt();
        temp_struct.m_time=QDateTime::fromString(temp_string_list.at(4),"yyyyMMddhhmm.ss");
        temp_struct.need_copy=false;
        file_info_vector.push_back(temp_struct);

        if(src_file_info.indexOf(temp_struct.name)<0)
        {
            qDebug()<<"Backup file lost! "<<temp_struct.name<<" lost.";
            return -1;
        }
    }
    if(!dir_name_temp.isEmpty())
    {
        dir_list_temp.dir_name = dir_name_temp;
        dir_list_temp.info_list = file_info_vector;
        dir_list_temp.need_copy = false;
        dir_list_vector.push_back(dir_list_temp);
        file_info_vector.clear();
    }
    qDebug()<<"Check done";
    return 0;
}

void check_copy_file(QString target_path, vector<DIR_LIST_STRUCT>& dir_list_vector)
{
    //QDir target(target_path);
    //QFileInfoList target_file_info = target.entryInfoList();

    vector<DIR_LIST_STRUCT>::iterator dir_iter;
    for(dir_iter = dir_list_vector.begin();dir_iter<dir_list_vector.end();dir_iter++)
    {
        QDir target(target_path + dir_iter->dir_name.left(dir_iter->dir_name.length()-1));
        QStringList target_name_list = target.entryList();
        vector<BACKUP_INFO_STRUCT>::iterator info_iter;
        for(info_iter = dir_iter->info_list.begin()+1;info_iter<dir_iter->info_list.end();info_iter++)
        {
            if(target_name_list.indexOf(info_iter->name)<0)
            {
                info_iter->need_copy = true;
            }
            else
            {
                QFileInfo checking_file(target_path + dir_iter->dir_name + "/" + info_iter->name);

                uint ttime = checking_file.lastModified().toTime_t();
                uint stime = info_iter->m_time.toTime_t();

                if(
                        !(checking_file.groupId()==info_iter->gid &&
                        checking_file.ownerId()==info_iter->uid &&
                        check_file_type(checking_file,info_iter->type) &&
                        (int)checking_file.permissions()==info_iter->permission &&
                        ttime<=stime)
                        )
                {
                    info_iter->need_copy=true;
                }
            }
        }
    }
    return;
}

bool check_file_type(QFileInfo& info, FILE_TYPE_ENUM type)
{
    if(type==DIR && info.isDir())
        return true;
    else if(type==C_FILE && info.isFile())
        return true;
    else if(type==Sym_link && info.isSymLink())
        return true;
    else
        return false;
    return false;
}

int rmAllFile(QFileInfo& info)
{
    if(info.isDir())
    {
        QDir dir(info.filePath());
        foreach (auto file, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden)) {
            rmAllFile(file);
        }
        if(!dir.rmdir(info.filePath()))
        {
            qDebug()<<"rmdir fail";
            return -1;
        }
        return 0;
    }
    if(!info.absoluteDir().remove(info.filePath()))
    {
        qDebug()<<"rmfile fail";
        return -1;
    }
    return 0;
}

int overwriteBackup(QString backup_dir, QString raw_dir)
{
    QDir r(backup_dir);
    QFileInfoList b_info = r.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
    foreach(auto b_iter, b_info)
    {
        if(rmAllFile(b_iter)!=0)
        {
            qDebug()<<"Clear backup directory fail";
            return -1;
        }
    }

    QDir d(raw_dir);
    QFileInfoList file_info = d.entryInfoList(QDir::AllEntries | QDir::Hidden);
    std::fstream data_file;
    data_file.open(backup_dir.toStdString() + "/.b_data.txt",ios::in|ios::out|ios::trunc);
    if(!data_file.is_open())
    {
        qDebug()<<"Open fail."<<endl;
        return -1;
    }
    if(readFileInfo(data_file,file_info,"/",raw_dir)!=0)
    {
        return -1;
    }
    data_file.close();

    QFile d_data(backup_dir+"/.b_data.txt");
    if(CopyFile(file_info,"",raw_dir,backup_dir)!=0)
    {
        return -1;
    }
    return 0;
}

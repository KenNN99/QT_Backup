    #include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include "dialog.h"
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_OpenDir_clicked()
{
    int i;
    Dir_name = QFileDialog::getExistingDirectory(this,tr("Dir Dialog"));
    ui->dir_display->setText(Dir_name);
    QDir d(Dir_name);
    file_info = d.entryInfoList();

    qDebug()<<file_info.count();

    if(scoll_pointer == NULL)
    {
        scoll_pointer = new QGridLayout();//网格布局
        qDebug()<<"New pointer"<<endl;
    }
    else
    {
        qDebug()<<"Pointer already exist"<<endl;
        //清空horizontalLayout布局内的所有元素
        QLayoutItem *child;
         while ((child = scoll_pointer->takeAt(0)) != 0)
         {
                //setParent为NULL，防止删除之后界面不消失
                if(child->widget())
                {
                    child->widget()->setParent(NULL);
                }
                delete child;
         }
    }
    scoll_pointer->setColumnStretch(0,3);
    scoll_pointer->setColumnStretch(1,1);

    QFileInfoList::iterator iter=file_info.begin();
    i=0;
    for(;iter!=file_info.end();iter++)
    {
        qDebug()<<iter->fileName()<<" "<<iter->isDir()<<" "<<iter->isFile()<<" "<<iter->isSymLink()<<endl;
        QLineEdit *pLe = new QLineEdit(iter->fileName(), this);
        QLineEdit *pLe1 = new QLineEdit();
        if(iter->isDir())
            pLe1->setText("Directory");
        else if(iter->isFile())
            pLe1->setText("File");
        pLe->setMinimumWidth(500);//可以注释掉该行，观察效果
        scoll_pointer->addWidget(pLe, i, 0);//把输入框添加到布局的第i行第0列
        scoll_pointer->addWidget(pLe1, i, 1);//把按钮添加到布局的第i行第1列
        i++;
    }

    ui->scrollArea->widget()->setLayout(scoll_pointer);//把布局放置到QScrollArea的内部QWidget中
}

void MainWindow::on_backupButton_clicked()
{

    QDir backup_dir;
    if(!backup_dir.exists(backup_dir_name))
        qDebug()<<"Make backup diretory "<<backup_dir.mkdir(backup_dir_name)<<" "<<endl;
    CopyFile(file_info,"");
    QFile database("database.bakd");
    database.open(QIODevice::ReadWrite);
    database.write("Hello");
    database.close();
    Dialog* dia_window = new Dialog();
    dia_window->show();
}

void MainWindow::CopyFile(QFileInfoList file_list, QString dir_name)
{
    QFileInfoList::iterator iter=file_list.begin();
    for(;iter!=file_list.end();iter++)
    {
        if(iter->isDir())
        {
            if(iter->fileName()==".."||iter->fileName()==".")
                continue;
            QString subdir_name = dir_name + "/" +iter->fileName();
            QDir d(Dir_name + subdir_name);
            QDir subdir_d;
            QFileInfoList subdir = d.entryInfoList();
            if(!subdir_d.exists(backup_dir_name+subdir_name))
                    qDebug()<<"Make backup diretory "<<subdir_d.mkdir(backup_dir_name+"/"+subdir_name)<<" "<<endl;
            qDebug()<<dir_name<<subdir_name;
            CopyFile(subdir,subdir_name);
        }
        else
        {
            QProcess q;
            if(iter->isSymLink())
            {
                q.execute("cp -d " + iter->absoluteFilePath() + " " + backup_dir_name + dir_name);
            }
            else
            {
                q.execute("cp " + iter->absoluteFilePath() + " " + backup_dir_name + dir_name);
            }
            q.close();
        }
    }
}

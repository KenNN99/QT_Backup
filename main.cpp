#include "mainwindow.h"
#include <QDir>
#include <QApplication>
#include <QFileInfoList>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QDir d("/home/ken/Backup");
    QFileInfoList file_info = d.entryInfoList();

    qDebug()<<file_info.count();

    QFileInfoList::iterator iter=file_info.begin();

    for(;iter!=file_info.end();iter++)
    {
        qDebug()<<iter->fileName()<<" "<<iter->isDir()<<" "<<iter->isFile()<<" "<<iter->isSymLink()<<endl;
    }

    return a.exec();
}

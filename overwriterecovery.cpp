#include "overwriterecovery.h"
#include "ui_overwriterecovery.h"

overwriteRecovery::overwriteRecovery(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::overwriteRecovery)
{
    ui->setupUi(this);
}

overwriteRecovery::~overwriteRecovery()
{
    delete ui;
}

void overwriteRecovery::on_buttonBox_2_accepted()
{
    emit confirmInfo(true);
}

void overwriteRecovery::on_buttonBox_2_rejected()
{
    emit confirmInfo(true);
}

void overwriteRecovery::get_file_list(QString backup_dir)
{
    qDebug()<<backup_dir;

    model = new QFileSystemModel;

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->setRootPath(backup_dir));
    ui->treeView->header()->resizeSection(0,400);
}

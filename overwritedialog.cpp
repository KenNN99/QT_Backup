#include "overwritedialog.h"
#include "ui_overwritedialog.h"

overWriteDialog::overWriteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::overWriteDialog)
{
    ui->setupUi(this);
}

overWriteDialog::~overWriteDialog()
{
    delete ui;
}

void overWriteDialog::on_buttonBox_accepted()
{
    emit confirmInfo(true);
}

void overWriteDialog::on_buttonBox_rejected()
{
    emit confirmInfo(false);
}

void overWriteDialog::get_file_list(QString backup_dir)
{
    qDebug()<<backup_dir;

    model = new QFileSystemModel;

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->setRootPath(backup_dir));
    ui->treeView->header()->resizeSection(0,400);
}

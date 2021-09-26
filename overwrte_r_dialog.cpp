#include "overwrte_r_dialog.h"
#include "ui_overwrte_r_dialog.h"

overwrte_R_Dialog::overwrte_R_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::overwrte_R_Dialog)
{
    ui->setupUi(this);
}

overwrte_R_Dialog::~overwrte_R_Dialog()
{
    delete ui;
}


void overwrte_R_Dialog::on_buttonBox_accepted()
{
    emit confirmInfo(true);
}

void overwrte_R_Dialog::on_buttonBox_rejected()
{
    emit confirmInfo(true);
}

void overwrte_R_Dialog::get_file_list(QString backup_dir)
{
    qDebug()<<backup_dir;

    model = new QFileSystemModel;

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->setRootPath(backup_dir));
    ui->treeView->header()->resizeSection(0,400);
}


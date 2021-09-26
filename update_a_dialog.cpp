#include "update_a_dialog.h"
#include "ui_update_a_dialog.h"

Update_a_Dialog::Update_a_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Update_a_Dialog)
{
    ui->setupUi(this);
}

Update_a_Dialog::~Update_a_Dialog()
{
    delete ui;
}

void Update_a_Dialog::get_file_list(QStringList file_list)
{
    qDebug()<<file_list;

    QListView* t = ui->listView;
    t->setEditTriggers(QListView::NoEditTriggers);			//单元格不能编辑
    t->setSelectionBehavior(QListView::SelectRows);			//一次选中整行
    t->setSelectionMode(QListView::SingleSelection);        //单选，配合上面的整行就是一次选单行
    t->setFocusPolicy(Qt::NoFocus);                         //去掉鼠标移到单元格上时的虚线框

    model = new QStandardItemModel(ui->listView);
    foreach (auto list_iter, file_list) {
        QFileInfo* info = new QFileInfo(list_iter);
        QStandardItem* item0 = new QStandardItem(info->fileName());
        model->appendRow(item0);
    }

    ui->listView->setModel(model);
}

void Update_a_Dialog::on_buttonBox_accepted()
{
    emit confirmInfo(true);
}

void Update_a_Dialog::on_buttonBox_rejected()
{
    emit confirmInfo(false);
}

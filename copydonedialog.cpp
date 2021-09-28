#include "copydonedialog.h"
#include "ui_copydonedialog.h"

CopyDoneDialog::CopyDoneDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CopyDoneDialog)
{
    ui->setupUi(this);
}

CopyDoneDialog::~CopyDoneDialog()
{
    delete ui;
}

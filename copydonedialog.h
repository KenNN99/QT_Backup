#ifndef COPYDONEDIALOG_H
#define COPYDONEDIALOG_H

#include <QDialog>

namespace Ui {
class CopyDoneDialog;
}

class CopyDoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CopyDoneDialog(QWidget *parent = nullptr);
    ~CopyDoneDialog();

private:
    Ui::CopyDoneDialog *ui;
};

#endif // COPYDONEDIALOG_H

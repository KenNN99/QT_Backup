#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QDebug>
#include <QFileInfo>

namespace Ui {
class updateDialog;
}

class updateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit updateDialog(QWidget *parent = nullptr);
    ~updateDialog();

signals:
    void confirmInfo(bool);

private slots:
    void get_file_list(QStringList file_list);

    void on_buttonBox_2_accepted();

    void on_buttonBox_2_rejected();

private:
    Ui::updateDialog *ui;
    QStandardItemModel* model;
};

#endif // UPDATEDIALOG_H

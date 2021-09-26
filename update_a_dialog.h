#ifndef UPDATE_A_DIALOG_H
#define UPDATE_A_DIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QDebug>
#include <QFileInfo>

namespace Ui {
class Update_a_Dialog;
}

class Update_a_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Update_a_Dialog(QWidget *parent = nullptr);
    ~Update_a_Dialog();

signals:
    void confirmInfo(bool);

private slots:
    void get_file_list(QStringList file_list);


    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::Update_a_Dialog *ui;
    QStandardItemModel* model;
};

#endif // UPDATE_A_DIALOG_H

#ifndef OVERWRTE_R_DIALOG_H
#define OVERWRTE_R_DIALOG_H

#include <QDialog>
#include <QFileSystemModel>
#include <QDebug>
#include <QFileInfo>

namespace Ui {
class overwrte_R_Dialog;
}

class overwrte_R_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit overwrte_R_Dialog(QWidget *parent = nullptr);
    ~overwrte_R_Dialog();

signals:
    void confirmInfo(bool);

private slots:
    void get_file_list(QString backup_dir);


    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::overwrte_R_Dialog *ui;
    QFileSystemModel* model;
};

#endif // OVERWRTE_R_DIALOG_H

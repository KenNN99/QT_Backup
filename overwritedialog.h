#ifndef OVERWRITEDIALOG_H
#define OVERWRITEDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QDateTime>
#include <QDebug>

namespace Ui {
class overWriteDialog;
}

class overWriteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit overWriteDialog(QWidget *parent = nullptr);
    ~overWriteDialog();

signals:
    void confirmInfo(bool flag);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void get_file_list(QString backup_dir);

private:
    Ui::overWriteDialog *ui;
    QFileSystemModel* model;
};

#endif // OVERWRITEDIALOG_H

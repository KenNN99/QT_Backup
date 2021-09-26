#ifndef OVERWRITERECOVERY_H
#define OVERWRITERECOVERY_H

#include <QDialog>
#include <QDebug>
#include <QFileSystemModel>

namespace Ui {
class overwriteRecovery;
}

class overwriteRecovery : public QDialog
{
    Q_OBJECT

public:
    explicit overwriteRecovery(QWidget *parent = nullptr);
    ~overwriteRecovery();

signals:
    void confirmInfo(bool flag);

private slots:
    void get_file_list(QString backup_dir);


    void on_buttonBox_2_accepted();

    void on_buttonBox_2_rejected();

private:
    Ui::overwriteRecovery *ui;
    QFileSystemModel* model;
};

#endif // OVERWRITERECOVERY_H

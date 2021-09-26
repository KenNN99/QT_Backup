#ifndef APPENDDIALOG_H
#define APPENDDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QDateTime>
#include <QDebug>

namespace Ui {
class AppendDialog;
}

class AppendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppendDialog(QWidget *parent = nullptr);
    ~AppendDialog();

signals:
    void confirmInfo(bool flag);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void get_file_list(QStringList file_list);

private:
    Ui::AppendDialog *ui;
    QStandardItemModel* model;
};

#endif // APPENDDIALOG_H

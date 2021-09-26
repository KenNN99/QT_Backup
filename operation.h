#ifndef OPERATION_H
#define OPERATION_H

#include <fstream>
#include <QFileInfoList>
#include <string>
#include <QDateTime>

enum FILE_TYPE_ENUM
{
    DIR,
    C_FILE,
    Sym_link
};

struct BACKUP_INFO_STRUCT
{
    QString name;
    uint gid;
    uint uid;
    FILE_TYPE_ENUM type;
    int permission;
    QDateTime m_time;
    bool need_copy;
};

struct DIR_LIST_STRUCT
{
    QString dir_name;
    std::vector<BACKUP_INFO_STRUCT> info_list;
    bool need_copy;
};

int readFileInfo(std::fstream& data_file, QFileInfoList file_info, QString dir_name, QString raw_dir);
int CopyFile(QFileInfoList& file_list, QString dir_name, QString raw_dir, QString backup_dir);
int check_src_file_info(QString src_path, QFile& data_file, std::vector<DIR_LIST_STRUCT>& dir_list_vector);
bool check_file_type(QFileInfo& info, FILE_TYPE_ENUM type);
void check_copy_file(QString target_path, std::vector<DIR_LIST_STRUCT>& dir_list_vector);
int recovery(QString src_path, QString target_path);
int rmAllFile(QFileInfo& info);
int overwriteBackup(QString backup_dir, QString raw_dir);

#endif // OPERATION_H

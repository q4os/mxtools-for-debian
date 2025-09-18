#ifndef PMFILES_H
#define PMFILES_H

#include "cmd.h"

//class QString;
//class QStringList;

class PMFiles
{
public:
    PMFiles();
    QString get_qdistro();
    bool get_debian_mxtools_installed();
    bool get_has_debian_backports_repo();
    bool get_has_mx_repo();
    bool get_is_ubuntu_based();
    bool checkpm(const QString pmfile_name);

private:
    void setup();

    Cmd cmd;

    bool flag_debian_mxtools_installed = false;
    bool has_debian_backports_repo = false;
    bool has_mx_repo = false; //mx repository is present in the system
    bool is_ubuntu_based = false; //system based on ubuntu
    QString qdistro = "unknown";
    QStringList pm_whitelist = QStringList(); //whitelisted pm files
    QStringList pm_blacklist = QStringList(); //blacklisted pm files
    bool blacklist_file_exists = false; //whitelist file found
    bool whitelist_file_exists = false; //blacklist file found
};

#endif // PMFILES_H

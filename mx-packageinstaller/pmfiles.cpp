#include <QDebug>
#include <QString>
#include <QStringList>
#include <QFile>

#include "pmfiles.h"

PMFiles::PMFiles()
{
  setup();
}

void PMFiles::setup()
{
  pm_blacklist = QStringList();
  pm_whitelist = QStringList();
  blacklist_file_exists = false;
  whitelist_file_exists = false;
  qdistro = "unknown";
  q4os_mxtools_installed = false;
  has_debian_backports_repo = false;
  has_mx_repo = false;
  is_ubuntu_based = false;

  if(QFile::exists("/usr/share/apps/q4os_system/bin/print_qaptdistr.sh")) {
    qdistro = cmd.getOut("dash /usr/share/apps/q4os_system/bin/print_qaptdistr.sh");
  }
  if(qdistro.length() < 1) {
    if(has_mx_repo)
      qdistro = "mxlinux";
    else
      qdistro = "unknown";
  }
  qDebug() << "qdistro: " << qdistro;

  QString hstr1 = cmd.getOut("dpkg --get-selections q4os-mxtools-common 2>/dev/null");
  if(hstr1.length() > 10) {
    q4os_mxtools_installed = true;
  }
  qDebug() << "q4os_mxtools_installed: " << q4os_mxtools_installed;

  hstr1 = cmd.getOut("grep '" + qdistro + "-backports ' /etc/apt/sources.list.d/*.list /etc/apt/sources.list | grep '.list:deb ' | grep 'debian'");
  if(hstr1.length() > 10) {
    has_debian_backports_repo = true;
  }
  qDebug() << "has_debian_backports_repo: " << has_debian_backports_repo;

  hstr1 = cmd.getOut("grep mx/testrepo /etc/apt/sources.list.d/*.list | grep \"deb \"");
  if(hstr1.length() > 10) {
    has_mx_repo = true;
  }
  qDebug() << "has_mx_repo: " << has_mx_repo;

  hstr1 = cmd.getOut("lsb_release -d | grep Ubuntu");
  if(hstr1.length() > 8) {
    is_ubuntu_based = true;
  }
  qDebug() << "is_ubuntu_based: " << is_ubuntu_based;

  QString file1_str = "", file2_str = "";
  if(has_mx_repo) {
    file1_str = "/usr/share/mx-packageinstaller-pkglist/pm_mx_" + qdistro + "_disabled.lst";
    file2_str = "/usr/share/mx-packageinstaller-pkglist/pm_mx_" + qdistro + "_enabled.lst";
  } else {
    file1_str = "/usr/share/mx-packageinstaller-pkglist/pm_" + qdistro + "_disabled.lst";
    file2_str = "/usr/share/mx-packageinstaller-pkglist/pm_" + qdistro + "_enabled.lst";
  }
  QFile file1(file1_str);
  if(file1.open(QFile::ReadOnly | QFile::Text)) {
    while(! file1.atEnd()) {
      pm_blacklist.append(QString(file1.readLine()).remove("\n"));
    }
    file1.close();
    pm_blacklist.removeAll(QString(""));
    pm_blacklist.removeDuplicates();
    pm_blacklist.sort();
    blacklist_file_exists = true;
  }
  QFile file2(file2_str);
  if(file2.open(QFile::ReadOnly | QFile::Text)) {
    while(! file2.atEnd()) {
      pm_whitelist.append(QString(file2.readLine()).remove("\n"));
    }
    file2.close();
    pm_whitelist.removeAll(QString(""));
    pm_whitelist.removeDuplicates();
    pm_whitelist.sort();
    whitelist_file_exists = true;
  }
}

bool PMFiles::get_q4os_mxtools_installed()
{
  return(q4os_mxtools_installed);
}

bool PMFiles::get_has_debian_backports_repo()
{
  return(has_debian_backports_repo);
}

bool PMFiles::get_has_mx_repo()
{
  return(has_mx_repo);
}

bool PMFiles::get_is_ubuntu_based()
{
  return(is_ubuntu_based);
}

QString PMFiles::get_qdistro()
{
  return(qdistro);
}

bool PMFiles::checkpm(const QString pmfile_name)
{
  //qDebug() << "Checking: " << pmfile_name;
  if(blacklist_file_exists) {
    //qDebug() << "blacklist: indexof: " << pmfile_name << " :: " << pm_blacklist.indexOf(pmfile_name);
    if(pm_blacklist.indexOf(pmfile_name) >= 0) //is pmfile_name on blacklist ?
      return(false);
    else
      return(true);
  }
  if(whitelist_file_exists) {
    //qDebug() << "whitelist: indexof: " << pmfile_name << " :: " << pm_whitelist.indexOf(pmfile_name);
    if(pm_whitelist.indexOf(pmfile_name) >= 0)
      return(true);
    else
      return(false);
  }
  return(true);
}

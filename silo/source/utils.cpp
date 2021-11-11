
#include "utils.h"

#include <QStandardPaths>
#include <QDir>

using namespace std;

string top_dir()
{
    auto top_dir = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory) + "revere" + QDir::separator() + "silo";
    if(!QDir(top_dir).exists())
        QDir().mkpath(top_dir);
    return top_dir.toStdString();
}

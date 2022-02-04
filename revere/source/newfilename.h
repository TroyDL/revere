#ifndef __NewFileName_h
#define __NewFileName_h

#include <QDialog>
#include "ui_newfilename.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NewFileName; }
QT_END_NAMESPACE

class NewFileName : public QDialog
{
    Q_OBJECT

public:
    NewFileName(QWidget *parent = nullptr);
    ~NewFileName();

private:
    Ui::NewFileName* _ui;
};

#endif

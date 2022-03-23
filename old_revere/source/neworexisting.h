#ifndef __NewOrExisting_h
#define __NewOrExisting_h

#include <QDialog>
#include "ui_neworexisting.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NewOrExisting; }
QT_END_NAMESPACE

class NewOrExisting : public QDialog
{
    Q_OBJECT

public:
    NewOrExisting(QWidget *parent = nullptr);
    ~NewOrExisting();

private:
    Ui::NewOrExisting* _ui;
};

#endif

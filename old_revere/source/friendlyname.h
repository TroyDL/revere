#ifndef __FriendlyName_h
#define __FriendlyName_h

#include <QDialog>
#include "ui_rtspcredentials.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FriendlyName; }
QT_END_NAMESPACE

class FriendlyName : public QDialog
{
    Q_OBJECT

public:
    FriendlyName(QWidget *parent = nullptr);
    ~FriendlyName();

private:
    Ui::FriendlyName* _ui;
};

#endif

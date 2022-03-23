#ifndef __PleaseWait_h
#define __PleaseWait_h

#include <QDialog>
#include "ui_pleasewait.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PleaseWait; }
QT_END_NAMESPACE

class PleaseWait : public QDialog
{
    Q_OBJECT

public:
    PleaseWait(QWidget *parent = nullptr);
    ~PleaseWait();

private:
    Ui::PleaseWait* _ui;
};

#endif

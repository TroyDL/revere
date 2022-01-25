#ifndef __Retention_h
#define __Retention_h

#include <QDialog>
#include "ui_retention.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Retention; }
QT_END_NAMESPACE

class Retention : public QDialog
{
    Q_OBJECT

public:
    Retention(QWidget *parent = nullptr);
    ~Retention();

private:
    Ui::Retention* _ui;
};

#endif

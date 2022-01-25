#ifndef __RTSPCredentials_h
#define __RTSPCredentials_h

#include <QDialog>
#include "ui_rtspcredentials.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RTSPCredentials; }
QT_END_NAMESPACE

class RTSPCredentials : public QDialog
{
    Q_OBJECT

public:
    RTSPCredentials(QWidget *parent = nullptr);
    ~RTSPCredentials();

private:
    Ui::RTSPCredentials* _ui;
};

#endif

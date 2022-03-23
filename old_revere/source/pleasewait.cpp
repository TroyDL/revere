#include "pleasewait.h"
#include "ui_pleasewait.h"

using namespace std;

PleaseWait::PleaseWait(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::PleaseWait)
{
    _ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setModal(true);
}

PleaseWait::~PleaseWait()
{
    delete _ui;
}

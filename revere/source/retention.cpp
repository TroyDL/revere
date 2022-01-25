#include "retention.h"
#include "ui_retention.h"

using namespace std;

Retention::Retention(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::Retention)
{
    _ui->setupUi(this);
}

Retention::~Retention()
{
    delete _ui;
}

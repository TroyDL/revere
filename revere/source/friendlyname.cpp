#include "friendlyname.h"
#include "ui_friendlyname.h"

using namespace std;

FriendlyName::FriendlyName(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::FriendlyName)
{
    _ui->setupUi(this);
}

FriendlyName::~FriendlyName()
{
    delete _ui;
}

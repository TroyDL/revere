#include "neworexisting.h"
#include "ui_neworexisting.h"

using namespace std;

NewOrExisting::NewOrExisting(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::NewOrExisting)
{
    _ui->setupUi(this);
}

NewOrExisting::~NewOrExisting()
{
    delete _ui;
}

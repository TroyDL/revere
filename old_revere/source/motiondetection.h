#ifndef __MotionDetection_h
#define __MotionDetection_h

#include <QDialog>
#include "ui_motiondetection.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MotionDetection; }
QT_END_NAMESPACE

class MotionDetection : public QDialog
{
    Q_OBJECT

public:
    MotionDetection(QWidget *parent = nullptr);
    ~MotionDetection();

private:
    Ui::MotionDetection* _ui;
};

#endif

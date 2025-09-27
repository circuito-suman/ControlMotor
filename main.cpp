#include <QApplication>
#include "MotorControlWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Create the motor control widget
    MotorControlWidget *motorControl = new MotorControlWidget();
    motorControl->resize(800, 600);
    motorControl->show();

    return app.exec();
}

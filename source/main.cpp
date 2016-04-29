#include "mimemainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    int currentExitCode = 0;
    do
    {
        QApplication a(argc, argv);
        MIMEMainWindow w;
        w.show();
        currentExitCode = a.exec();
    } while(currentExitCode == MIMEMainWindow::RESTART_CODE);

    return currentExitCode;
}

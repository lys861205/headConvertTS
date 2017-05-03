#include <QtGui/QApplication>
#include "convertui.h"
#include "mainui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QApplication::setStyle("windows");
    //QApplication::setStyle("windowsxp");
    //QApplication::setStyle("motif");
    //QApplication::setStyle("cde");
    QApplication::setStyle("macintosh");
    //QApplication::setStyle("plastique");
    ConvertUI CUI;
    CUI.show();
    return a.exec();
}

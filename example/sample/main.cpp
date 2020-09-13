#include <QCoreApplication>
#include <qoption.h>
#include <QDebug>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto i = QOption<int>::Some(55);

    qDebug()<<i.unwrap_or(0,[&](){qDebug()<<"SORRY";});

    return a.exec();
}

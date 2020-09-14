#include <QCoreApplication>
#include <qoption.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto none_option = QOption<int>::NONE;
    auto i32_option =  QOption<int>::Some(55);
    auto appptr_opt =  QOption<QCoreApplication*>::Some(&a);

    // "OOOOOPS, something wrong"
    qDebug() << none_option.match<QString>(
                    [&](int val){
                        return QString::number(val);
                    },
                    [&]{
                        return "OOOOOPS, something wrong";
                    });

    // "55"
    qDebug() << i32_option.match<QString>(
                    [&](int val){
                        return QString::number(val);
                    },
                    [&]{
                        return "OOOOOPS, something wrong";
                    });

    // true
    qDebug() << i32_option.match<bool>(
                    [&](int val){
                        return val > 10;
                    },
                    [&](){
                        return false;
                    });

    // 0
    qDebug()<<none_option.unwrap_def(0);

    // 55
    qDebug()<<i32_option.unwrap_or([&](){
            qDebug()<<"SORRY, IT'S NONE OPTION";
            return 0;
    });

    // 0 and print "SORRY, IT'S NONE OPTION"
    qDebug()<<none_option.unwrap_or([&](){
            qDebug()<<"SORRY, IT'S NONE OPTION";
            return 0;
    });


    // addr of QCoreApplication a
    qDebug()<<appptr_opt.unwrap();

    return a.exec();
}

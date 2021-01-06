#include <QCoreApplication>
#include <qoption.h>
#include <QDebug>

#include <QElapsedTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QElapsedTimer tmr;
    tmr.start();
    std::vector<long> v;
    for(int i = 0; i < 1000000; i++)
        v.push_back(i);

    qDebug()<<tmr.restart();
    auto first = QOption<std::vector<long>>::Some(std::move(v));

     // after copy or moving first statement will be None
    auto second = std::move(first);

    qDebug()<<second.unwrap().size();
    // first.unwrap() - throws exception
    qDebug()<<tmr.elapsed();


    auto none_option = QOption<int>::NONE;
    auto i32_option =  QOption<int>::Some(55);
    auto appptr_opt =  QOption<QCoreApplication*>::Some(&a);

    // "OOOOOPS, something wrong" - option statement is None now
    qDebug() << none_option.match<QString>(
                    [&](int val){
                        return QString::number(val);
                    },
                    [&]{
                        return "OOOOOPS, something wrong";
                    });

    // "0x37" - is string value 55 as hex
    qDebug() << i32_option.match<QString>(
                    [&](int val){
                        return "0x"+QString::number(val, 16);
                    },
                    [&]{
                        return "OOOOOPS, something wrong";
                    });

    // false - value already moved and option statement is None now
    qDebug() << i32_option.match<bool>(
                    [&](int val){
                        return val > 10;
                    },
                    [&](){
                        return false;
                    });

    // 777 - default value
    qDebug()<<none_option.unwrap_def(777);

    // print "SORRY, IT'S NONE OPTION" and print 0 - option already moved and statement is None now
    qDebug()<<i32_option.unwrap_or([&](){
            qDebug()<<"SORRY, IT'S NONE OPTION";
            return 0;
    });

    // 0 and print "SORRY, IT'S NONE OPTION" - option statement is None now
    qDebug()<<none_option.unwrap_or([&](){
            qDebug()<<"SORRY, IT'S NONE OPTION";
            return 0;
    });


    // programm exec file path
    qDebug()<<appptr_opt.unwrap()->arguments();

    appptr_opt.if_none([]{
        // in this case exec this
        qDebug()<< "unwrapped";
    }).if_some([](QCoreApplication * app) {
        // not this
        qDebug()<<app->arguments();
    });

    appptr_opt.match<bool>([](QCoreApplication * app){
        // in this case not exec
        qDebug()<< app->arguments();
        return true;
    },
    [](){
        // in this case exec this
        qDebug()<< "unwrapped";
        return false;
    });



    return a.exec();
}

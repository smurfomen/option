#include <QCoreApplication>
#include <qoption.h>
#include <QDebug>

#include <QElapsedTimer>


QOption<int> sum(int a, int b) {
    int s = a+b;
    if(s%2 == 0)
        return s;

    return None();
}



QOption<QByteArray> arr() {
    return QByteArray(3,0);
    // or
    return None();
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QElapsedTimer tmr;
    tmr.start();
    std::vector<long> v;
    for(int i = 0; i < 1000000; i++)
        v.push_back(i);

    qDebug()<<tmr.restart();
    QOption<std::vector<long>> first = std::move(v);

     // after copy or moving first statement will be None
    auto second = std::move(first);

    qDebug()<<second.unwrap().size();
    // first.unwrap() - throws exception
    qDebug()<<tmr.elapsed();

    auto sumOpt = sum(10,12);
    if(sumOpt)
        qDebug()<<sumOpt.unwrap();

    auto array = arr();
    if(array)
        qDebug()<<array.unwrap().size();

    QOption<int> none_option = None();
    QOption<int> i32_option = 55;
    auto a_opt = Some(&a);
    // or
    // QOption<QCoreApplication*> a_opt = &a;

    // "OOOOOPS, something wrong" - option statement is None now
    qDebug() << none_option.match<QString>(
                    [&](int val){
                        return QString::number(val);
                    },
                    [&]{
                        return "match: none_option is None, return this string";
                    });

    // "0x37" - is string value 55 as hex
    qDebug() << i32_option.match<QString>(
                    [&](int val){
                        return "0x"+QString::number(val, 16);
                    },
                    [&]{
                        return "match: i32_option is None, return this string";
                    });

    // false - value already moved and option statement is None now
    qDebug() << i32_option.match<bool>(
                    [&](int val){
                        return val > 10;
                    },
                    [&](){
                        return false;
                    });

    i32_option = 44;
    if(i32_option)
        qDebug()<<"explicit bool () operator: i32_option is Some";

    // 777 - default value
    qDebug()<<none_option.unwrap_def(777);
    if(!none_option)
        qDebug()<<"bool ! operator: none_option is None";

    // print "SORRY, IT'S NONE OPTION" and print 0 - option already moved and statement is None now
    qDebug()<<i32_option.unwrap_or([&](){
            qDebug()<<"unwrap_or: i32_option already been unwrapped, return 0";
            return 0;
    });

    // 0 and print "SORRY, IT'S NONE OPTION" - option statement is None now
    qDebug()<<none_option.unwrap_or([&](){
            qDebug()<<"unwrap_or: sorry, none_option already been unwrapped, return 0";
            return 0;
    });


    // programm exec file path
    qDebug()<<a_opt.unwrap()->arguments();

    a_opt.if_none([]{
        // in this case exec this
        qDebug()<< "compousing: a_opt already been unwrapped";
    }).if_some([](QCoreApplication * app) {
        // not this
        qDebug()<<app->arguments();
    });

    a_opt.match<bool>([](QCoreApplication * app){
        // in this case not exec
        qDebug()<< app->arguments();
        return true;
    },
    [](){
        // in this case exec this
        qDebug()<< "match: a_opt already been unwrapped, return false";
        return false;
    });



    return a.exec();
}

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
    qDebug() << none_option.match(   Q_SOME(QString, int) impl ([&](int val){
                                    return QString::number(val);
                                  }),
                                  Q_NONE(QString) impl ([&]{
                                    return "OOOOOPS, something wrong";
                                  })
                              );

    // "55"
    qDebug() << i32_option.match(   Q_SOME(QString, int) impl ([&](int val){
                                        return QString::number(val);
                                    }),
                                    Q_NONE(QString) impl ([&]{
                                        return "OOOOOPS, something wrong";
                                    })
                                );

    // true
    qDebug() << i32_option.match(   Q_SOME(bool, int) impl ([&](int val){
                                        return val > 10;
                                    }),
                                    Q_NONE(bool) impl ([&]{
                                        return false;
                                    })
                                );

    // 551
    qDebug()<<none_option.unwrap_def(551);

    // 55
    qDebug()<<i32_option.unwrap_or(0,[&](){qDebug()<<"SORRY, IT'S NONE OPTION"; });

    // "SORRY, IT'S NONE OPTION"
    qDebug()<<i32_option.unwrap_or(0,[&](){qDebug()<<"SORRY, IT'S NONE OPTION"; });


    // addr of QCoreApplication a
    qDebug()<<appptr_opt.unwrap();

    return a.exec();
}

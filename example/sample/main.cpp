#include <QCoreApplication>
#include <qoption.h>
#include <QDebug>

class exc : public std::exception {
public:
    exc(const char *  msg){_M_msg = msg;}
    const char * _M_msg;

    const char * what() const noexcept override{
        return _M_msg;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication * ptr_a = &a;
    int dsa = 110;
    auto none_opt = QOption<int>::none;

    auto opt_55 =  QOption<int>::Some(55);
    auto dsa_opt = QOption<int>::Some(dsa);

    auto appptr_opt =  QOption<QCoreApplication*>::Some(ptr_a);

    qDebug()<<none_opt.unwrap_def(551);

    qDebug()<<dsa_opt.unwrap_def(12345);
    qDebug()<<opt_55.unwrap_or(0,[&](){qDebug()<<"SORRY, IT'S NONE OPTION"; });

    qDebug()<<appptr_opt.unwrap();

    return a.exec();
}

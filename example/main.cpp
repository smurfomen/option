#include <QCoreApplication>
#include <QOption>
#include <QDebug>



QOption<uint> someFileDescriptor(){
#if 1
    return 255;
#else
    return None();
#endif
}

QOption<int> safeDevide(int a, int b) {
    if(b == 0)
        return None();

    return a / b;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /* good usecases */
    {
        auto devided = safeDevide(25,25);
        if(devided)
        {
            qDebug() << devided.unwrap();
        }


        auto errorDevide = safeDevide(25, 0);
        try {
            qDebug() << errorDevide.expect("devide is fail");
        } catch (QUnwrapException & re) {
            qDebug () << "exception:" << re.what();
        }

        {
            auto fd = someFileDescriptor();
            if(fd)
                qDebug() << fd.unwrap();
            else
                qDebug() << "file descriptor not found";
        }

    }


    /* unwrap */
    {
        /* create option */
        /* other case for create: QOption<int> option (255) or auto option = Some(255) */
        QOption<int> option = 255;

        /* print 55 */
        qDebug() << option.unwrap();

        /* reset new value to option and print */
        option = 128;
        if(option) {
            qDebug()<<"explicit bool () operator: digit contains value" << option.unwrap();
        }

        /* with catch exception */
        {
            try {
                qDebug() << option.unwrap();
            }  catch (QUnwrapException &le) {
                qDebug() << "Invalid value in option:" << le.what();
            }

            /* or */
            try{
                qDebug() << option.expect<std::logic_error>("Sorry, option is empty");
            } catch(std::logic_error & re) {
                qDebug() << "Invalid value in option:" << re.what();
            }

            /* or if you want customize message */
            try{
                qDebug() << option.unwrap();
            } catch(...) {
                qDebug() << "Invalid value in option";
            }
        }
    }

    /* unwrap or default value */
    {
        QOption<int> option = None();

        /* print 777 */
        qDebug()<<option.unwrap_def(777);
        if(!option)
            qDebug()<<"bool ! operator: option is None";
    }

    /* unwrap or exec function */
    {
        auto option = Some(255);
        /* print 255 */
        qDebug() << option.unwrap_or([&](){
                qDebug()<<"unwrap_or: option is none, return 0";
                return 0;
        });

        /* print none message, because option already have been unwrapped */
        qDebug() << option.unwrap_or([&](){
                qDebug()<<"unwrap_or: option is none, return 0";
                return 0;
        });
    }

    /* mathing */
    {
        {
            /* option is QOption<int> type value */
            /* other case: QOption<int> option (255) */
            auto option = Some(255);

            /* print 0xff */
            qDebug() << option.match(
            [&](int val){
                return "0x"+QString::number(val, 16);
            },
            [&]{
                return "match: option is None, return this string";
            });

            /* exec none branch, because option already been unwrapped */
            qDebug() << option.match(
            [&](int val){
                return "0x"+QString::number(val, 16);
            },
            [&]{
                return "match: option is None, return this string";
            });
        }


        {
            auto option = Some(&a);
            // or
            // QOption<QCoreApplication*> option = &a;

            // status is QPair<bool, QString> type value
            auto status = option.match([](QCoreApplication * app){
                qDebug()<< app->arguments();
                QString argString;
                const QStringList argsList = app->arguments();
                for(const auto & a : argsList) {
                    argString += a + " ";
                }

                return QPair<bool, QString>(true, argString.trimmed());
            },
            [](){
                return QPair<bool, QString>(false, "match: option is none, return false");
            });

            /* print application args */
            qDebug() << status.second;

            /* commpouse functions */
            /* exec none branch, because option already been unwrapped */
            option.if_none([]{
                qDebug() << "compousing: option already been unwrapped";
            }).if_some([](QCoreApplication * app) {
                qDebug() << app->arguments();
            });
        }
    }

    return 0;
}

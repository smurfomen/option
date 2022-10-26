#include <QCoreApplication>
#include <QOption>
#include <QDebug>



QOption<uint> someFileDescriptor(){
#if 1
	return 3;
#else
    return None();
#endif
}

QOption<int> safeDevide(int a, int b) {
    if(b == 0)
        return None();

    return a / b;
}


#define TEST(x1,x2) Q_ASSERT(x1 == x2)


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /* good usecases */
    {
		TEST(safeDevide(25,25).unwrap(), 1);

		{
			auto errorDevide = safeDevide(25, 0);
			try {
				qDebug() << errorDevide.expect("devide is fail");
			} catch (QUnwrapException & e) { qDebug () << "exception:" << e.what(); }
		}

        {
            auto fd = someFileDescriptor();
            if(fd)
				qDebug() << "file descriptor:" << fd.unwrap();
            else
                qDebug() << "file descriptor not found";
        }

    }


    /* unwrap */
    {
        /* create option */
        /* other case for create: QOption<int> option (255) or auto option = Some(255) */
        QOption<int> option = 255;

		TEST(option.unwrap(), 255);

        /* reset new value to option and print */
        option = 128;
		TEST(option.unwrap(), 128);


        /* with catch exception */
		{
			/* unwrap */
			{
				try {
					qDebug() << option.unwrap();
				}  catch (QUnwrapException &e) { qDebug() << e.what(); }

				/* or */
				try {
					qDebug() << option.unwrap<std::runtime_error>();
				}  catch (std::runtime_error &e) { qDebug() << e.what(); }
			}

			/* expect */
			{
				try{
					qDebug() << option.expect("Sorry, option is empty");
				} catch(QUnwrapException &e) { qDebug() << e.what(); }

				/* or */
				try{
					qDebug() << option.expect<std::logic_error>("Sorry, option is empty");
				} catch(std::logic_error & e) { qDebug() << e.what(); }
			}
		}
	}

    /* unwrap or default value */
    {
        QOption<int> option = None();

		/* print 777 */
		TEST(option.unwrap_def(777), 777);
	}

	/* unwrap or exec function */
	{
		auto option = Some(255);
		/* print 255 */
		TEST(option.unwrap_or([&](){
			qDebug()<<"unwrap_or: option is none, return 0";
			return 0;
		}), 255);


		/* print none message, because option already have been unwrapped */
		TEST(option.unwrap_or([&](){
			qDebug()<<"unwrap_or: option is none, return 0";
			return 0;
		}), 0);
	}

	/* mathing */
	{
		{
			/* option is QOption<int> type value */
			auto option = Some(255);

			/* str == 0xff */
			auto str = option.match(
			[&](int val){
				return "0x"+QString::number(val, 16);
			},
			[&]{
				return "match: option is None, return this string";
			});
			TEST(str, "0xff");

			/* exec none branch, because option already been unwrapped */
			auto strerr = option.match(
			[&](int val){
				return "0x"+QString::number(val, 16);
			},
            [&]{
                return "match: option is None, return this string";
            });

			TEST(strerr, "match: option is None, return this string");
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

			TEST(status.first, true);

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

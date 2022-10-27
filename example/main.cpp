#include <QCoreApplication>
#include <option>
#include <QDebug>
#include <ctime>
#define MOVE_SEM 1


struct A {
	int x;
	A() : x(100) { }
	A(int x) : x(x) { }

	~A() {
		qDebug() << "dtor" << x;
	}

	A(const A & lhs) {
		qDebug() << "copy ctor";
		x = lhs.x;
	}

#if MOVE_SEM == 1
	A(A && lhs) {
		qDebug() << "move ctor";
		std::swap(x, lhs.x);
	}
#endif
};


option<A> foo() {
	return A(1);
}

option<A> bar() {
	A x(2);
	return x;
}

std::array<int, 10000> gl;
option<std::array<int,10000>> check_arr() {
	return gl;
}


option<uint> someFileDescriptor(){
#if 1
	return 3;
#else
    return None();
#endif
}

option<int> safeDevide(int a, int b) {
    if(b == 0)
		return none_option();

    return a / b;
}


#define TEST(x1,x2) Q_ASSERT((x1) == (x2))


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	{
		auto o1 = foo();			// move ctor
		qDebug() << o1.unwrap().x;	// 1

		o1 = bar();					// move ctor
		qDebug() << o1.unwrap().x;	// 2

		o1 = none_option();				// [clear value]

		o1 = some_option(A{3});			// move ctor
		qDebug() << o1.unwrap().x;	// 3

		{
			A c{4};
			o1 = c;						// copy ctor
			qDebug() << o1.unwrap().x;	// 4
		}	// delete not moved c

		A m{5};
		o1 = std::move(m);			// move ctor
		qDebug() << o1.unwrap().x;	// 5
	}


	/* check big data returned using optional type */
	{
		std::srand(std::time(nullptr));
		for(int i = 0; i < 10000; ++i)
		{
			gl[i] = std::rand()%50;
		}

		auto chgl = check_arr().unwrap();

		TEST(chgl == gl, true);
	}


    /* good usecases */
    {
		TEST(safeDevide(25,25).unwrap(), 1);

		{
			auto errorDevide = safeDevide(25, 0);
			try {
				qDebug() << errorDevide.expect("devide is fail");
			} catch (unwrap_exception & e) { qDebug () << "exception:" << e.what(); }
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
		option<int> option = 255;

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
				}  catch (unwrap_exception &e) { qDebug() << e.what(); }

				/* or */
				try {
					qDebug() << option.unwrap<std::runtime_error>();
				}  catch (std::runtime_error &e) { qDebug() << e.what(); }
			}

			/* expect */
			{
				try{
					qDebug() << option.expect("Sorry, option is empty");
				} catch(unwrap_exception &e) { qDebug() << e.what(); }

				/* or */
				try{
					qDebug() << option.expect<std::logic_error>("Sorry, option is empty");
				} catch(std::logic_error & e) { qDebug() << e.what(); }
			}
		}
	}

    /* unwrap or default value */
    {
		option<int> option = none_option();

		/* print 777 */
		TEST(option.unwrap_def(777), 777);
	}

	/* unwrap or exec function */
	{
		auto option = some_option(255);
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
			auto option = some_option(255);

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
			auto option = some_option(&a);
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

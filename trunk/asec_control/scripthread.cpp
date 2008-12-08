/*
 * scripthread.cpp
 *
 *  Created on: 24.10.2008
 *      Author: livid
 */
#include "scriptthread.h"

QScriptValue ScriptThread::call(QScriptContext *context, QScriptEngine *engine)
{
	QString name=context->argument(0).toString();
	QString service=context->argument(1).toString();

	QList<QScriptValue> list;
	for(int i=2;i<context->argumentCount();i++)
	{
		list<<context->argument(i);
	}

	CControlBus* bus=(CControlBus*)(engine->parent());

	bus->is_paused=false;


	bool success=bus->call(name,service,list);

	//while call has not returned without error
	while (!success)
	{
		//pause execution
		bus->is_paused=true;
		do //wait until execution is resumed
		{
			//if user called stopped(), then stop execution
			//or if bus->call returned false because of it
			if(bus->stopped)
				return context->throwError("Stopped by User");
			//don't eat up all the resources
			sleep(1);//second
		} while(bus->is_paused);

		success=bus->call(name,service,list);
	}

	return QScriptValue();
}

void ScriptThread::run()
{
	QString error_msg;
	bool success=true;
	bus = new CControlBus(filename,description,code,&success);
	connect(bus,SIGNAL(bus_error(QString)), this, SIGNAL(error(QString)));
	connect(bus,SIGNAL(call_error(QString)), this, SIGNAL(error(QString)));
	connect(bus,SIGNAL(call_error(QString)), this, SIGNAL(paused()));
	if (success)
	{
		QScriptEngine *engine=new QScriptEngine(bus);

		engine->globalObject().setProperty("call",engine->newFunction(call,2));
		engine->setProcessEventsInterval(100);

		QString init_functions=bus->init_functions(&success);
		if (success)
		{
			if (engine->canEvaluate(init_functions))
			{
				QScriptValue res = engine->evaluate(init_functions);;
				//exception handling
				if(engine->hasUncaughtException())
					emit bug(res.toString(),engine->uncaughtExceptionLineNumber());
				else if (engine->canEvaluate(code))
				{
					QScriptValue res = engine->evaluate(code);
					//exception handling
					if(engine->hasUncaughtException())
						emit bug(res.toString(),engine->uncaughtExceptionLineNumber());
				}
				else
					emit bug("There was a syntax error: code incomplete");
			}
			else
				emit bug("There was a syntax error: init code incomplete");
		}
		delete engine;
	}
	/* For a record, methods are (should be?) automagically
	 * disconnected when sender or receiver is deleted, so
	 * there's no need to explicitly disconnect it here.
	 */
	disconnect(bus,SIGNAL(bus_error(QString)), this, SIGNAL(error(QString)));
	disconnect(bus,SIGNAL(call_error(QString)), this, SIGNAL(error(QString)));
	disconnect(bus,SIGNAL(call_error(QString)), this, SIGNAL(paused()));
	delete bus;
}

/* ATTENTION: ScriptThread::stop() is called from GUI thread,
 * NOT from ScriptThread. Should be VERY careful with this.
 */
void ScriptThread::stop()
{
	bool success=true;
	bus->stop(&success);
}

/* ATTENTION: ScriptThread::resume() is called from GUI thread,
 * NOT from ScriptThread. Should be VERY careful with this.
 */
void ScriptThread::resume()
{
	bus->is_paused=false;
}


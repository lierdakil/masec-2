#include "asec_control.h"

vib_control::vib_control(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	//QErrorMessage::qtHandler()->setModal(true);

	connect(&scriptthread,SIGNAL(started()),this,SLOT(script_started()));
	connect(&scriptthread,SIGNAL(finished()),this,SLOT(script_finished()));
	connect(&scriptthread,SIGNAL(bug(QString,int)),this,SLOT(script_bug(QString,int)));
}

vib_control::~vib_control()
{

}

void vib_control::on_btRun_clicked()
{
	QFileDialog fd(this,
			QString::fromUtf8("Сохранить результат"),
			"",
			QString::fromUtf8("Файл результата эксперимента (*.red);;Все файлы (*.*)"));
	fd.setAcceptMode(QFileDialog::AcceptSave);
	fd.setDirectory("data");
	if(fd.exec())
	{
		scriptthread.code=ui.code->textEdit()->toPlainText();
		scriptthread.filename=fd.selectedFiles().first();
		scriptthread.description=QInputDialog::getText(this,
				QString::fromUtf8("Описание эксперимента"),
				QString::fromUtf8("Краткое описание проводимого эксперимента"));
		scriptthread.start();
	}
}

void vib_control::on_btStop_clicked()
{
	scriptthread.stop();
}


void vib_control::script_started()
{
	ui.btRun->setEnabled(false);
	ui.btStop->setEnabled(true);
	ui.code->setBugLine(-1);
	ui.code->repaint();
}

void vib_control::script_finished()
{
	ui.btStop->setEnabled(false);
	ui.btRun->setEnabled(true);
}

void vib_control::script_bug(QString message, int BugLine)
{
	ui.lbStatus->setText(message);
	ui.lbStatus->setToolTip(message);
	ui.lbStatus->setCursorPosition(0);
	ui.code->setBugLine(BugLine);
	ui.code->repaint();
}

void vib_control::on_btSaveCode_clicked()
{
	QFileDialog fd(0,QString::fromUtf8("Сохранить скрипт"),"",QString::fromUtf8("Скрипт управления (*.fcs)"));
	fd.setAcceptMode(QFileDialog::AcceptSave);
	fd.setDirectory("scripts");
	fd.setDefaultSuffix("fcs");
	if(fd.exec())
	{
		QFile f(fd.selectedFiles().first());
		if(f.open(QIODevice::WriteOnly))
		{
			f.write(ui.code->textEdit()->toPlainText().toUtf8());
			f.close();
		}
	}
}

void vib_control::on_btLoadCode_clicked()
{
	QFileDialog fd(0,QString::fromUtf8("Загрузить скрипт"),"",QString::fromUtf8("Скрипт управления (*.fcs)"));
	fd.setAcceptMode(QFileDialog::AcceptOpen);
	fd.setDirectory("scripts");
	if(fd.exec())
	{
		QFile f(fd.selectedFiles().first());
		if(f.open(QIODevice::ReadOnly))
		{
			ui.code->textEdit()->setPlainText(QString::fromUtf8(f.readAll()));
			f.close();
		}
	}
}

void vib_control::on_tabWidget_currentChanged(int index)
{
	if (index==1)
	{
		ui.cbFunction->clear();
		ui.cbFunction->addItems(CControlBus::build_help_index());
	}
}

void vib_control::on_cbFunction_currentIndexChanged(QString item)
{
	ui.tbHelp->setHtml(CControlBus::get_help(item));
}

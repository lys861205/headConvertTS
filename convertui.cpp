#include "convertui.h"
#include "ui_convertui.h"
#include <QString>
#include "encoding.h"
#include "QTextCodec"
#include "QMessageBox"
#include "Convert2JS.h"
#include "QFileDialog"
#include "qlist.h"
#include "QUrl"
#include "QDebug"
#include "QDragEnterEvent"
#include "QDragMoveEvent"
#include "QDropEvent"
#include "qevent.h"


ConvertUI::ConvertUI(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConvertUI)
{
    ui->setupUi(this);
    setFixedSize(this->width(), this->height());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    this->setWindowTitle(Encoding::ANSIToUTF8("文件转换工具"));
    //this->setWindowTitle(QString("文件转换工具"));
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setAcceptDrops(true);
    //setDragDropMode(QAbstractItemView::DragDrop);
}

ConvertUI::~ConvertUI()
{
    delete ui;
}

void ConvertUI::on_ConvertBtn_clicked()
{
    //QString file_name = QFileDialog::getOpenFileName(this,tr("Open File"), "","*.h|*.txt", 0);
    QString sPath = ui->fileTxt->text();
    if ( sPath.isEmpty())
    {
         QMessageBox::information(this, Encoding::ANSIToUTF8("警告"), Encoding::ANSIToUTF8("路径为空!"));
         return;
    }
    ui->ConvertBtn->setEnabled(false);
    ui->addBtn->setEnabled(false);

    Convert c;
    string stdString = sPath.toStdString();
    int ret = c.toJSFile(stdString);
    if ( ret != error_ok )
    {
        QString strErr = QString("%1%2").arg("error:").arg(ret);
        //QMessageBox::information(this, Encoding::ANSIToUTF8("错误"), strErr);
        QMessageBox::critical(this, Encoding::ANSIToUTF8("错误"), strErr, QMessageBox::Yes);
    }
    else
    {
        QMessageBox::information(this, Encoding::ANSIToUTF8("提示"), Encoding::ANSIToUTF8("转换完成"));
    }
    ui->ConvertBtn->setEnabled(true);
    ui->addBtn->setEnabled(true);
}

void ConvertUI::on_addBtn_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,tr("Open File"), "","*.h", 0);
    ui->fileTxt->setText(file_name);
}


void ConvertUI::dragEnterEvent(QDragEnterEvent *event)
{
     //QMessageBox::information(this, QString("1"),  QString("1"));
    if(event->mimeData()->hasFormat("text/uri-list"))
                   event->acceptProposedAction();
           qDebug() << "drag enter";

}
void ConvertUI::dragMoveEvent(QDragMoveEvent *event)
{
//    QList<QUrl> urls = event->mimeData()->urls();
//    if(urls.isEmpty())
//            return;
//    for ( int i=0; i<urls.count(); ++i )
//    {
//        QUrl url = urls.at(i);
//        QString file_name = url.toLocalFile();
//        qDebug() << file_name;
//    }

    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty()) {
        return;
    }
    ui->fileTxt->setText(fileName);
}
void ConvertUI::dropEvent(QDropEvent *event)
{
    //QMessageBox::information(this, QString("3"),  QString("3"));
}

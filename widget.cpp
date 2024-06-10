#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QDateTime>

using namespace std;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_analyseView = new AnalyseView();
    m_analyseView->hide();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    QString lastPath= "D:/File/Code/qt_projects/Demo/QChartDemo/PlotFrameSelect/";
    QString filePath = QFileDialog::getOpenFileName(this,
                                            QApplication::translate("文件选择窗口", "选择文件"), //title
                                            lastPath, //path
                                            "AWA files (*.AWA)"); //filter

    if(filePath.isEmpty())//如果文件名为空则报错
    {
        return;
    }

    m_analyseView->loadDataFile(filePath);

    updateBtnState();
}

void Widget::on_pushButton_2_clicked()
{
    m_analyseView->setAnalyseDataName(ui->comboBox->currentText(), ui->comboBox_2->currentText());
    m_analyseView->show();
}

void Widget::updateBtnState()
{
    ui->comboBox->clear();
    ui->comboBox_2->clear();

    // 如果不能分析，那就不用管了
    if (!m_analyseView->isAnalyseEnable())
    {
        ui->comboBox->setEnabled(false);
        ui->comboBox_2->setEnabled(false);

        ui->pushButton_2->setEnabled(false);
        return;
    }

    ui->comboBox->setEnabled(m_analyseView->isMostValueEnable());
    ui->comboBox_2->setEnabled(m_analyseView->isAnalyseEnable());

    ui->comboBox->addItems(m_analyseView->instDataNames());
    ui->comboBox_2->addItems(m_analyseView->leqtDataNames());

    ui->pushButton_2->setEnabled(true);
}

#include "operationview.h"
#include "ui_operationview.h"

OperationView::OperationView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OperationView)
{
    ui->setupUi(this);
}

OperationView::~OperationView()
{
    delete ui;
}

void OperationView::setType(int type)
{
    m_type = type;

    switch (m_type)
    {
    case OperationPricison:
        ui->wdg_precise->setVisible(true);
        ui->wdg_peek->setVisible(false);
        ui->wdg_modify->setVisible(false);

        this->setWindowTitle("精确选区");
        break;
    case OperationPeek:
        ui->wdg_precise->setVisible(false);
        ui->wdg_peek->setVisible(true);
        ui->wdg_modify->setVisible(false);

        this->setWindowTitle("下调峰值");
        break;
    case OperationModify:
        ui->wdg_precise->setVisible(false);
        ui->wdg_peek->setVisible(false);
        ui->wdg_modify->setVisible(true);

        this->setWindowTitle("修改数据");
        break;
    }
}

void OperationView::on_btn_cancel_clicked()
{
    this->hide();
}

void OperationView::on_btn_confirm_clicked()
{
    switch (m_type)
    {
    case OperationPricison:
    {
        qint64 startTime = ui->dte_start->dateTime().toMSecsSinceEpoch();
        qint64 endTime = ui->dte_end->dateTime().toMSecsSinceEpoch();

        emit sigPreciseTimeRangeConfirm(startTime, endTime);
    }
        break;
    case OperationPeek:
    {
        int peekValue = ui->le_peek->text().toFloat() * 10;

        emit sigLowerPeekValueConfirm(peekValue);
    }
        break;
    case OperationModify:
    {
        int modifyValue = ui->le_modify->text().toFloat() * 10;

        emit sigModifyValueConfirm(modifyValue);
    }
        break;
    }

    this->hide();
}

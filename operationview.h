#ifndef OPERATIONVIEW_H
#define OPERATIONVIEW_H

#include <QWidget>

namespace Ui {
class OperationView;
}

class OperationView : public QWidget
{
    Q_OBJECT
public:
    enum OperationType_E
    {
        OperationPricison,
        OperationPeek,
        OperationModify
    };

    explicit OperationView(QWidget *parent = nullptr);
    ~OperationView();

    void setType(int type);

signals:
    void sigPreciseTimeRangeConfirm(qint64 start, qint64 end);
    void sigLowerPeekValueConfirm(int value);
    void sigModifyValueConfirm(int value);

private slots:
    void on_btn_cancel_clicked();

    void on_btn_confirm_clicked();

private:
    Ui::OperationView *ui;
    int m_type;
};

#endif // OPERATIONVIEW_H

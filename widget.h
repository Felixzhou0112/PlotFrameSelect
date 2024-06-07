#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "analyseview.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    void updateBtnState();

private:
    Ui::Widget *ui;

    AnalyseView* m_analyseView;// 分析界面
};

#endif // WIDGET_H

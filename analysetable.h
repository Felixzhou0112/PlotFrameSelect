#ifndef SATTABLEWIDGET_H
#define SATTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include "datetime.h"

typedef struct {
   DateTime startTime;  // 开始时间
   DateTime endTime;    // 开始时间
   int      tm;         // 测量持续时间
   int      ts;         // 目标测量时间
   float    leqT;       // 等效连续声级
   float    Lmax;       // 最大声级
   float    Lmin;       // 最小声级
   float    L5;         // 累计百分比声级
   float    L10;        // 累计百分比声级
   float    L50;        // 累计百分比声级
   float    L90;        // 累计百分比声级
   float    L95;        // 累计百分比声级
   float    SD;         // 样本标准差
   float    SEL;        // 声暴露级
}LeqStat_S;

class NoFocusDelegate : public QStyledItemDelegate
{
protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};


class AnalyseTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit AnalyseTable(QWidget *parent = nullptr);

    void updateTableData(int page);
    bool deleteSelectedData();
    void addRow(LeqStat_S &stat);

public slots:
    void slotEChartsDataClicked(qint64 time);

signals:
    void sigShowSpecifiedRowDetails(int row);
    void sigChangeCurrentPage(int page);

private slots:
    void slotSelectCurrentPage(bool checked);
    void slotBtnDetailClicked();
    void slotRowClicked(QTableWidgetItem *item);

private:
    void initTable();
    QWidget* createDetailBtn(int colum);
    void highLightSelectedRow();

private:
    QMap<QString, QCheckBox*> m_checkList;
    QMap<QString, QPushButton*> m_detailList;
    qint64 m_selectedItem;// 被选中的某个数据记录的时间戳

};


#endif // SATTABLEWIDGET_H

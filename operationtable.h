#ifndef OPERATIONTABLE_H
#define OPERATIONTABLE_H

#include <QObject>
#include <QTableWidget>
#include <QPushButton>
#include <QStyledItemDelegate>


class Delegate : public QStyledItemDelegate
{
protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};


class OperationTable : public QTableWidget
{
    Q_OBJECT
public:
    OperationTable(QWidget *parent = nullptr);
    void addRow(QString uid);


signals:
    void sigDeleteTableData(QString uid);
    void sigStoreTableData(QString uid);

private slots:
    void slotDeleteBtnClicked();
    void slotCollectBtnClicked();

private:
    void initTable();
    void destroyBtn(QString uid);

    QList<QPushButton*> m_btnDeleteList; // 删除按钮列表
    QList<QPushButton*> m_btnCollectList;// 收藏按钮列表
};

#endif // OPERATIONTABLE_H

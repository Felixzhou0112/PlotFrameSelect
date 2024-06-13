#include "operationtable.h"
#include <QHeaderView>
#include <QDebug>


void Delegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    if (itemOption.state & QStyle::State_HasFocus)
    {
        itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
    }

    QStyledItemDelegate::paint(painter, itemOption, index);
}

OperationTable::OperationTable(QWidget *parent) : QTableWidget (parent)
{
    initTable();
}

void OperationTable::addRow(QString uid)
{
    // 添加一行数据
    int count = rowCount();
    insertRow(count);

    // 设置数据行
    int rowIndex = rowCount() - 1;
    int colIndex = 0;

    QPushButton* btnDel = new QPushButton("删除");
    btnDel->setObjectName(uid);
    m_btnDeleteList.append(btnDel);
    connect(btnDel, &QPushButton::clicked, this, &OperationTable::slotDeleteBtnClicked);

    QPushButton* btnCol = new QPushButton("收藏");
    btnCol->setObjectName(uid);
    m_btnCollectList.append(btnCol);
    connect(btnCol, &QPushButton::clicked, this, &OperationTable::slotCollectBtnClicked);

    this->setCellWidget(rowIndex, colIndex++, btnDel);
    this->setCellWidget(rowIndex, colIndex++, btnCol);
}

void OperationTable::slotDeleteBtnClicked()
{
    auto btn = sender();
    qDebug() << "delete uid:" << btn->objectName();
}

void OperationTable::slotCollectBtnClicked()
{
    auto btn = sender();
    qDebug() << "delete uid:" << btn->objectName();
}

void OperationTable::initTable()
{
    //表头对象
    QStringList m_header;
    m_header<< QObject::tr("删除")
            << QObject::tr("收藏");

    this->horizontalHeader()->setStyleSheet( "QHeaderView::section {background-color: rgb(244, 242, 243);"
                                             "padding-left: 5px;"
                                             "border-left: 0px solid #000;"
                                             "border-right: 1px solid rgb(190, 190, 190);"
                                             "border-top: 0px solid #000;"
                                             "color: rgb(51, 51, 51);"
                                             "font: 16px;}");

    //设置表格参数
    this->setColumnCount(2);//列数量
    this->setHorizontalHeaderLabels(m_header);//列名称
    this->setAlternatingRowColors(true); // 隔行变色
    this->setPalette(QPalette(QColor(249,249,249)));// 设置隔行变色的颜色  gray灰色
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);// 根据内容自动调整列宽
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);//单元格不可编辑
    this->setSelectionMode(QAbstractItemView::SingleSelection);//设置单行模式
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->verticalHeader()->setVisible(false);//隐藏列表头
    this->setShowGrid(false);//隐藏表格线

    // set the item delegate to your table widget
    this->setItemDelegate(new Delegate());
}

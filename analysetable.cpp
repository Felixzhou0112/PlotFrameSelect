#include "analysetable.h"
#include <QDateTime>
#include <QDebug>
#include <cmath>


#define PAGE_NUM 12

void NoFocusDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    if (itemOption.state & QStyle::State_HasFocus)
    {
        itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
    }

    QStyledItemDelegate::paint(painter, itemOption, index);
}

AnalyseTable::AnalyseTable(QWidget *parent) : QTableWidget (parent)
{
    initTable();
}

void AnalyseTable::updateTableData(int page)
{
    //清空表格数据
    this->setRowCount(0);
    this->clearContents();
    m_checkList.clear();
    m_detailList.clear();

    page = qMax(page, 1);

    int height = 55;

    QFont font;
    font.setFamily("MicroSoft YaHei");
    font.setWeight(QFont::Thin);
    font.setBold(false); // 设置为粗体
    font.setPixelSize(14); // 字体大小为 12

    int num = PAGE_NUM;

    // 添加一些示例数据
    for (int i = 0; i < num; i++)
    {
        int index = (page - 1) * PAGE_NUM + i;
        this->insertRow(i);
//        this->setCellWidget(i, 0, createCheckBox(i + 1));

        // 设置中间的文本列
        QTableWidgetItem* timeItem = new QTableWidgetItem();
        timeItem->setFont(font);
        timeItem->setForeground(QBrush(Qt::black)); // 设置文本颜色为黑色
        timeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        QTableWidgetItem* satItem = new QTableWidgetItem();
        satItem->setFont(font);
        satItem->setForeground(QBrush(Qt::black)); // 设置文本颜色为黑色
        satItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        QTableWidgetItem* retItem = new QTableWidgetItem();
        retItem->setFont(font);
        retItem->setForeground(QBrush(Qt::black)); // 设置文本颜色为黑色
        retItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        this->setItem(i, 1, timeItem);
        this->setItem(i, 2, satItem);
        this->setItem(i, 3, retItem);

        // 最后一列的详情按钮
//        this->setCellWidget(i, 4, createDetailBtn(i));

        // 判断这行是不是选中行，是的话高亮
        highLightSelectedRow();
//        if (m_records.value(index).time == m_selectedItem)
//        {
//            this->selectRow(i);
//        }

        this->setRowHeight(i, height);
    }
}

bool AnalyseTable::deleteSelectedData()
{
    bool isEmpty = true;
    QList<qint64> timeList;

    // 首先找出选中的数据对应的时间戳
    for (auto iter = m_checkList.begin(); iter != m_checkList.end(); iter++)
    {
        if (iter.value()->isChecked())
        {
            int row = iter.key().mid(2).toInt() - 1;
            QString strTime = this->item(row, 1)->text();
            qint64 time = QDateTime::fromString(strTime, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
            timeList.append(time);
            isEmpty = false;
        }
    }

    if (isEmpty)
    {
        return false;
    }
    else
    {
        // 删掉数据库中对应的数据
        for (auto item : timeList)
        {
            QMap<QVariant,QVariant> map;
            map.insert(":Time", item);
//            MyDataBase::GetInstance().searchData(Constant::sqlDeleteDataFromSatInfo, map);
        }

        return true;
    }
}

void AnalyseTable::addRow(LeqStat_S &stat)
{
    // 添加一行数据
    int count = rowCount();
    insertRow(count);

    // 设置数据行
    int rowIndex = rowCount() - 1;
    int colIndex = 0;

    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString("%1").arg(rowIndex + 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString(stat.startTime.datetime().c_str())));
//    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString(stat.endTime.datetime().c_str())));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.tm)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.LeqT, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.Lmax, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.Lmin, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.L5, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.L10, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.L50, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.L90, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(stat.L95, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(floor(stat.SD * 10) / 10, 'f', 1)));
    this->setItem(rowIndex, colIndex++, new QTableWidgetItem(QString::number(floor(stat.SEL * 10) / 10, 'f', 1)));

    for (int j = 0; j < columnCount(); j++)// 列居中
    {
        item(rowIndex, j)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    }
}

void AnalyseTable::slotEChartsDataClicked(qint64 time)
{
    m_selectedItem = time;
    int index = -1;

    // 首先遍历找到这条数据
//    for (auto iter = m_records.begin(); iter != m_records.end(); iter++)
//    {
//        if (iter.value().time == time)
//        {
//            index = iter.key();
//            break;
//        }
//    }

    // 计算这个数据所在的页码
    int page = index / PAGE_NUM;
    int num = index % PAGE_NUM;

    // 如果有余数，说明这条数据在下一页
    if (num > 0)
    {
        page++;
    }

    // 好的，那我们先切到这一页
    emit sigChangeCurrentPage(page);

    // 然后我们再遍历找到这条数据，然后设为选中状态
    highLightSelectedRow();
//    int rows = this->rowCount();

//    for (int i = 0; i < rows; i++)
//    {
//        auto item = this->item(i, 1);
//        auto temp = QDateTime::fromString(item->text(), Constant::timeStrFormat).toSecsSinceEpoch();
//        if (m_selectedItem == temp)
//        {
//            this->selectRow(item->row());
//        }
//    }
}

void AnalyseTable::initTable()
{
    //表头对象
    QStringList m_header;
    m_header<<QObject::tr("序号")
            <<QObject::tr("开始时间")
//            <<QObject::tr("结束时间")
            <<QObject::tr("Tm(s)")
            <<QObject::tr("Leq,T")
            <<QObject::tr("Lmax")
            <<QObject::tr("Lmin")
            <<QObject::tr("L5")
            <<QObject::tr("L10")
            <<QObject::tr("L50")
            <<QObject::tr("L90")
            <<QObject::tr("L95")
            <<QObject::tr("SD")
            <<QObject::tr("SEL");

    this->horizontalHeader()->setStyleSheet( "QHeaderView::section {background-color: rgb(244, 242, 243);"
                                             "padding-left: 5px;"
                                             "border-left: 0px solid #000;"
                                             "border-right: 1px solid rgb(190, 190, 190);"
                                             "border-top: 0px solid #000;"
                                             "color: rgb(51, 51, 51);"
                                             "font: 16px;}");

    //设置表格参数
    this->setColumnCount(m_header.size());//列数量
    this->setHorizontalHeaderLabels(m_header);//列名称
    this->setAlternatingRowColors(true); // 隔行变色
    this->setPalette(QPalette(QColor(249,249,249)));// 设置隔行变色的颜色  gray灰色
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);// 根据内容自动调整列宽
    this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);// 手动调整列宽
    this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);// 手动调整列宽
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);//单元格不可编辑
    this->setSelectionMode(QAbstractItemView::SingleSelection);//设置单行模式
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->verticalHeader()->setVisible(false);//隐藏列表头
    this->setShowGrid(false);//隐藏表格线

    this->setColumnWidth(1, 150);
//    this->setColumnWidth(2, 150);

    // set the item delegate to your table widget
    this->setItemDelegate(new NoFocusDelegate());
}

void AnalyseTable::slotSelectCurrentPage(bool checked)
{
//    auto senderName = sender()->objectName();

    // 判断是否是表头的按钮，是的话就全部勾选或者取消，否则只改一个
//    if (senderName == "cb0")
//    {
        for (auto iter = m_checkList.begin(); iter != m_checkList.end(); iter++)
        {
            iter.value()->setChecked(checked);
        }
//    }
//    else
//    {
//        m_checkList.value(senderName)->setChecked(checked);
        //    }
}

void AnalyseTable::slotBtnDetailClicked()
{
    QPushButton* btn = (QPushButton*)(sender());

    auto name = btn->objectName();
    int colum = name.mid(3).toInt();

    emit sigShowSpecifiedRowDetails(colum);
}

void AnalyseTable::slotRowClicked(QTableWidgetItem *item)
{
    int row = item->row();
    // 先拿到选中行的数据时间戳，并保存
    auto timeItem = this->item(row, 1);
    m_selectedItem = QDateTime::fromString(timeItem->text(), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();

    highLightSelectedRow();
}

QWidget *AnalyseTable::createDetailBtn(int colum)
{
    QWidget* container = new QWidget(this);
//    container->setStyleSheet("background-color:blue;");

    QHBoxLayout* hlayout = new QHBoxLayout(container);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setAlignment(Qt::AlignLeft);
    container->setLayout(hlayout);

    QString name = QString("btn%1").arg(colum);
    QPushButton *detailBtn = new QPushButton("详情", container);
    detailBtn->setCursor(Qt::PointingHandCursor);
    detailBtn->setFixedSize(30, 25);
//    detailBtn->setStyleSheet(style_btn_table_detail);
    detailBtn->setObjectName(name);
    connect(detailBtn, &QPushButton::clicked, this, &AnalyseTable::slotBtnDetailClicked);
    hlayout->addWidget(detailBtn);

    m_detailList.insert(name, detailBtn);

    return container;
}

void AnalyseTable::highLightSelectedRow()
{
    // 首先判断现有的数据中有没有选中行了，因为可能之前选中的被删了
    bool flag = false;
//    for (auto item : m_records)
//    {
//        if (item.time == m_selectedItem)
//        {
//            flag = true;
//        }
//    }

    if (flag)
    {
        // 查看当前页有没有这个数据，有的话就选中
        int rows = this->rowCount();

        for (int i = 0; i < rows; i++)
        {
            auto item = this->item(i, 1);
            auto temp = QDateTime::fromString(item->text(), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
            if (m_selectedItem == temp)
            {
                this->selectRow(item->row());
                emit sigShowSpecifiedRowDetails(item->row());
            }
        }
    }
    else
    {
        // 直接设置第一行选中
        this->selectRow(0);
        emit sigShowSpecifiedRowDetails(0);
    }
}

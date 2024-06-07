#include "analyseplot.h"
#include <QAction>

AnalysePlot::AnalysePlot(QWidget *parent) : QCustomPlot(parent), m_contextMenu(new QMenu(this)), m_selecting(false)
{
    // 初始化曲线
    initPlot();

    // 设置上下文菜单
    setupContextMenu();
    setContextMenuPolicy(Qt::CustomContextMenu);
//    connect(this, &QCustomPlot::customContextMenuRequested, this, &AnalysePlot::contextMenuRequest);
}

void AnalysePlot::addData(const QVector<double>& keys, const QVector<double>& values)
{
    graph(0)->setData(keys, values);
    replot();
}

void AnalysePlot::loadDataFromJson(const QString &jsonData)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8(), &error);

    double yMin = 0;
    double yMax = 0;

    if (error.error == QJsonParseError::NoError)
    {
        if (!(doc.isNull() || doc.isEmpty()))
        {
            if (doc.isObject())
            {
                // 清空数据
                m_keys.clear();
                m_leqValues.clear();

                // 解析数据
                QJsonObject jsonObject = doc.object();
                QJsonArray dataArray = jsonObject["list"].toArray();

                for (const QJsonValue &value : dataArray)
                {
                    QJsonObject obj = value.toObject();

                    // 解析时间戳
                    QString dateTimeStr = obj["dateTime"].toString();
                    QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss:zzz");
                    double time = dateTime.toMSecsSinceEpoch() / 1000.0;

                    // 解析 Leq,T
                    auto valueObj = obj["infoList"].toObject();
                    double leqValue = valueObj[m_leqtName].toString().toDouble();
                    yMin = yMin > leqValue ? leqValue : yMin;// 保存最小值
                    yMax = yMax < leqValue ? leqValue : yMax;// 保存最大值

                    m_keys.append(time);
                    m_leqValues.append(leqValue);
                    m_data.insert(time, valueObj);
                }
            }
            else
            {
                qDebug() << "数据包格式错误";
                return;
            }
        }
        else
        {
            qDebug() << "数据包内容为空";
            return;
        }
    }
    else
    {
        qDebug() << "数据包解析错误" << error.errorString();
        return;
    }

    this->graph(0)->setData(m_keys, m_leqValues);

    this->xAxis->setRange(QCPRange(m_keys.first(), m_keys.last()));
    this->yAxis->setRange(QCPRange(yMin - 10, yMax + 10));

    replot();
}

void AnalysePlot::setInstName(QString name)
{
    m_instName = name;
}

void AnalysePlot::setLeqtNmae(QString name)
{
    m_leqtName = name;
}

void AnalysePlot::setupContextMenu()
{
    preciseAction = new QAction("精确选区", this);         // 精确选区
    lowerPeekAction = new QAction("下调峰值", this);       // 下调峰值
    modifyAction = new QAction("修改数据", this);          // 修改数据
    deleteAction = new QAction("删除数据", this);          // 删除数据
    resetAction = new QAction("还原数据", this);           // 还原数据

    connect(preciseAction, &QAction::triggered, this, &AnalysePlot::preciseActionTriggered);
    connect(lowerPeekAction, &QAction::triggered, this, &AnalysePlot::lowerPeekActionTriggered);
    connect(modifyAction, &QAction::triggered, this, &AnalysePlot::modifyActionTriggered);
    connect(deleteAction, &QAction::triggered, this, &AnalysePlot::deleteActionTriggered);
    connect(resetAction, &QAction::triggered, this, &AnalysePlot::resetActionTriggered);

    m_contextMenu->addAction(preciseAction);
    m_contextMenu->addAction(lowerPeekAction);
    m_contextMenu->addAction(modifyAction);
    m_contextMenu->addAction(deleteAction);
    m_contextMenu->addAction(resetAction);
}

void AnalysePlot::preciseActionTriggered()
{
    qDebug() << "preciseActionTriggered" << m_selectedAreaUid;
}

void AnalysePlot::lowerPeekActionTriggered()
{
    qDebug() << "lowerPeekActionTriggered" << m_selectedAreaUid;
}

void AnalysePlot::modifyActionTriggered()
{
    qDebug() << "modifyActionTriggered" << m_selectedAreaUid;
}

void AnalysePlot::deleteActionTriggered()
{
    qDebug() << "deleteActionTriggered" << m_selectedAreaUid;
}

void AnalysePlot::resetActionTriggered()
{
    qDebug() << "resetActionTriggered" << m_selectedAreaUid;
}

void AnalysePlot::selectAreaBtnToggled(bool checked)
{
    setPlotInteraction(!checked);

    m_selectState = checked;

    // 如果是退出框选，需要清除所有框选区域
    if (!checked)
    {
        setCursor(Qt::CustomCursor);

        // 删除所有矩形选区
        for (auto rect : m_selectionRects)
        {
            removeItem(rect);
        }
        m_selectionRects.clear();
        replot();
    }
    else
    {
        setCursor(Qt::CrossCursor);
    }
}

void AnalysePlot::contextMenuRequest(QPoint pos)
{
    if (m_selectState && !m_selectionRects.isEmpty())
    {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void AnalysePlot::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_selectState)
        {
            m_selecting = true;
            m_selectionStart = xAxis->pixelToCoord(event->pos().x());

            createSelectArea();
        }
    }

    if (event->button() == Qt::RightButton)
    {
        // 将鼠标点击位置从像素坐标转换为图表坐标
        double x = xAxis->pixelToCoord(event->pos().x());
        double y = yAxis->pixelToCoord(event->pos().y());
        QPointF coordPoint(x, y);

        // 遍历选区看看右键有没有在选区中
        for (auto iter = m_selectionRects.begin(); iter != m_selectionRects.end(); iter++)
        {
            QString uid = iter.key();
            QCPItemRect *rect = iter.value();

            QPointF topLeft = rect->topLeft->coords();
            QPointF bottomRight = rect->bottomRight->coords();
            QRectF rectBounds = QRectF(topLeft, bottomRight);
            if (rectBounds.contains(coordPoint))
            {
                m_selectedAreaUid = uid;
                contextMenuRequest(event->pos());
            }
        }
    }

    QCustomPlot::mousePressEvent(event);
}

void AnalysePlot::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selecting && !m_selectionRects.isEmpty())
    {
        m_selectionEnd = xAxis->pixelToCoord(event->pos().x());
        if (m_selectionEnd < m_selectionStart)
        {
            std::swap(m_selectionStart, m_selectionEnd);
        }

        QCPItemRect *currentRect = m_selectionRects.value(m_selectedAreaUid);
        currentRect->topLeft->setCoords(m_selectionStart, yAxis->range().upper);
        currentRect->bottomRight->setCoords(m_selectionEnd, yAxis->range().lower);
        replot();
    }
    QCustomPlot::mouseMoveEvent(event);
}

void AnalysePlot::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_selecting = false;


    }

    if (m_selectState && !m_selectionRects.isEmpty())
    {
        scanSelectedAreaData(m_selectedAreaUid);
    }


    QCustomPlot::mouseReleaseEvent(event);
}

void AnalysePlot::initPlot()
{
    // 创建一个画笔对象，设置线条样式为实线，颜色为黑色，宽度为1
    QPen gridPen;
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(216, 216, 216));
    gridPen.setWidth(1);

    // 坐标轴刻度标签的字体
    QFont xAxisTickLabelFont;
    xAxisTickLabelFont.setFamily("Arial");
    xAxisTickLabelFont.setPixelSize(12);

    // 设置 x 轴相关属性
    m_xTicker = new QCPAxisTickerDateTime();// 创建一个固定刻度的刻度对象
    m_xTicker->setDateTimeFormat("yyyy-MM-dd\nhh:mm:ss:zzz");
    QSharedPointer<QCPAxisTicker> ptrXTicker(m_xTicker);// 创建一个QSharedPointer来管理fixedTicker对象
    this->xAxis->setTicker(ptrXTicker);//设置给坐标轴
    this->xAxis->setTicks(true);//设置刻度显示
    this->xAxis->setTickLabels(true);//设置刻度标签显示
    this->xAxis->setSubTicks(false);//隐藏子刻度
    this->xAxis->grid()->setVisible(false);//隐藏竖着的网格线
    this->xAxis->setBasePen(gridPen);//设置坐标轴的颜色
    this->xAxis->setTickPen(Qt::NoPen);//隐藏刻度
    this->xAxis->setTickLabelFont(xAxisTickLabelFont);//设置刻度标签字体
    this->xAxis->setTickLabelColor(QColor(74, 74, 74));//设置刻度标签颜色

    // 设置 y 轴相关属性
    this->yAxis->setTicks(true);//设置刻度显示
    this->yAxis->setTickLabels(true);//设置刻度标签显示
    this->yAxis->setSubTicks(false);//隐藏子刻度
    this->yAxis->grid()->setPen(gridPen);// 设置横着的网格线样式
    this->yAxis->setBasePen(gridPen);//设置坐标轴的颜色
    this->yAxis->setTickPen(Qt::NoPen);//隐藏刻度
    this->yAxis->setTickLabelFont(xAxisTickLabelFont);//设置刻度标签字体
    this->yAxis->setTickLabelColor(QColor(74, 74, 74));//设置刻度标签颜色

    // 设置 x2 轴相关属性
    this->xAxis2->setVisible(true);//显示上方的 x 轴
    this->xAxis2->setTicks(false);//隐藏刻度
    this->xAxis2->setBasePen(gridPen);//设置坐标轴颜色
    this->xAxis2->setTickPen(Qt::NoPen);//隐藏刻度

    // 设置 y2 轴相关属性
    this->yAxis2->setVisible(true);//显示上方的 y 轴
    this->yAxis2->setTicks(false);//隐藏刻度
    this->yAxis2->setBasePen(gridPen);//设置坐标轴颜色
    this->yAxis2->setTickPen(Qt::NoPen);//隐藏刻度

    //设置基本坐标轴（左侧Y轴和下方X轴）可拖动、可缩放
    setPlotInteraction(true);

    // 添加一条曲线
    addGraph();
    graph(0)->setPen(QPen(QBrush(QColor(194, 54, 50)), 2));

    // 添加一个按钮
    QCheckBox* selectBtn = new QCheckBox(this);
    selectBtn->setGeometry(this->width() - 40, 10, 30, 30);
    connect(selectBtn, &QCheckBox::toggled, this, &AnalysePlot::selectAreaBtnToggled);

    // 初始化鼠标追踪线
    m_crossLine = new CrossLinePlot(this, nullptr);
    m_crossLine->SetLineShow(QCP::E_Vertical);
}

void AnalysePlot::createSelectArea()
{
    QCPItemRect *newRect = new QCPItemRect(this);
    newRect->setPen(QPen(QColor(205, 205, 205, 100)));
    newRect->setBrush(QBrush(QColor(205, 205, 205, 100)));

    newRect->topLeft->setCoords(m_selectionStart, yAxis->range().upper);
    newRect->bottomRight->setCoords(m_selectionStart, yAxis->range().lower);

    m_selectedAreaUid = QUuid::createUuid().toString().replace("{", "").replace("}", "");
    m_selectionRects.insert(m_selectedAreaUid, newRect);
}

void AnalysePlot::setPlotInteraction(bool enable)
{
    // 如果是选中状态，那么需要禁用缩放功能，并且开启框选功能
    setInteraction(QCP::iRangeDrag, enable);
    setInteraction(QCP::iRangeZoom, enable);

    if (enable)
    {
        axisRect()->setRangeDrag(Qt::Horizontal);
        axisRect()->setRangeZoom(Qt::Horizontal);
    }
}

void AnalysePlot::scanSelectedAreaData(const QString& uid)
{
    // 首先拿到这个选区
    QCPItemRect* area = m_selectionRects.value(uid);
    qDebug() << "选区左边" << area->topLeft->coords();
}

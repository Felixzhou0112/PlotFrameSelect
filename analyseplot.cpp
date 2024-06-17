#include "analyseplot.h"
#include <QAction>

AnalysePlot::AnalysePlot(QWidget *parent) : QCustomPlot(parent), m_contextMenu(new QMenu(this)), m_selecting(false)
{
    // 初始化曲线
    initPlot();

    // 设置上下文菜单
    setupContextMenu();
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_operationView = new OperationView();
    connect(m_operationView, &OperationView::sigPreciseTimeRangeConfirm, this, &AnalysePlot::slotPreciseTimeRangeConfirm);
    connect(m_operationView, &OperationView::sigLowerPeekValueConfirm, this, &AnalysePlot::slotLowerPeekValueConfirm);
    connect(m_operationView, &OperationView::sigModifyValueConfirm, this, &AnalysePlot::slotModifyValueConfirm);
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

    m_yMin = 0;
    m_yMax = 0;

    if (error.error == QJsonParseError::NoError)
    {
        if (!(doc.isNull() || doc.isEmpty()))
        {
            if (doc.isObject())
            {
                // 清空数据
                m_keys.clear();
                m_leqValues.clear();
                m_instValues.clear();

                // 解析数据
                QJsonObject jsonObject = doc.object();
                QJsonArray dataArray = jsonObject["list"].toArray();

                for (const QJsonValue &value : dataArray)
                {
                    QJsonObject obj = value.toObject();

                    // 解析开始时间
                    m_startTime = obj["startTime"].toString();

                    // 解析时间戳
                    QString dateTimeStr = obj["dateTime"].toString();
                    QDateTime dateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss:zzz");
                    double time = dateTime.toMSecsSinceEpoch() / 1000.0;

                    // 解析 Leq,T
                    auto valueObj = obj["infoList"].toObject();
                    double leqtValue = valueObj[m_leqtName].toString().toDouble();
                    double instValue = valueObj[m_instName].toString().toDouble();
                    m_yMin = m_yMin > leqtValue ? leqtValue : m_yMin;// 保存最小值
                    m_yMax = m_yMax < leqtValue ? leqtValue : m_yMax;// 保存最大值

                    m_keys.append(time);
                    m_leqValues.append(leqtValue);
                    m_instValues.append(instValue);
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

    // 生成唯一标识
    m_mainAreaUid = QUuid::createUuid().toString().replace("{", "").replace("}", "");

    // 保留一份原始数据
    m_rawKeys = m_keys;
    m_rawValues = m_leqValues;

    // 计算 Tm
    m_tm = m_keys.last() - m_keys.first();

    // 进行二次计算
    emit plotDataLoadFinish();

    // 绘制曲线
    drawPlot();
}

void AnalysePlot::setInstName(QString name)
{
    m_instName = name;
}

void AnalysePlot::setLeqtNmae(QString name)
{
    m_leqtName = name;
}

SelectArea_S *AnalysePlot::currentArea()
{
    return m_currentArea;
}

QString AnalysePlot::mainUid()
{
    return m_mainAreaUid;
}

QString AnalysePlot::startTime()
{
    return m_startTime;
}

int AnalysePlot::tm()
{
    return m_tm;
}

QVector<double> &AnalysePlot::leqtData()
{
    return m_leqValues;
}

QVector<double> &AnalysePlot::instData()
{
    return m_instValues;
}

std::vector<double> AnalysePlot::leqtDataStd()
{
    return std::vector<double>(m_leqValues.begin(), m_leqValues.end());
}

std::vector<double> AnalysePlot::instDataStd()
{
    return std::vector<double>(m_instValues.begin(), m_instValues.end());
}

std::vector<double> AnalysePlot::keyDataStd()
{
    return std::vector<double>(m_rawKeys.begin(), m_rawKeys.end());
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
    m_operationView->setType(OperationView::OperationPricison);
    m_operationView->show();
}

void AnalysePlot::lowerPeekActionTriggered()
{
    qDebug() << "lowerPeekActionTriggered" << m_selectedAreaUid;
    m_operationView->setType(OperationView::OperationPeek);
    m_operationView->show();
}

void AnalysePlot::modifyActionTriggered()
{
    qDebug() << "modifyActionTriggered" << m_selectedAreaUid;
    m_operationView->setType(OperationView::OperationModify);
    m_operationView->show();
}

void AnalysePlot::deleteActionTriggered()
{
    qDebug() << "deleteActionTriggered" << m_selectedAreaUid;

    // 找到选区中的数据并删除
    int index = m_keys.indexOf(m_currentArea->keys.front());
    int length = m_currentArea->keys.size();

    m_keys.remove(index, length);
    m_leqValues.remove(index, length);

    deleteArea(m_currentArea->uid);

    drawPlot();
}

void AnalysePlot::resetActionTriggered()
{
    qDebug() << "resetActionTriggered" << m_selectedAreaUid;

    // 恢复原始数据
    m_keys = m_rawKeys;
    m_leqValues = m_rawValues;

    drawPlot();
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
        for (auto rect : m_areaList)
        {
            removeItem(rect->area);

            delete rect;
            rect = nullptr;
        }
        m_areaList.clear();
        replot();
    }
    else
    {
        setCursor(Qt::CrossCursor);
    }
}

void AnalysePlot::slotPreciseTimeRangeConfirm(qint64 start, qint64 end)
{
    // 先把时间戳转换成横坐标
    m_selectionStart = start / 1000.0;
    m_selectionEnd = end / 1000.0;

    if (checkOverlap(m_selectionStart))
    {
        qDebug() << "选区重复！！";
        return;
    }

    createSelectArea();

    m_currentArea->startPos = m_selectionStart;
    m_currentArea->endPos = m_selectionEnd;

    m_currentArea->area->topLeft->setCoords(m_selectionStart, yAxis->range().upper);
    m_currentArea->area->bottomRight->setCoords(m_selectionEnd, yAxis->range().lower);

    // 如果框选的有效数据点小于 2 个，判断这个选区无效
    checkSelectAreaValid();

    replot();
}

void AnalysePlot::slotLowerPeekValueConfirm(int value)
{
    // 首先给数据恢复精度，需要缩小 10 倍
    float peekValue = value * 0.1;

    // 然后遍历选区中的点，将所有峰值都改为设定值
    for (auto item : m_currentArea->keys)
    {
        if (m_leqValues.value(m_keys.indexOf(item)) > peekValue)
        {
            m_leqValues[m_keys.indexOf(item)] = peekValue;
        }
    }

    // 更新曲线
    drawPlot();
}

void AnalysePlot::slotModifyValueConfirm(int value)
{
    // 首先给数据恢复精度，需要缩小 10 倍
    float peekValue = value * 0.1;

    // 然后遍历选区中的点，将所有峰值都加上设定值
    for (auto item : m_currentArea->keys)
    {
        if (m_leqValues.value(m_keys.indexOf(item)) > peekValue)
        {
            m_leqValues[m_keys.indexOf(item)] += peekValue;
        }
    }

    // 更新曲线
    drawPlot();
}

void AnalysePlot::contextMenuRequest(QPoint pos)
{
    if (m_selectState)
    {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void AnalysePlot::slotShowSpecifiedRowArea(QString uid)
{
    m_selectedAreaUid = uid;

    updateAreasColor();
}

void AnalysePlot::slotDeleteArea(QString uid)
{
    deleteArea(uid);
}

void AnalysePlot::mousePressEvent(QMouseEvent *event)
{
    bool isOverlap = checkOverlap(event);

    if (event->button() == Qt::LeftButton)
    {
        if (m_selectState && !isOverlap)
        {
            m_selecting = true;
            m_selectionStart = xAxis->pixelToCoord(event->pos().x());

            createSelectArea();
        }
    }

    if (event->button() == Qt::RightButton)
    {
        if (isOverlap)
        {
            changeContextMenuState(true);
        }
        else
        {
            changeContextMenuState(false);
        }
        contextMenuRequest(event->pos());
    }

    QCustomPlot::mousePressEvent(event);
}

void AnalysePlot::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selecting && m_currentArea)
    {
        m_selectionEnd = xAxis->pixelToCoord(event->pos().x());
        if (m_selectionEnd < m_selectionStart)
        {
            std::swap(m_selectionStart, m_selectionEnd);
        }

        m_currentArea->startPos = m_selectionStart;
        m_currentArea->endPos = m_selectionEnd;

        m_currentArea->area->topLeft->setCoords(m_selectionStart, yAxis->range().upper);
        m_currentArea->area->bottomRight->setCoords(m_selectionEnd, yAxis->range().lower);
        replot();
    }
    QCustomPlot::mouseMoveEvent(event);
}

void AnalysePlot::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_selecting)
        {
            m_selecting = false;

            // 如果框选的有效数据点小于 2 个，判断这个选区无效
            checkSelectAreaValid();
        }
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
    // 新建一个框选区域
    QCPItemRect *newRect = new QCPItemRect(this);
    newRect->setPen(QPen(m_selectColor));
    newRect->setBrush(QBrush(m_selectColor));

    newRect->topLeft->setCoords(m_selectionStart, yAxis->range().upper);
    newRect->bottomRight->setCoords(m_selectionStart, yAxis->range().lower);

    // 维护起来
    m_currentArea = new SelectArea_S;
    m_currentArea->startTime = m_startTime;
    m_currentArea->tm = m_tm;
    m_currentArea->area = newRect;

    // 生成唯一标识
    m_selectedAreaUid = QUuid::createUuid().toString().replace("{", "").replace("}", "");
    m_currentArea->uid = m_selectedAreaUid;
    m_currentArea->startPos = m_selectionStart;

    updateAreasColor();
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

int AnalysePlot::scanSelectedAreaData(const QString& uid)
{
    Q_UNUSED(uid);

    // 遍历曲线横坐标，找到所有在选区范围内的横坐标
    for (int i = 0; i < m_keys.size(); i++)
    {
        if (m_keys[i] > m_currentArea->endPos)
        {
            break;
        }

        if (m_keys[i] >= m_currentArea->startPos)
        {
            m_currentArea->keys.push_back(m_keys[i]);
            m_currentArea->values.push_back(m_data.value(m_keys[i]).value(m_leqtName).toVariant().toDouble() + 0.0005);

            // 如果有瞬时数据，那么就筛选一下
            if (!m_instName.isEmpty())
            {
                m_currentArea->instData.push_back(m_data.value(m_keys[i]).value(m_instName).toVariant().toDouble() + 0.0005);
            }
        }
    }
    qDebug() << "选中的有效数据点数量为:" << m_currentArea->values.size();
    return m_currentArea->keys.size();
}

void AnalysePlot::changeContextMenuState(bool isArea)
{
    preciseAction->setEnabled(true);         // 精确选区
    lowerPeekAction->setEnabled(isArea);       // 下调峰值
    modifyAction->setEnabled(isArea);          // 修改数据
    deleteAction->setEnabled(isArea);          // 删除数据
    resetAction->setEnabled(true);           // 还原数据
}

void AnalysePlot::drawPlot()
{
    // 调整坐标轴范围
    if (!m_keys.isEmpty())
    {
        this->xAxis->setRange(QCPRange(m_keys.first(), m_keys.last()));
        this->yAxis->setRange(QCPRange(m_yMin - 10, m_yMax + 10));
    }

    this->graph(0)->setData(m_keys, m_leqValues);

    replot();
}

void AnalysePlot::deleteArea(QString uid)
{
    for (int i = 0; i < m_areaList.size(); i++)
    {
        if (m_areaList[i]->uid == uid)
        {
            m_currentArea = m_areaList.takeAt(i);
            removeItem(m_currentArea->area);
            delete m_currentArea;
            m_currentArea = nullptr;
            break;
        }
    }

    replot();
}

bool AnalysePlot::checkOverlap(QMouseEvent *event)
{
    // 将鼠标点击位置从像素坐标转换为图表坐标
    double x = xAxis->pixelToCoord(event->pos().x());
    double y = yAxis->pixelToCoord(event->pos().y());
    QPointF coordPoint(x, y);
    bool isOverlap = false;

    // 遍历选区看看右键有没有在选区中
    for (int i = 0; i < m_areaList.size(); i++)
    {
        QCPItemRect *rect = m_areaList[i]->area;

        QPointF topLeft = rect->topLeft->coords();
        QPointF bottomRight = rect->bottomRight->coords();
        QRectF rectBounds = QRectF(topLeft, bottomRight);
        if (rectBounds.contains(coordPoint))
        {
            m_selectedAreaUid = m_areaList[i]->uid;
            m_currentArea = m_areaList[i];

            // 更新下选区颜色
            updateAreasColor();

            isOverlap = true;
            break;
        }
    }

    return isOverlap;
}

bool AnalysePlot::checkOverlap(double startTime)
{
    bool isOverlap = false;

    // 遍历选区看看右键有没有在选区中
    for (int i = 0; i < m_areaList.size(); i++)
    {
        if (m_areaList[i]->startPos <= startTime && m_areaList[i]->endPos >= startTime)
        {
            isOverlap = true;
            break;
        }
    }

    return isOverlap;
}

void AnalysePlot::updateAreasColor()
{
    // 先找到对应的选区
    for (auto item : m_areaList)
    {
        if (item->uid == m_selectedAreaUid)
        {
            m_currentArea = item;
            m_currentArea->area->setPen(m_selectColor);
            m_currentArea->area->setBrush(m_selectColor);
        }
        else
        {
            item->area->setPen(m_defaultColor);
            item->area->setBrush(m_defaultColor);
        }
    }

    replot();
}

bool AnalysePlot::checkSelectAreaValid()
{
    if (scanSelectedAreaData(m_currentArea->uid) < 2)
    {
        removeItem(m_currentArea->area);

        delete m_currentArea;
        m_currentArea = nullptr;

        return false;
    }
    else
    {
        m_areaList.append(m_currentArea);
        emit selectAreaFinish();

        return true;
    }
}

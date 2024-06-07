/*************************************************
  Copyright (C), 2020-2021, AIHua Tech. Co., Ltd.
  File name:    crosslineplot.h
  Author:       yangchun
  Version:      0.0.01
  Date:         2021-5-6
  Description:  十字线绘制类
  History:
*************************************************/
#include "crosslineplot.h"

std::vector<CrossLinePlot *>CrossLinePlot::m_BrotherLine;

struct CrossLinePlotPrivate
{
    QCP::LineStates m_bIsVisible;
    bool m_bLeftButtonPress = false;
    double m_dAxisXValue = -1;
    int    m_dAxisXIndex = -1;
    QPoint m_MousePoint;
    QCPPainter * m_pPainter = nullptr;
    //QPen m_Pen = QPen(Qt::blue, 1, Qt::DashLine);
    QPen m_Pen = QPen(QColor(74, 144, 226), 1, Qt::DashLine);
};

CrossLinePlot::CrossLinePlot(QCustomPlot *plot, QCPBars *bar)
	: QCPLayerable(plot)
	, d_ptr(new CrossLinePlotPrivate)
{
    mParentPlot->addLayer(LayerName());
    setLayer(LayerName());

    m_dateTime = DateTime();
    m_intervalTime = 100;

    connect(mParentPlot, &QCustomPlot::mousePress, this, [this](QMouseEvent * event){
        if (event->button() & Qt::LeftButton)
        {
            QPoint pos = event->pos();
            if(pos.x() < mParentPlot->xAxis->axisRect()->left())
            {
                return;
            }
            else if(pos.x() > mParentPlot->xAxis->axisRect()->right())
            {
                return;
            }

            d_ptr->m_MousePoint.setX(pos.x());
            if (d_ptr->m_bIsVisible.testFlag(QCP::E_NULL) == false)
            {
                for (CrossLinePlot * crossline : CrossLinePlot::m_BrotherLine)
                {
                    if (crossline != this)
                    {
                        crossline->SyncLinePosition(d_ptr->m_MousePoint, d_ptr->m_dAxisXValue);
                    }
                }

                d_ptr->m_dAxisXIndex = FindNearlyindex(GetDataContainer(), mParentPlot->axisRect(0)->axis(QCPAxis::atBottom)->pixelToCoord(d_ptr->m_MousePoint.x()));
                mParentPlot->layer(LayerName())->replot();
            }

            d_ptr->m_bLeftButtonPress = true;
        }
    });

    connect(mParentPlot, &QCustomPlot::mouseMove, this, [this](QMouseEvent * event){
        QPoint pos = event->pos();
        if(pos.x() < mParentPlot->xAxis->axisRect()->left())
        {
            pos.setX(mParentPlot->xAxis->axisRect()->left());
        }
        else if(pos.x() > mParentPlot->xAxis->axisRect()->right())
        {
            pos.setX(mParentPlot->xAxis->axisRect()->right());
        }

        if(pos.x() ==  d_ptr->m_MousePoint.x())
        {
            return;
        }

        d_ptr->m_MousePoint.setX(pos.x());
        if (d_ptr->m_bIsVisible.testFlag(QCP::E_NULL) == false)
        {
            for (CrossLinePlot * crossline : CrossLinePlot::m_BrotherLine)
            {
                if (crossline != this)
                {
                    crossline->SyncLinePosition(d_ptr->m_MousePoint, d_ptr->m_dAxisXValue);
                }
            }

            d_ptr->m_dAxisXIndex = FindNearlyindex(GetDataContainer(), mParentPlot->axisRect(0)->axis(QCPAxis::atBottom)->pixelToCoord(d_ptr->m_MousePoint.x()));
            mParentPlot->layer(LayerName())->replot();
        }
    });

    connect(mParentPlot, &QCustomPlot::mouseRelease, this, [this](QMouseEvent * event){
            if (event->button() & Qt::LeftButton)
            {
                    d_ptr->m_bLeftButtonPress = false;
            }
    });

//	QVector<qreal> dashes;
//	qreal space = 4;
//	dashes << 3 << space << 9 << space;
//	d_ptr->m_Pen.setDashPattern(dashes);
}

CrossLinePlot::~CrossLinePlot()
{

}

int CrossLinePlot::BinaryFind(const QSharedPointer<QCPGraphDataContainer> &array, int left, int right, double value)
{
    if (left == right)
    {
        return left;
    }

    int mid = (left + right) / 2;
    if (left + 1 == right)
    {
        if (qFabs(array->at(right)->key - value) <= qFabs(value - array->at(left)->key))
        {
            return right;
        }
        else
        {
            return left;
        }
    }

    if (array->at(mid)->key < value)
    {
        return BinaryFind(array, mid, right, value);
    }
    else if (array->at(mid)->key > value)
    {
        return BinaryFind(array, left, mid, value);
    }
    else
    {
        return mid;
    }
}

int CrossLinePlot::BinaryFind(const QVector<double> &array, int left, int right, double value)
{
    if (left == right)
    {
        return left;
    }

    int mid = (left + right) / 2;
    if (left + 1 == right)
    {
        if (qFabs(array[right] - value) <= qFabs(value - array[left]))
        {
            return right;
        }
        else
        {
            return left;
        }
    }

    if (array[mid] < value)
    {
        return BinaryFind(array, mid, right, value);
    }
    else if (array[mid] > value)
    {
        return BinaryFind(array, left, mid, value);
    }
    else
    {
        return mid;
    }
}

QSharedPointer<QCPGraphDataContainer> CrossLinePlot::GetDataContainer()
{
    return mParentPlot->graph(0)->data();
}

int CrossLinePlot::FindNearlyindex(const QSharedPointer<QCPGraphDataContainer> &array, double value)
{
    if (array->size() == 0)
    {
        return -1;
    }

    return BinaryFind(array, 0, array->size() - 1, value);
}

int CrossLinePlot::FindNearlyindex(const QVector<double> &array, double value)
{
    if (array.size() == 0)
    {
        return -1;
    }

    return BinaryFind(array, 0, array.size() - 1, value);
}

QString CrossLinePlot::LayerName() const
{
    return QStringLiteral("crossline");
}

void CrossLinePlot::SetVisible(bool visible)
{
    QCPLayer * layer = mParentPlot->layer(LayerName());
    if (layer)
    {
            layer->setVisible(visible);
    }
}

void CrossLinePlot::SetPen(const QPen &pen)
{
    d_ptr->m_Pen = pen;
}

bool CrossLinePlot::GetLineVisible(QCP::LineState line) const
{
    switch ((int)line)
	{
	case Qt::Horizontal:
		return d_ptr->m_bIsVisible.testFlag(QCP::E_Horizontal);
		break;
	case Qt::Vertical:
		return d_ptr->m_bIsVisible.testFlag(QCP::E_Vertical);
		break;
	}

	return false;
}

void CrossLinePlot::SetLineShow(QCP::LineState lines)
{
    switch (lines)
    {
        case QCP::E_NULL:
                d_ptr->m_bIsVisible = QCP::E_NULL;
                break;
        case QCP::E_Horizontal:
                d_ptr->m_bIsVisible = QCP::E_Horizontal;
                break;
        case QCP::E_Vertical:
                d_ptr->m_bIsVisible = QCP::E_Vertical;
                break;
        case QCP::E_ALL:
                d_ptr->m_bIsVisible = QCP::E_ALL;
                break;
    }

    if (QCPLayer * layer = mParentPlot->layer(LayerName()))
    {
        layer->replot();
    }

    if (d_ptr->m_bIsVisible == QCP::E_NULL)
    {
        for (CrossLinePlot * crossline : CrossLinePlot::m_BrotherLine)
        {
            if (crossline != this)
            {
                    crossline->SyncLinePosition(QPoint(), d_ptr->m_dAxisXValue);
            }
        }
    }
}

void CrossLinePlot::SetIntervalTime(int interval)
{
    m_intervalTime = interval;
}

void CrossLinePlot::SetDateTime(DateTime time)
{
    m_dateTime = time;
}

void CrossLinePlot::draw(QCPPainter *painter)
{
    Q_UNUSED(painter);

    d_ptr->m_pPainter = painter;
    d_ptr->m_pPainter->setPen(d_ptr->m_Pen);
    d_ptr->m_dAxisXIndex = FindNearlyindex(GetDataContainer(), mParentPlot->axisRect(0)->axis(QCPAxis::atBottom)->pixelToCoord(d_ptr->m_MousePoint.x()));

    QSharedPointer< QCPGraphDataContainer > mData = GetDataContainer();
    if(d_ptr->m_dAxisXIndex <= mData.data()->size() && mData.data()->size() > 0)
    {

        if (d_ptr->m_bIsVisible.testFlag(QCP::E_Vertical))// 画x轴
        {
            DrawLine(mParentPlot->xAxis, Qt::Vertical);
        }

        auto axisYValue = *mData.data()->at(d_ptr->m_dAxisXIndex);
        double yPos = mParentPlot->yAxis->coordToPixel(axisYValue.value);
        double xPos = mParentPlot->xAxis->coordToPixel(axisYValue.key);
        d_ptr->m_MousePoint.setY(yPos);
        if (d_ptr->m_bIsVisible.testFlag(QCP::E_Horizontal) &&
            mParentPlot->yAxis->range().contains(axisYValue.value))// 画y轴
        {
            DrawLine(mParentPlot->yAxis, Qt::Horizontal);
        }

        painter->save();

        // 画文本
        painter->setPen(QColor(74, 144, 226));
        painter->setBrush(QBrush(QColor(74, 144, 226))); //设置画刷为背景色
        painter->drawEllipse(QPoint(xPos, yPos),3,3);//画数据点

        int noteX = xPos + 10;
        int noteY = yPos + 10;
        int noteW = 120;
        int noteH = 40;
        if(noteY + noteH > mParentPlot->height())
        {
            noteY = yPos - 10 - noteH;
        }

        if(noteX + noteW > mParentPlot->width())
        {
            noteX = xPos - 10 - noteW;
        }

        // 画背景区域
        painter->setPen(QPen(QColor(255, 255, 255, 125), 1, Qt::SolidLine));
        painter->setBrush(QBrush(QColor(255, 255, 255, 125)));
        painter->drawRect(noteX, noteY, noteW, noteH);

        // 输出坐标信息
        QFont font(QLatin1String("宋体"), 8);
        painter->setFont(font);
        painter->setPen(QColor(0,0,0));

//        DateTime dateTime = m_dateTime + (axisYValue.key + 1) * m_intervalTime / 1000;
        DateTime dateTime = axisYValue.key;
        static QString name = "";
        name = QString("%1").arg(dateTime.datetime().c_str());

        painter->drawText(noteX + 5, noteY + 15, name);// 局部变量，输出文字模糊原因未知
        painter->drawText(noteX + 5, noteY + 30, QString("%1 dB").arg(axisYValue.value));

        painter->restore();
    }

}

void CrossLinePlot::DrawLine(QCPAxis *axis, Qt::Orientation orientation)
{
    if (axis == nullptr)
    {
        return;
    }

    if (d_ptr->m_MousePoint.isNull())
    {
        return;
    }

    if (Qt::Vertical == orientation)// 画垂直线
    {
        double tickPos = axis->coordToPixel(GetDataContainer().data()->at(d_ptr->m_dAxisXIndex)->key);
        d_ptr->m_dAxisXValue = tickPos;

        QLineF verLine(tickPos, axis->axisRect()->rect().bottom(), tickPos, axis->axisRect()->rect().top());
        d_ptr->m_pPainter->drawLine(verLine);
    }
    else// 画水平线
    {
        double top = d_ptr->m_MousePoint.y();

        QLineF horLine(axis->axisRect()->rect().left(), top, axis->axisRect()->rect().right(), top);
        d_ptr->m_pPainter->drawLine(horLine);
    }
}

void CrossLinePlot::SyncLinePosition(const QPoint& pos, double x)
{
    if (pos.isNull())
    {
        d_ptr->m_bIsVisible = QCP::E_NULL;
    }
    else
    {
        d_ptr->m_bIsVisible = QCP::E_Vertical;
    }

    d_ptr->m_MousePoint = pos;
    d_ptr->m_dAxisXValue = x;

    if (QCPLayer * layer = mParentPlot->layer(LayerName()))
    {
        layer->replot();
    }
}

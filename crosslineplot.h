/*************************************************
  Copyright (C), 2020-2021, AIHua Tech. Co., Ltd.
  File name:    crosslineplot.h
  Author:       yangchun
  Version:      0.0.01
  Date:         2021-5-6
  Description:  十字线绘制类
  History:
*************************************************/
#ifndef QCPCROSSLINE_H
#define QCPCROSSLINE_H

#include <QtCore/QString>
#include "qcustomplot.h"
#include "datetime.h"

namespace QCP
{
	enum LineState
	{
        E_NULL = 0x1,// 空
        E_Vertical = 0x2,// 垂直线
        E_Horizontal = 0x4,// 水平线
        E_ALL = E_Vertical + E_Horizontal// 十字线
	};
	Q_DECLARE_FLAGS(LineStates, LineState);
}

struct CrossLinePlotPrivate;

class CrossLinePlot : public  QCPLayerable
{
	Q_OBJECT
signals :
	void DrawCrossLine(const QPoint & pos);

public:
    CrossLinePlot(QCustomPlot *plot, QCPBars *bar);

	~CrossLinePlot();

public:
	QString LayerName() const;
    void SetVisible(bool visible);// 设置层是否绘制
    void SetPen(const QPen &pen);// 画笔设置
    bool GetLineVisible(QCP::LineState line) const;// 显示十字线显示状态
    void SetLineShow(QCP::LineState lines);// 设置十字线显示状态
    void SetDateTime(DateTime time);// 开始时间
    void SetIntervalTime(int interval);// 间隔时间

protected:
    virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const{ Q_UNUSED(painter); };
    virtual void draw(QCPPainter *painter);// 画光标指示层

private:
    void DrawLine(QCPAxis *axis, Qt::Orientation orientation);// 画线
    void SyncLinePosition(const QPoint &pos, double x);// 同步位置
    int  FindNearlyindex(const QSharedPointer<QCPGraphDataContainer> &array, double value);//查找值最接近的位置
    int  FindNearlyindex(const QVector<double> &array, double value);//查找值最接近的位置
    int  BinaryFind(const QSharedPointer<QCPGraphDataContainer> &array, int left, int right, double value);// 二分查找
    int  BinaryFind(const QVector<double> &array, int left, int right, double value);// 二分查找
    QSharedPointer<QCPGraphDataContainer> GetDataContainer();
private:
	QScopedPointer<CrossLinePlotPrivate> d_ptr;
	static std::vector<CrossLinePlot *> m_BrotherLine;
    DateTime m_dateTime;
    QString m_strDateTime;
    int m_intervalTime;
};

#endif // QCPCROSSLINE_H

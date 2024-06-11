#ifndef MYCHART_H
#define MYCHART_H
#include "qcustomplot.h"
#include <QMenu>
#include "crosslineplot.h"


struct SelectArea_S
{
    QString uid;                    // 区域 uid
    QString startTime;              // 测量开始时间
    int tm;                         // 测量持续时间
    double startPos;                // 开始位置
    double endPos;                  // 结束位置
    QCPItemRect * area;             // 框选区域
    std::vector<double> keys;       // 框选区域包含的有效横坐标
    std::vector<double> values;     // 框选区域包含的有效纵坐标
    std::vector<double> instData;   // 框选区域包含的有效瞬时数据
};


class AnalysePlot : public QCustomPlot {
    Q_OBJECT
public:
    explicit AnalysePlot(QWidget *parent = nullptr);
    void addData(const QVector<double>& keys, const QVector<double>& values);
    void loadDataFromJson(const QString &jsonData);
    void setInstName(QString name);
    void setLeqtNmae(QString name);
    SelectArea_S* currentArea();

signals:
    void selectAreaFinish();// 框选结束信号，用来进行二次计算

public slots:
    void contextMenuRequest(QPoint pos);

private slots:
    void setupContextMenu();
    void preciseActionTriggered();
    void lowerPeekActionTriggered();
    void modifyActionTriggered();
    void deleteActionTriggered();
    void resetActionTriggered();
    void selectAreaBtnToggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initPlot();
    void createSelectArea();
    void setPlotInteraction(bool enable);
    int scanSelectedAreaData(const QString &uid);
    void changeContextMenuState(bool isArea);
    void drawPlot();
    void deleteArea(QString uid);
    bool checkOverlap(QMouseEvent *event);// 检查是否重叠

private:
    QMenu *m_contextMenu;                       // 右键菜单
    QList<SelectArea_S*> m_areaList;            // 框选的选区列表
    SelectArea_S* m_currentArea = nullptr;      // 当前操作的选区

    bool m_selectState = false;         // 是否是框选状态
    bool m_selecting;                   // 是否是框选中
    double m_selectionStart;            // 框选区域的开始位置
    double m_selectionEnd;              // 框选区域的结束位置
    QCPAxisTickerDateTime* m_xTicker;   // 曲线的横坐标轴（时间轴）

    QAction *preciseAction;         // 精确选区
    QAction *lowerPeekAction;       // 下调峰值
    QAction *modifyAction;          // 修改数据
    QAction *deleteAction;          // 删除数据
    QAction *resetAction;           // 还原数据

    QString m_instName;             // 瞬时数据字段名
    QString m_leqtName;             // 等效声级数据名称
    QVector<double> m_rawKeys;      // 曲线的横坐标（原始数据）
    QVector<double> m_rawValues;    // 曲线的纵坐标（原始数据）
    QVector<double> m_keys;         // 曲线的横坐标
    QVector<double> m_leqValues;    // 曲线的纵坐标
    QMap<double, QJsonObject> m_data;   // 原始数据
    double m_yMin = 0;
    double m_yMax = 0;

    QString m_startTime;              // 测量开始时间
    int m_tm;                         // 测量持续时间

    QString m_selectedAreaUid = "";     // 右键要操作的选区 uid

    CrossLinePlot* m_crossLine = nullptr;// 鼠标追踪十字线
};

#endif // MYCHART_H

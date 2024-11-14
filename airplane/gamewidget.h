#pragma once

#include <qwidget.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qgraphicsitem.h>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPolygonItem>
#include <QPixmap>
#include <QPolygon>
#include <qpen.h>
#include <qbrush.h>
#include <qlist.h>
#include <qtimer.h>
#include <qcolor.h>
#include <qevent.h>


enum ItemTypes
{
	GItem	   = QGraphicsItem::UserType + 10,
	GAirPlane  = QGraphicsItem::UserType + 11,
	GBullet    = QGraphicsItem::UserType + 12,
	GRocket    = QGraphicsItem::UserType + 13,
	GShip      = QGraphicsItem::UserType + 14
};


class GameItem : public QGraphicsRectItem
{
public:
	GameItem(QGraphicsItem* parent = nullptr) : QGraphicsRectItem(parent)
	{
		ax = 0;
		ay = 0;
	}

	GameItem(const QRectF& rect, QGraphicsItem* parent = nullptr) : QGraphicsRectItem(rect, parent)
	{
		ax = 0;
		ay = 0;
	}

	GameItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem* parent = nullptr) : QGraphicsRectItem(x,y,width, height, parent)
	{
		ax = 0;
		ay = 0;
	}

	// set the displayed pixmap for this element
	void setPixmap(QPixmap px)
	{
		_px = px;
	}

	virtual int type() const override { return GItem; }

	virtual void paint(QPainter* painter,
		               const QStyleOptionGraphicsItem* option,
		               QWidget* widget = nullptr)
	{
		//QGraphicsRectItem::paint(painter, option, widget);
		painter->drawPixmap(boundingRect(), _px, QRectF(0,0, _px.width(), _px.height()));
	}

public:
	int ax;
	int ay;

protected:
	QPixmap _px;
};

class GameAirPlane : public GameItem
{
public:
	GameAirPlane(qreal x, qreal y, qreal width, qreal height, QGraphicsItem* parent = nullptr) : GameItem(x, y, width, height, parent)
	{
		_px.load(":/AirPlane/images/airplane.png");
	}
	int type() { return GAirPlane; }
};

class GameRocket : public GameItem
{
public:
	GameRocket(qreal x, qreal y, qreal width, qreal height, QGraphicsItem* parent = nullptr) : GameItem(x, y, width, height, parent)
	{
		_px.load(":/AirPlane/images/missile.png");
	}
	int type() const override { return GRocket; }
};

class GameShip : public GameItem
{
public:
	GameShip(qreal x, qreal y, qreal width, qreal height, QGraphicsItem* parent = nullptr) : GameItem(x, y, width, height, parent)
	{
		_px.load(":/AirPlane/images/ship.png");
	}
	virtual int type() const override { return GShip; }
};



class GameScene : public QGraphicsScene
{
	Q_OBJECT 
public:
	GameScene(QWidget *parent = NULL) : QGraphicsScene(parent)
	{}
	~GameScene() {}
};

class GameView : public QGraphicsView
{
	Q_OBJECT
public:
	GameView(QWidget* parent = NULL);
	~GameView() {}

	void initScene();

public slots:
	void startGame();

protected slots:
	void gameTimeout();
	void fire();
	void gameOver();

protected:
	void drawForeground(QPainter* painter, const QRectF& rect);
	void resizeEvent(QResizeEvent* e) {}
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

signals:
	void escFromGame();

private:
	GameAirPlane* airplane;
	QList<GameItem*> items;
	QTimer* timer;
	int movex;
	int movey;
	int firemode;
	int frametimeout;
	int screen_frame_move; // y offset of view move up
	int lastfired;
	int screen_y;		// Contains top edge of visible area
	bool gameover;

	QList<QPixmap*> players;
	QList<QPixmap*> nums;
	QPixmap* gameover_px;
	int score;


};


class GameWidget : public QWidget
{
	Q_OBJECT
public:
	GameWidget(QWidget* parent = NULL);
	~GameWidget() {}

public slots:
	void startGame();

protected:
	void resizeEvent(QResizeEvent* e) 
	{
		view->setGeometry(0, 0, e->size().width(), e->size().height());
	}


signals:
	void escFromGame();


private:
	GameScene* scene;
	GameView* view;

};


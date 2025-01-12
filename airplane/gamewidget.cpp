#include "gamewidget.h"

GameWidget::GameWidget(QWidget* parent) : QWidget(parent), scene(nullptr), view(nullptr)
{
	scene = new GameScene(this);
	scene->setSceneRect(0, 0, 1000, 50000);
	
	view = new GameView(this);
	view->setScene(scene);
	QObject::connect(view, SIGNAL(escFromGame()), this, SIGNAL(escFromGame()));

	view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view->initScene();
	view->scale(1, 1);
}

GameView::GameView(QWidget* parent) : QGraphicsView(parent), airplane(nullptr), firemode(0), gameover(false), lastfired(0), movex(0), movey(0), screen_frame_move(5), screen_y(50000), score(0)
{
    timer = new QTimer();
    timer->setSingleShot(false);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(gameTimeout()));

	for (int i = 0; i < 10; i++)
	{
		nums << new QPixmap(":/AirPlane/images/"+QString::number(i) + ".png");
	}
	players << new QPixmap(":/AirPlane/images/player1.png");
	gameover_px = new QPixmap(":/AirPlane/images/gameover.png");
}

void GameWidget::startGame()
{
	view->startGame();
	showFullScreen();
}

void GameView::startGame()
{
	gameover = false;
	movex = 0;
	movey = 0;
	firemode = 0;
	frametimeout = 25;
	screen_frame_move = 5;
	lastfired = 100000;
	screen_y = 50000;
	score = 0;
	initScene();
	timer->start(frametimeout);
}

void GameView::initScene()
{
	scene()->clear();
	items.clear();

	// Create background
	QList<QPoint> left_beach_list;
	QList<QPoint> right_beach_list;
	for (int y = 0; y < 50000; y += 200)
	{
		int x1 = rand() % 230 + 20;
		int x2 = rand() % 250 + 730;

		left_beach_list << QPoint(x1, y);
		right_beach_list << QPoint(x2, y);
	}

	//Creating left beach
	QGraphicsPolygonItem* left_beach = new QGraphicsPolygonItem();
	QPolygonF poly;
	poly << QPointF(0, 0);
	for (int i = 0; i < left_beach_list.count(); i++)
		poly << left_beach_list.at(i);
	poly << QPointF(0, 50000);
	poly << QPointF(0, 0);
	left_beach->setPolygon(poly);
	left_beach->setBrush(QBrush(QColor(Qt::green)));
	scene()->addItem(left_beach);

	// Creating right beach
	QGraphicsPolygonItem* right_beach = new QGraphicsPolygonItem();
	QPolygonF rpoly;
	rpoly << QPointF(1000, 0);
	for (int i = 0; i < right_beach_list.count(); i++)
		rpoly << right_beach_list.at(i);
	rpoly << QPointF(1000, 50000);
	rpoly << QPointF(1000, 0);
	right_beach->setPolygon(rpoly);
	right_beach->setBrush(QBrush(QColor(Qt::green)));
	scene()->addItem(right_beach);

	// Creating the river
	QGraphicsPolygonItem* waterpoly = new QGraphicsPolygonItem();
	QPolygonF wpoly;
	for (int i = 0; i < left_beach_list.count(); i++)
		wpoly << left_beach_list.at(i);
	for (int i = right_beach_list.count() - 1; i >= 0; i--)
		wpoly << right_beach_list.at(i);
	wpoly << left_beach_list.at(0);
	waterpoly->setPolygon(wpoly);
	waterpoly->setBrush(QBrush(QColor(Qt::blue)));
	scene()->addItem(waterpoly);

	// Create user's airplane
	airplane = new GameAirPlane(0, 0, 100, 100);
	scene()->addItem(airplane);
	airplane->setPos(500, 49500);

	// Create enemy boats and airplanes
	for (int i = 0; i < 48000; i += 400)
	{
		GameShip *ship = new GameShip(0, 0, 100, 100);
		ship->setPos(270+rand()%480, i);
		scene()->addItem(ship);
		items.append(ship);

		GameEnemyPlane* eplane = new GameEnemyPlane(0, 0, 100, 100);
		eplane->setPos(rand() % 1000, i+100);
        eplane->ax = rand() % 3 - 2;
		eplane->ay = rand() % 2+3;
		scene()->addItem(eplane);
		items.append(eplane);
	}
}

void GameView::keyPressEvent(QKeyEvent* e)
{
	if (gameover)
	{
		return;
	}

	switch (e->key())
	{
		case Qt::Key_Left:
			movex =-15;
			break;
		case Qt::Key_Right:
			movex=15;
			break;
		case Qt::Key_Up:
			movey=-15;
			break;
		case Qt::Key_Down:
			movey=+15;
			break;
		case Qt::Key_E:
			firemode += 1;
			if (firemode > 3)
				firemode = 1;
			break;
		case Qt::Key_Space:
			fire();
			break;
		default:
			break;
	}
}

void GameView::keyReleaseEvent(QKeyEvent* e)
{
	if (gameover)
	{
		if (e->key()==Qt::Key_Space)
		{ 
			startGame();
		}
		return;
	}

	switch (e->key())
	{
	case Qt::Key_Left:
		movex=0;
		break;
	case Qt::Key_Right:
		movex=0;
		break;
	case Qt::Key_Up:
		movey=0;
		break;
	case Qt::Key_Down:
		movey=0;
		break;
	case Qt::Key_Escape:
		gameOver();
		timer->stop();
		emit escFromGame();
		break;
	default:
		break;
	}
}


void GameView::gameTimeout() // This is frame calculations
{
	if (gameover)
		return;

	screen_y -= screen_frame_move;		   // Bottom viewable screen line
	fitInView(0, screen_y-height(), width(), height());// , Qt::AspectRatioMode::KeepAspectRatio);
    QPoint bl = mapToScene(0, height()).toPoint();
    QPoint tr = mapToScene(width(), 0).toPoint();
	bl.setX(0);
	tr.setX(1000);
	lastfired += frametimeout;

	// Move airplane as user wants 
	int x = airplane->x();
	int y = airplane->y();
	airplane->setPos(x + movex, y + movey);
	int airplane_x = airplane->x();
	int airplane_y = airplane->y();

	// move all items on the scene
	for (int i = 0; i < items.count(); i++)
	{
		int oy = items.at(i)->y();
		if (tr.y()<oy && oy<bl.y())
		{
			int ox = items.at(i)->x(); // current coordinates
			int nx = items.at(i)->ax;  // x, y offsets
			int ny = items.at(i)->ay;
			items.at(i)->setPos(ox + nx, oy + ny);
		}
		else
		{
			if (items.at(i)->type() == GRocket)
			{
					scene()->removeItem(items.at(i));
					items.removeAll(items.at(i));
			}
		}
	}

	// calculatiung margins
	int x_left = bl.x()+50;
	int x_right = tr.x()-50;
	int y_top = tr.y()+50;
	int y_bottom = bl.y()-110;

	// Check and enforce airplane boundaries

    airplane_x = qBound(x_left, airplane_x, x_right);
    airplane_y = qBound(y_top, airplane_y, y_bottom);
	
	airplane->setPos(airplane_x, airplane_y);

	// Collision detection

	// Check for user airplane collision
	QList<QGraphicsItem*> lst = scene()->collidingItems(airplane);
	for (int i = 0; i < lst.count(); i++)
	{
		if (lst.at(i)->type() < GItem)
			continue;
		switch (lst.at(i)->type())
		{
		case GShip:
		case GEnemyPlane:
        case GEnemyRocket:
			gameOver();
			break;
		}
	}

	for (int i = 0; i < items.count(); i++)
	{
		if (items.at(i)->type() == GRocket)
		{
			QList<QGraphicsItem*> lst = scene()->collidingItems(items.at(i));
			for (int j = 0; j < lst.count(); j++)
			{
				if (lst.at(j)->type() == GShip || lst.at(j)->type() == GEnemyPlane)
				{
					scene()->removeItem(lst.at(j));		// remove ship from the scene
					scene()->removeItem(items.at(i));   // remove missile from the scene
					items.removeAll(items.at(i));		// remove missile from our list
					items.removeAll(lst.at(j));		    // remove ship from our list
					score += 200;
				}
			}
		}
	}
}

void GameView::fire()
{
	int mintime;
	switch (firemode)
	{
	case 0:
		mintime = 1000;
		break;
	case 1:
		mintime = 2000;
		break;
	case 2:
		mintime = 3000;
		break;
	default:
		mintime = 1000;
	}

	if (lastfired < mintime)
	{
		return;
	}

	lastfired = 0;

	switch (firemode)
	{
	case 0:
		{
			GameRocket* rocket = new GameRocket(0, 0, 30, 60);
			rocket->setVisible(true);
			scene()->addItem(rocket);
			items.append(rocket);

			QPointF spos = airplane->mapToScene(airplane->boundingRect().width()/2, 0);
			rocket->setPos(spos.x(), spos.y());
			rocket->ax = 0;
			rocket->ay = -30;
		}
		break;
	case 1:
		{
			GameRocket* rocket = new GameRocket(0, 0, 30, 60);
			rocket->setVisible(true);
			scene()->addItem(rocket);
			items.append(rocket);

			QPointF spos = airplane->mapToScene(0, 0);
			rocket->setPos(spos.x() - rocket->boundingRect().width() / 2, spos.y() - rocket->boundingRect().height());
			rocket->ax = -10;
			rocket->ay = -30;
	
			rocket = new GameRocket(0, 0, 30, 60);
			rocket->setVisible(true);
			scene()->addItem(rocket);
			items.append(rocket);

			spos = airplane->mapToScene(airplane->boundingRect().width() , 0);
			rocket->setPos(spos.x() - rocket->boundingRect().width() / 2, spos.y() - rocket->boundingRect().height());
			rocket->ax = 10;
			rocket->ay = -30;
	}

		break;
	case 2:
		break;
	}
}

void GameView::gameOver()
{
	gameover = true;
//	timer->stop();
}

void GameView::drawForeground(QPainter* painter, const QRectF& rect)
{
	// Drawing Player1 from pixmap
	const QRectF target(0, rect.y(), players.at(0)->width(), players.at(0)->height());
	const QRectF source(0, 0, players.at(0)->width(), players.at(0)->height());
	painter->drawPixmap(target, *players.at(0), source);

	// Drawing score
	QString n = QString::number(score);
	int cw = nums.at(0)->width();
	int ch = nums.at(0)->height();
	n = "000000" + n;				
	n = n.mid(n.length() - 6);
	for (int i = 0; i < 6; i++)
	{
		QString w = n.mid(i, 1);
		int index = w.toInt();
		const QRectF trg(1000 - (6 - i) * cw, rect.y(), cw, ch);
		const QRectF src(0, 0, cw, ch);
		painter->drawPixmap(trg, *nums.at(index), src);
	}

	if (gameover)
	{
		cw = gameover_px->width();
		ch = gameover_px->height();
		int y = rect.y() + rect.height() / 2;
		int x = 500 - cw / 2;
		const QRectF trg(x, y, cw, ch);
		const QRectF src(0, 0, cw, ch);
		painter->drawPixmap(trg, *gameover_px, src);
	}
}




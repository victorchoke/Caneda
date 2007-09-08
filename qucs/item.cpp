/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "item.h"
#include "undocommands.h"
#include "schematicscene.h"
#include "schematicview.h"
#include "qucsmainwindow.h"
#include "xmlutilities.h"

#include <QtXml/QXmlStreamWriter>

QucsItem::QucsItem(QGraphicsItem* parent, SchematicScene* scene) : QGraphicsItem(parent,(QGraphicsScene*)scene)
{
}

QucsItem::~QucsItem()
{
}

void QucsItem::copyTo(QucsItem *_item) const
{
   _item->setPenColor(m_penColor);
   _item->setBoundingRect(m_boundingRect);
   _item->setTransform(transform());
   _item->setPos(pos());
   _item->update();
}

void QucsItem::setBoundingRect(const QRectF& rect)
{
   static qreal pw = 1.0;
   prepareGeometryChange();
   m_boundingRect = rect;

   m_boundingRect.adjust(-pw/2, -pw/2, pw, pw);
   update();
}

SchematicScene* QucsItem::schematicScene() const
{
   SchematicScene *s = qobject_cast<SchematicScene*>(this->scene());
   return s;
}

QGraphicsView* QucsItem::activeView() const
{
   if(!scene() || scene()->views().isEmpty())
      return 0;
   return scene()->views()[0];
}

QucsMainWindow* QucsItem::mainWindow() const
{
   QGraphicsView *view = activeView();
   if(!view) return 0;

   QucsMainWindow *mw = qobject_cast<QucsMainWindow*>(view->parent());
   return mw;
}

void QucsItem::setPenColor(QColor color)
{
   m_penColor = color;
}

void QucsItem::writeXml(Qucs::XmlWriter *writer)
{
   Q_UNUSED(writer);
}

void QucsItem::readXml(Qucs::XmlReader *reader)
{
   if(reader->isStartElement())
      reader->readUnknownElement();
}

void QucsItem::mirrorX()
{
   update();
   scale(1.0,-1.0);
}

void QucsItem::mirrorY()
{
   update();
   scale(-1.0,1.0);
}

void QucsItem::rotate()
{
   update();
   rotate(-90.0);
}

QMenu* QucsItem::defaultContextMenu() const
{
   //TODO:
   return 0;
}
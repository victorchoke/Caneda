/***************************************************************************
                          msvia.cpp  -  description
                             -------------------
    begin                : Sat Oct 30 2004
    copyright            : (C) 2004 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "msvia.h"


MSvia::MSvia()
{
  Description = QObject::tr("microstrip via");

  Arcs.append(new Arc(-5,-4, 10, 8, 0, 16*360,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-20,  0, -5,  0,QPen(QPen::darkBlue,2)));
  Lines.append(new Line( -5,  0, -5, 14,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(  5,  0,  5, 14,QPen(QPen::darkBlue,2)));
  Lines.append(new Line(-11, 14, 11, 14,QPen(QPen::darkBlue,3)));
  Lines.append(new Line( -7, 20,  7, 20,QPen(QPen::darkBlue,3)));
  Lines.append(new Line( -3, 26,  3, 26,QPen(QPen::darkBlue,3)));

  Ports.append(new Port(-20,  0));

  x1 = -20; y1 = -7;
  x2 =  14; y2 = 30;

  tx = 20;
  ty = 0;
  Model = "MVIA";
  Name  = "MS";

  Props.append(new Property("Subst", "Subst1", true,
		QObject::tr("substrate")));
  Props.append(new Property("D", "1 mm", true,
		QObject::tr("diameter of round via conductor")));
}

MSvia::~MSvia()
{
}

Component* MSvia::newOne()
{
  return new MSvia();
}

Component* MSvia::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("Microstrip Via");
  BitmapFile = "msvia";

  if(getNewOne)  return new MSvia();
  return 0;
}
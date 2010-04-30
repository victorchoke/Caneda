/***************************************************************************
 * Copyright (C) 2007 by Gopala Krishna A <krishna.ggk@gmail.com>          *
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

#include "canedaview.h"

#include <QFileInfo>

namespace Caneda
{
    // icon for unsaved files (diskette)
    const char *smallsave_xpm[] = {
        "16 17 66 1", " 	c None",
        ".	c #595963","+	c #E6E6F1","@	c #465460","#	c #FEFEFF",
        "$	c #DEDEEE","%	c #43535F","&	c #D1D1E6","*	c #5E5E66",
        "=	c #FFFFFF","-	c #C5C5DF",";	c #FCF8F9",">	c #BDBDDA",
        ",	c #BFBFDC","'	c #C4C4DF",")	c #FBF7F7","!	c #D6D6E9",
        "~	c #CBCBE3","{	c #B5B5D6","]	c #BCBCDA","^	c #C6C6E0",
        "/	c #CFCFE5","(	c #CEC9DC","_	c #D8D8EA",":	c #DADAEB",
        "<	c #313134","[	c #807FB3","}	c #AEAED1","|	c #B7B7D7",
        "1	c #E2E2EF","2	c #9393C0","3	c #E3E3F0","4	c #DDD5E1",
        "5	c #E8E8F3","6	c #2F2F31","7	c #7B7BAF","8	c #8383B5",
        "9	c #151518","0	c #000000","a	c #C0C0DC","b	c #8E8FBD",
        "c	c #8989BA","d	c #E7EEF6","e	c #282829","f	c #6867A1",
        "g	c #7373A9","h	c #A7A7CD","i	c #8080B3","j	c #7B7CB0",
        "k	c #7070A8","l	c #6D6DA5","m	c #6E6EA6","n	c #6969A2",
        "o	c #7A79AF","p	c #DCDCEC","q	c #60609A","r	c #7777AC",
        "s	c #5D5D98","t	c #7676AB","u	c #484785","v	c #575793",
        "w	c #50506A","x	c #8787B8","y	c #53536E","z	c #07070E",
        "A	c #666688",
        "        .       ",
        "       .+.      ",
        "      .+@#.     ",
        "     .$%###.    ",
        "    .&*####=.   ",
        "   .-.#;#####.  ",
        "  .>,'.#)!!!!~. ",
        " .{].'^./(!_:<[.",
        ".}|.1./2.3456789",
        "0a.$11.bc.defg9 ",
        " 011h11.ij9kl9  ",
        "  0_1h1h.mno9   ",
        "   0p12h9qr9    ",
        "    0hh9st9     ",
        "     09uv9w     ",
        "      0x9y      ",
        "       zA       "
    };

    const char *empty_xpm[] = {  // provides same height than "smallsave_xpm"
        "1 17 1 1", "  c None", " ", " ", " ", " ", " ",
        " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "
    };

    //! Constructor
    CanedaView::CanedaView()
    {
    }

    bool CanedaView::load(const QString& fileName)
    {
        setFileName(fileName);
        return load();
    }

    SchematicWidget* CanedaView::toSchematicWidget() const
    {
        return 0;
    }

    QString CanedaView::tabText() const
    {
        QFileInfo info(fileName());
        QString name = info.fileName();
        if(name.isEmpty()) {
            return QObject::tr("Untitled");
        }
        return name;
    }

    QIcon CanedaView::modifiedTabIcon()
    {
        static QIcon modifiedIcon = QIcon(smallsave_xpm);
        return modifiedIcon;
    }

    QIcon CanedaView::unmodifiedTabIcon()
    {
        static QIcon unmodifiedIcon = QIcon(empty_xpm);
        return unmodifiedIcon;
    }

} // namespace Caneda

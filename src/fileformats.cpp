/***************************************************************************
 * Copyright (C) 2006 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 * Copyright (C) 2009-2015 by Pablo Daniel Pareja Obregon                  *
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

#include "fileformats.h"

#include "cgraphicsscene.h"
#include "component.h"
#include "global.h"
#include "idocument.h"
#include "library.h"
#include "port.h"
#include "portsymbol.h"
#include "wire.h"
#include "xmlutilities.h"

#include "paintings/painting.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>

namespace Caneda
{
    /*************************************************************************
     *                         FormatXmlSchematic                            *
     *************************************************************************/
    //! \brief Constructor.
    FormatXmlSchematic::FormatXmlSchematic(SchematicDocument *doc):
        m_schematicDocument(doc)
    {
    }

    bool FormatXmlSchematic::save()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug() << "Looks buggy! Null data to save! Was this expected?";
        }

        QFile file(fileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save document!"));
            return false;
        }
        QTextStream stream(&file);
        stream << text;
        file.close();

        return true;
    }

    bool FormatXmlSchematic::load()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ")+fileName());
            return false;
        }

        QTextStream stream(&file);

        bool result = loadFromText(stream.readAll());

        file.close();
        return result;
    }

    QString FormatXmlSchematic::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        //Fist we start the document and write current version
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        //Now we copy all the elements and properties in the schematic
        saveSchematics(writer);

        //Finally we finish the document
        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    void FormatXmlSchematic::saveSchematics(Caneda::XmlWriter *writer)
    {
        saveComponents(writer);
        savePorts(writer);
        saveWires(writer);
        savePaintings(writer);
        saveProperties(writer);
    }

    void FormatXmlSchematic::saveComponents(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Component*> components = filterItems<Component>(items);
        if(!components.isEmpty()) {
            writer->writeStartElement("components");
            foreach(Component *c, components) {
                c->saveData(writer);
            }
            writer->writeEndElement(); //</components>
        }
    }

    void FormatXmlSchematic::savePorts(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);
        if(!portSymbols.isEmpty()) {
            writer->writeStartElement("ports");
            foreach(PortSymbol *p, portSymbols) {
                p->saveData(writer);
            }
            writer->writeEndElement(); //</ports>
        }
    }

    void FormatXmlSchematic::saveWires(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Wire*> wires = filterItems<Wire>(items);
        if(!wires.isEmpty()) {
            writer->writeStartElement("wires");
            foreach(Wire *w, wires) {
                w->saveData(writer);
            }
            writer->writeEndElement(); //</wires>
        }
    }

    void FormatXmlSchematic::savePaintings(Caneda::XmlWriter *writer)
    {
        QList<QGraphicsItem*> items = cGraphicsScene()->items();
        QList<Painting*> paintings = filterItems<Painting>(items);
        if(!paintings.isEmpty()) {
            writer->writeStartElement("paintings");
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
            writer->writeEndElement(); //</paintings>
        }
    }

    /*!
     * \brief Save scene properties to an XmlWriter.
     *
     * This method saves all scene related propeties to an XmlWriter. To do so,
     * it takes each property from the PropertyGroup of the scene, and saves
     * the data using the Property::saveProperty() method.
     *
     * \param writer XmlWriter to save properties into.
     * \sa Property::saveProperty()
     */
    void FormatXmlSchematic::saveProperties(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();

        writer->writeStartElement("properties");

        // Save the properties position in the scene
        PropertyGroup *properties = scene->properties();
        writer->writePointAttribute(properties->pos(), "pos");

        // Save every individual property of the scene
        foreach(Property property, properties->propertyMap()) {
            property.saveProperty(writer);
        }

        writer->writeEndElement(); //</properties>
    }

    bool FormatXmlSchematic::loadFromText(const QString& text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isStartElement()) {
                if(reader->name() == "caneda" &&
                        Caneda::checkVersion(reader->attributes().value("version").toString())) {

                    while(!reader->atEnd()) {
                        reader->readNext();
                        if(reader->isEndElement()) {
                            Q_ASSERT(reader->name() == "caneda");
                            break;
                        }
                        loadSchematics(reader);
                    }
                }
                else {
                    reader->raiseError(QObject::tr("Not a caneda file or probably malformatted file"));
                }
            }
        }

        if(reader->hasError()) {
            QMessageBox::critical(0, QObject::tr("Xml parse error"), reader->errorString());
            delete reader;
            return false;
        }

        delete reader;
        return true;
    }

    void FormatXmlSchematic::loadSchematics(Caneda::XmlReader* reader)
    {
        if(reader->isStartElement()) {
            if(reader->name() == "components") {
                loadComponents(reader);
            }
            else if(reader->name() == "ports") {
                loadPorts(reader);
            }
            else if(reader->name() == "wires") {
                loadWires(reader);
            }
            else if(reader->name() == "paintings") {
                loadPaintings(reader);
            }
            else if(reader->name() == "properties") {
                loadProperties(reader);
            }
            else {
                reader->readUnknownElement();
            }
        }
    }

    void FormatXmlSchematic::loadComponents(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!reader->isStartElement() || reader->name() != "components") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "components");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "component") {
                    Component *c = Component::loadComponent(reader, scene);
                    c->checkAndConnect(Caneda::DontPushUndoCmd);
                }
                else {
                    qWarning() << "Error: Found unknown component type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    void FormatXmlSchematic::loadPorts(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!reader->isStartElement() || reader->name() != "ports") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "ports");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "port") {
                    PortSymbol *portSymbol = new PortSymbol(scene);
                    portSymbol->loadData(reader);
                }
                else {
                    qWarning() << "Error: Found unknown port type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    void FormatXmlSchematic::loadWires(Caneda::XmlReader* reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!reader->isStartElement() || reader->name() != "wires") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "wires");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "wire") {
                    Wire *w = Wire::loadWire(reader,scene);
                    w->checkAndConnect(Caneda::DontPushUndoCmd);
                }
                else {
                    qWarning() << "Error: Found unknown wire type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    void FormatXmlSchematic::loadPaintings(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!reader->isStartElement() || reader->name() != "paintings") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "paintings");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "painting") {
                    Painting::loadPainting(reader,scene);
                }
                else {
                    qWarning() << "Error: Found unknown painting type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    void FormatXmlSchematic::loadProperties(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();

        // Read and set the properties position in the scene
        PropertyGroup *properties = scene->properties();
        properties->setPos(reader->readPointAttribute("pos"));

        // Read every individual property and add it to the scene
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "properties");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    Property prop = Property::loadProperty(reader);
                    scene->addProperty(prop);
                }
                else {
                    qWarning() << "Error: Found unknown property type" << reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    SchematicDocument* FormatXmlSchematic::schematicDocument() const
    {
        return m_schematicDocument;
    }

    CGraphicsScene* FormatXmlSchematic::cGraphicsScene() const
    {
        return m_schematicDocument ? m_schematicDocument->cGraphicsScene() : 0;
    }

    QString FormatXmlSchematic::fileName() const
    {
        return m_schematicDocument ? m_schematicDocument->fileName() : QString();
    }


    /*************************************************************************
     *                           FormatXmlSymbol                             *
     *************************************************************************/
    //! \brief Constructor.
    FormatXmlSymbol::FormatXmlSymbol(SymbolDocument *doc) :
        m_symbolDocument(doc)
    {
        if(m_symbolDocument) {
            m_fileName = m_symbolDocument->fileName();
        }
        else {
            m_fileName = QString();
        }

        m_component = 0;
    }

    //! \brief Constructor.
    FormatXmlSymbol::FormatXmlSymbol(ComponentData *component) :
        m_component(component)
    {
        if(m_component) {
            m_fileName = m_component->filename;
        }
        else {
            m_fileName = QString();
        }

        m_symbolDocument = 0;
    }

    bool FormatXmlSymbol::save()
    {
        if(!cGraphicsScene()) {
            return false;
        }

        QString text = saveText();
        if(text.isEmpty()) {
            qDebug() << "Looks buggy! Null data to save! Was this expected?";
        }

        QFile file(fileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save document!"));
            return false;
        }

        QTextStream stream(&file);
        stream << text;
        file.close();

        return true;
    }

    //! \brief Parses the component data from file \a path.
    bool FormatXmlSymbol::load()
    {
        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot open file %1").arg(fileName()));
            return false;
        }

        QTextStream stream(&file);
        bool result = loadFromText(stream.readAll());
        file.close();

        return result;
    }

    SymbolDocument* FormatXmlSymbol::symbolDocument() const
    {
        return m_symbolDocument;
    }

    CGraphicsScene* FormatXmlSymbol::cGraphicsScene() const
    {
        return m_symbolDocument ? m_symbolDocument->cGraphicsScene() : 0;
    }

    ComponentData* FormatXmlSymbol::component() const
    {
        return m_component;
    }

    QString FormatXmlSymbol::fileName() const
    {
        return m_fileName;
    }

    QString FormatXmlSymbol::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        // Fist we start the document
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));

        // Write all view details
        writer->writeStartElement("component");
        QFileInfo info(fileName());
        writer->writeAttribute("name", info.baseName());
        writer->writeAttribute("version", Caneda::version());
        writer->writeAttribute("label", "X");

        writer->writeStartElement("displaytext");
        writer->writeLocaleText("C", "User created component");
        /*!
         * \todo When available use this to save user defined displaytext
         * writer->writeLocaleText("C", scene->displayText());
         */
        writer->writeEndElement(); //</displaytext>

        writer->writeStartElement("description");
        writer->writeLocaleText("C", "User created component based on user symbol");
        /*!
         * \todo When available use this to save user defined description
         * writer->writeLocaleText("C", scene->description());
         */
        writer->writeEndElement(); //</description>

        // Write symbol geometry (drawing), ports, properties and models
        saveSymbol(writer);
        savePorts(writer);
        saveProperties(writer);
        saveModels(writer);

        // Finally we finish the document
        writer->writeEndDocument(); //</component>

        delete writer;
        return retVal;
    }

    void FormatXmlSymbol::saveSymbol(XmlWriter *writer)
    {
        writer->writeStartElement("symbol");

        CGraphicsScene *scene = cGraphicsScene();
        QList<QGraphicsItem*> items = scene->items();
        QList<Painting*> paintings = filterItems<Painting>(items);
        if(!paintings.isEmpty()) {
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
        }

        writer->writeEndElement(); //</symbol>
    }

    void FormatXmlSymbol::savePorts(XmlWriter *writer)
    {
        writer->writeStartElement("ports");

        CGraphicsScene *scene = cGraphicsScene();
        QList<QGraphicsItem*> items = scene->items();
        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);
        if(!portSymbols.isEmpty()) {
            foreach(PortSymbol *p, portSymbols) {
                p->saveData(writer);
            }
        }

        writer->writeEndElement(); //</ports>
    }

    void FormatXmlSymbol::saveProperties(XmlWriter *writer)
    {
        writer->writeStartElement("properties");

        CGraphicsScene *scene = cGraphicsScene();
        PropertyGroup *properties = scene->properties();
        writer->writePointAttribute(properties->pos(), "pos");
        foreach(Property property, properties->propertyMap()) {
            property.saveProperty(writer);
        }

        writer->writeEndElement(); //</properties>
    }

    /*!
     * \brief Saves spice model data to xml file text.
     *
     * When editing a schematic's symbol, the spice model should be
     * automatically generated from the symbol properties. Generally
     * speaking, the model should have the following syntax:
     *
     * \code
     * X%label %port{1} %port{2} ... %port{n} modelname property_1=%property{property_1} property_2=%property{property_2} ... property_m=%property{property_m}
     * %subcircuit{modelname  %port{1} %port{2} ... %port{n} property_1=0 property_2=0 ... property_m=0
     * %n.include %librarypath/modelname.net}
     * %generateNetlist
     * \endcode
     *
     * XmlWriter doesn't allow including new lines, so the resulting
     * syntax will be appended to the writer in only one line. This,
     * however, doesn't affect the results.
     *
     * \param reader XmlWriter responsible for writing xml data.
     */
    void FormatXmlSymbol::saveModels(XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
        QList<QGraphicsItem*> items = scene->items();
        PropertyGroup *properties = scene->properties();
        QFileInfo info(fileName());

        // Generate the spice model syntax
        QString syntax = "X%label";

        QList<PortSymbol*> portSymbols = filterItems<PortSymbol>(items);
        if(!portSymbols.isEmpty()) {
            foreach(PortSymbol *p, portSymbols) {
                syntax.append(" %port{" + p->label() + "}");
            }
        }

        syntax.append(" " + info.baseName());

        foreach(Property property, properties->propertyMap()) {
            syntax.append(" " + property.name() + "=%property{" + property.name() + "}");
        }

        syntax.append(" %subcircuit{" + info.baseName());
        if(!portSymbols.isEmpty()) {
            foreach(PortSymbol *p, portSymbols) {
                syntax.append(" " + p->label());
            }
        }

        foreach(Property property, properties->propertyMap()) {
            syntax.append(" " + property.name() + "=0");
        }

        syntax.append(" %n.include %librarypath/" + info.baseName() + ".net}");
        syntax.append(" %generateNetlist");

        // Write the result to the XmlWriter
        writer->writeStartElement("models");

        writer->writeEmptyElement("model");
        writer->writeAttribute("type", "spice");
        writer->writeAttribute("syntax", syntax);

        writer->writeEndElement(); // </models>
    }

    /*!
     * \brief Reads symbol data from xml file text.
     *
     * \param text String containing xml data from component/symbol file.
     */
    bool FormatXmlSymbol::loadFromText(const QString &text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());

        while(!reader->atEnd()) {
            reader->readNext();
            if(reader->isStartElement() && reader->name() == "component") {
                break;
            }
        }

        if(reader->isStartElement() && reader->name() == "component") {

            // Check version compatibility, get name and label
            QXmlStreamAttributes attributes = reader->attributes();
            Q_ASSERT(Caneda::checkVersion(attributes.value("version").toString()));

            // Check if we are opening the file for edition or to include it in a library
            if(cGraphicsScene()) {
                // We are opening the file for symbol edition
                //! \todo Implement this.
            }
            else if(component()) {
                // We are opening the file as a component to include it in a library
                component()->name = attributes.value("name").toString();
                component()->labelPrefix = attributes.value("label").toString();
            }


            // Read the component body
            while(!reader->atEnd()) {
                reader->readNext();

                if(reader->isEndElement()) {
                    break;
                }

                if(reader->isStartElement()) {

                    // Read display text
                    if(reader->name() == "displaytext") {
                        // Check if we are opening the file for edition or to include it in a library
                        if(cGraphicsScene()) {
                            // We are opening the file for symbol edition
                            //! \todo Implement this.
                            reader->readUnknownElement();
                        }
                        else if(component()) {
                            // We are opening the file as a component to include it in a library
                            component()->displayText = reader->readLocaleText(Caneda::localePrefix());
                        }
                    }


                    // Read description
                    else if(reader->name() == "description") {
                        // Check if we are opening the file for edition or to include it in a library
                        if(cGraphicsScene()) {
                            // We are opening the file for symbol edition
                            //! \todo Implement this.
                            reader->readUnknownElement();
                        }
                        else if(component()) {
                            // We are opening the file as a component to include it in a library
                            component()->description = reader->readLocaleText(Caneda::localePrefix());
                        }
                    }

                    // Read symbol
                    else if(reader->name() == "symbol") {
                        readSymbol(reader);
                    }

                    // Read ports
                    else if(reader->name() == "ports") {
                        readPorts(reader);
                    }

                    // Read properties
                    else if(reader->name() == "properties") {
                        readProperties(reader);
                    }

                    // Read models
                    else if(reader->name() == "models") {
                        readModels(reader);
                    }
                }

            }
        }

        if(reader->hasError()) {
            qWarning() << "\nWarning: Failed to read data from\n" << fileName();
            QMessageBox::critical(0, QObject::tr("Xml parse error"), reader->errorString());
            delete reader;
            return false;
        }

        delete reader;
        return true;
    }

    /*!
     * \brief Read symbol section of component description xml file
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::readSymbol(Caneda::XmlReader *reader)
    {
        QPainterPath data;

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "painting") {

                // Check if we are opening the file for edition or to include it in a library
                if(cGraphicsScene()) {
                    // We are opening the file for symbol edition
                    Painting::loadPainting(reader, cGraphicsScene());
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    Painting *newSymbol = Painting::loadPainting(reader);
                    QRectF rect = newSymbol->paintingRect();
                    rect.moveTo(newSymbol->pos());
                    data.addPath(newSymbol->shapeForRect(rect));
                }

            }
        }


        // If we are opening the file as a component, register the recreated QPainterPath
        if(component()) {
            LibraryManager *libraryManager = LibraryManager::instance();
            libraryManager->registerComponent(component()->name, component()->library, data);
        }
    }

    /*!
     * \brief Reads the ports data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::readPorts(Caneda::XmlReader *reader)
    {
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "port") {
                // Check if we are opening the file for edition or to include it in a library
                if(cGraphicsScene()) {
                    // We are opening the file for symbol edition
                    PortSymbol *portSymbol = new PortSymbol(cGraphicsScene());
                    portSymbol->loadData(reader);
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    QPointF pos = reader->readPointAttribute("pos");
                    QString portName = reader->attributes().value("name").toString();
                    component()->ports << new PortData(pos, portName);

                    // Read until end of element
                    reader->readUnknownElement();
                }

            }
        }
    }

    /*!
     * \brief Reads component properties data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::readProperties(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();

        // If we are opening the file for symbol edition, read and set the
        // properties position in the scene
        if(scene) {
            PropertyGroup *properties = scene->properties();
            properties->setPos(reader->readPointAttribute("pos"));
        }

        // Read every individual property
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "properties");
                break;
            }

            if(reader->isStartElement() && reader->name() == "property") {
                Property prop = Property::loadProperty(reader);

                // Check if we are opening the file for edition or to include it in a library
                if(scene) {
                    // We are opening the file for symbol edition
                    scene->addProperty(prop);
                }
                else if(component()) {
                    // We are opening the file as a component to include it in a library
                    component()->properties->addProperty(prop.name(), prop);
                }

            }
        }

    }

    /*!
     * \brief Reads the models data from component description xml file.
     *
     * \param reader XmlReader responsible for reading xml data.
     */
    void FormatXmlSymbol::readModels(Caneda::XmlReader *reader)
    {
        Q_ASSERT(reader->isStartElement() && reader->name() == "models");

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                break;
            }

            if(reader->isStartElement() && reader->name() == "model") {
                    QXmlStreamAttributes attribs(reader->attributes());
                    QString modelType = attribs.value("type").toString();
                    QString modelSyntax = attribs.value("syntax").toString();

                    // Check if we are opening the file for edition or to include it in a library
                    CGraphicsScene *scene = cGraphicsScene();
                    if(scene) {
                        // We are opening the file for symbol edition
                        //! \todo We must add the model as a special property, or allow some form of editing the model
                        // scene->addProperty(prop);
                    }
                    else if(component()) {
                        // We are opening the file as a component to include it in a library
                        component()->models.insert(modelType, modelSyntax);
                    }

                    // Read till end element
                    reader->readUnknownElement();
            }
        }

    }


    /*************************************************************************
     *                           FormatXmlLayout                             *
     *************************************************************************/
    //! \brief Constructor.
    FormatXmlLayout::FormatXmlLayout(LayoutDocument *doc):
        m_layoutDocument(doc)
    {
    }

    bool FormatXmlLayout::save()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QString text = saveText();

        if(text.isEmpty()) {
            qDebug() << "Looks buggy! Null data to save! Was this expected?";
        }

        QFile file(fileName());
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot save document!"));
            return false;
        }
        QTextStream stream(&file);
        stream << text;
        file.close();

        return true;
    }

    bool FormatXmlLayout::load()
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!scene) {
            return false;
        }

        QFile file(fileName());
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(0, QObject::tr("Error"),
                    QObject::tr("Cannot load document ")+fileName());
            return false;
        }

        QTextStream stream(&file);

        bool result = loadFromText(stream.readAll());

        file.close();
        return result;
    }

    QString FormatXmlLayout::saveText()
    {
        QString retVal;
        Caneda::XmlWriter *writer = new Caneda::XmlWriter(&retVal);
        writer->setAutoFormatting(true);

        //Fist we start the document and write current version
        writer->writeStartDocument();
        writer->writeDTD(QString("<!DOCTYPE caneda>"));
        writer->writeStartElement("caneda");
        writer->writeAttribute("version", Caneda::version());

        //Now we copy all the elements and properties in the schematic
        savePaintings(writer);
        saveProperties(writer);

        //Finally we finish the document
        writer->writeEndDocument(); //</caneda>

        delete writer;
        return retVal;
    }

    void FormatXmlLayout::savePaintings(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();
        QList<QGraphicsItem*> items = scene->items();
        QList<Painting*> paintings = filterItems<Painting>(items);
        if(!paintings.isEmpty()) {
            writer->writeStartElement("paintings");
            foreach(Painting *p, paintings) {
                p->saveData(writer);
            }
            writer->writeEndElement(); //</paintings>
        }
    }

    /*!
     * \brief Save scene properties to an XmlWriter.
     *
     * This method saves all scene related propeties to an XmlWriter. To do so,
     * it takes each property from the PropertyGroup of the scene, and saves
     * the data using the Property::saveProperty() method.
     *
     * \param writer XmlWriter to save properties into.
     * \sa Property::saveProperty()
     */
    void FormatXmlLayout::saveProperties(Caneda::XmlWriter *writer)
    {
        CGraphicsScene *scene = cGraphicsScene();

        writer->writeStartElement("properties");

        // Save the properties position in the scene
        PropertyGroup *properties = scene->properties();
        writer->writePointAttribute(properties->pos(), "pos");

        // Read every individual property and add it to the scene
        foreach(Property property, properties->propertyMap()) {
            property.saveProperty(writer);
        }

        writer->writeEndElement(); //</properties>
    }

    bool FormatXmlLayout::loadFromText(const QString& text)
    {
        Caneda::XmlReader *reader = new Caneda::XmlReader(text.toUtf8());
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isStartElement()) {
                if(reader->name() == "caneda" &&
                        Caneda::checkVersion(reader->attributes().value("version").toString())) {

                    while(!reader->atEnd()) {
                        reader->readNext();
                        if(reader->isEndElement()) {
                            Q_ASSERT(reader->name() == "caneda");
                            break;
                        }

                        if(reader->isStartElement()) {
                            if(reader->name() == "paintings") {
                                loadPaintings(reader);
                            }
                            else if(reader->name() == "properties") {
                                loadProperties(reader);
                            }
                            else {
                                reader->readUnknownElement();
                            }
                        }

                    }
                }
                else {
                    reader->raiseError(QObject::tr("Not a caneda file or probably malformatted file"));
                }
            }
        }

        if(reader->hasError()) {
            QMessageBox::critical(0, QObject::tr("Xml parse error"), reader->errorString());
            delete reader;
            return false;
        }

        delete reader;
        return true;
    }

    void FormatXmlLayout::loadPaintings(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();
        if(!reader->isStartElement() || reader->name() != "paintings") {
            reader->raiseError(QObject::tr("Malformatted file"));
        }

        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "paintings");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "painting") {
                    Painting::loadPainting(reader,scene);
                }
                else {
                    qWarning() << "Error: Found unknown painting type" <<
                        reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    void FormatXmlLayout::loadProperties(Caneda::XmlReader *reader)
    {
        CGraphicsScene *scene = cGraphicsScene();

        // Read and set the properties position in the scene
        PropertyGroup *properties = scene->properties();
        properties->setPos(reader->readPointAttribute("pos"));

        // Read every individual property and add it to the scene
        while(!reader->atEnd()) {
            reader->readNext();

            if(reader->isEndElement()) {
                Q_ASSERT(reader->name() == "properties");
                break;
            }

            if(reader->isStartElement()) {
                if(reader->name() == "property") {
                    Property prop = Property::loadProperty(reader);
                    scene->addProperty(prop);
                }
                else {
                    qWarning() << "Error: Found unknown property type" <<
                                  reader->name().toString();
                    reader->readUnknownElement();
                    reader->raiseError(QObject::tr("Malformatted file"));
                }
            }
        }
    }

    LayoutDocument* FormatXmlLayout::layoutDocument() const
    {
        return m_layoutDocument;
    }

    CGraphicsScene* FormatXmlLayout::cGraphicsScene() const
    {
        return m_layoutDocument ? m_layoutDocument->cGraphicsScene() : 0;
    }

    QString FormatXmlLayout::fileName() const
    {
        return m_layoutDocument ? m_layoutDocument->fileName() : QString();
    }

} // namespace Caneda

#ifndef STYLES_HPP
#define STYLES_HPP

#include <QString>

extern QString g_generalWidgetStyleSheet;

class QDockWidget;
class QWidget;
class QPushButton;
void SetupDockWidgetStyle( QDockWidget & _dockWidget );
void SetupTextEditorWidgetStyle( QWidget & _widget );
void SetupPushButtonWidgetStyle( QPushButton & _button );
QString GetRandomBackground();

#endif // STYLES_HPP

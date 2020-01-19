#include "styles.hpp"
#include <QDockWidget>
#include <QPushButton>
#include <QHostInfo>

const char* g_pushButtonWidgetStyle = "background-color: rgba(255,255,255,128);";
const char* g_textEditorWidgetStyle = "background-color: rgba(0,0,0,180 );";
const char* g_dockWidgetStyleSheet = "color: rgb(255,255,255); background-color: rgba(0,0,0,32);";
QString g_generalWidgetStyleSheet;

void SetupDockWidgetStyle( QDockWidget & _dockWidget )
{
	_dockWidget.setStyleSheet( g_dockWidgetStyleSheet );
}

void SetupTextEditorWidgetStyle( QWidget & _widget ){
	_widget.setStyleSheet( g_textEditorWidgetStyle );
}

void SetupPushButtonWidgetStyle( QPushButton & _button ){
	_button.setStyleSheet( g_pushButtonWidgetStyle );
}

QString GetRandomBackground() {
	if (QHostInfo::localHostName() == "Haddock") {
		return ":/resources/background2.png";
	}
	return ":/resources/botw2.jpg";
}

#include <sailfishapp.h>

#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QScopedPointer>

#include <gst/gst.h>

#include "appengine.h"

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    AppEngine engine;
    view->rootContext()->setContextProperty("appEngine", &engine);

    view->setSource(SailfishApp::pathToMainQml());
    view->show();

    return app->exec();
}

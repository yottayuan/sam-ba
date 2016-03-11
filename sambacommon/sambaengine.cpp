#include "sambaengine.h"
#include <QFile>

SambaEngine::SambaEngine(QObject *parent)
    : QObject(parent),
      m_hasWarnings(false)
{
	m_qmlEngine.setOutputWarningsToStandardError(false);

	// forward signals from engine
	QObject::connect(&m_qmlEngine, &QQmlEngine::quit,
					 this, &SambaEngine::engineQuit, Qt::QueuedConnection);
	QObject::connect(&m_qmlEngine, &QQmlEngine::warnings,
					 this, &SambaEngine::engineWarnings);

	// add import path
	m_qmlEngine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
}

void SambaEngine::engineQuit()
{
	QCoreApplication::quit();
}

void SambaEngine::engineWarnings(const QList<QQmlError> &warnings)
{
	foreach(QQmlError warning, warnings)
	{
		QString url;
		if (warning.url().isLocalFile())
			url = warning.url().toLocalFile();
		else
			url = warning.url().toString();
		qCWarning(sambaLogQml, "%s:%d: %s", url.toLocal8Bit().constData(),
				  warning.line(), warning.description().toLocal8Bit().constData());
		m_hasWarnings = true;
	}
}

SambaEngine::~SambaEngine()
{
}

QQmlEngine* SambaEngine::qmlEngine()
{
	return &m_qmlEngine;
}

QObject* SambaEngine::createComponentInstance(QQmlComponent* component, QQmlContext* context)
{
	m_hasWarnings = false;

	if (component->status() != QQmlComponent::Ready)
	{
		qCCritical(sambaLogCore) << component->errorString();
		return 0;
	}

	QObject* obj = component->create(context);
	if (!obj)
		return 0;

	if (m_hasWarnings) {
		delete obj;
		return 0;
	}

	return obj;
}

QObject* SambaEngine::createComponentInstance(const QString& script, QQmlContext* context)
{
	QQmlComponent component(&m_qmlEngine);
	component.setData(script.toLocal8Bit(), QUrl());
	return createComponentInstance(&component, context);
}

QObject* SambaEngine::createComponentInstance(const QUrl &url, QQmlContext* context)
{
	QQmlComponent component(&m_qmlEngine, url);
	return createComponentInstance(&component, context);
}

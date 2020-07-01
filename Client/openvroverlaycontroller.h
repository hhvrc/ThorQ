//====== Copyright Valve Corporation, All rights reserved. =======

#ifndef OPENVROVERLAYCONTROLLER_H
#define OPENVROVERLAYCONTROLLER_H

#ifdef _WIN32
#pragma once
#endif

#include "openvr.h"

#include <QtCore/QtCore>
// because of incompatibilities with QtOpenGL and GLEW we need to cherry pick includes
#include <QtGui/QVector2D>
#include <QtGui/QMatrix4x4>
#include <QtCore/QVector>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtWidgets/QGraphicsScene>
#include <QtGui/QOffscreenSurface>

class OpenVROverlayController : public QObject
{
	Q_OBJECT

public:
	static OpenVROverlayController *SharedInstance();

public:
	OpenVROverlayController();
	virtual ~OpenVROverlayController();

	bool Init(const QString& name);
	void Shutdown();
	void EnableRestart();

	bool BHMDAvailable();
	vr::IVRSystem* GetVRSystem();
	vr::HmdError GetLastHmdError();

	QString GetVRDriverString();
	QString GetVRDisplayString();
	QString GetName() { return m_strName; }

	void SetWidget( QWidget* pWidget );
	QWidget* GetWidget() const;

	void SetTint(const QColor& color);
	QColor GetTint() const;

	void SetAlpha(float alpha);
	float GetAlpha() const;

	void SetWidth(float meters);
	float GetWidth() const;
public slots:
	void OnSceneChanged( const QList<QRectF>& );
	void OnTimeoutPumpEvents();

protected:

private:
	bool ConnectToVRRuntime();
	void DisconnectFromVRRuntime();

	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	QString m_strVRDriver;
	QString m_strVRDisplay;
	QString m_strName;


private:
	vr::HmdError m_hmdError;
	vr::HmdError m_compositorError;
	vr::HmdError m_overlayError;
	vr::VROverlayHandle_t m_overlayHandle;

	vr::IVRSystem* m_VRSystem;

	QOpenGLContext *m_openGLContext;
	QGraphicsScene *m_scene;
	QOpenGLFramebufferObject *m_frameBuffer;
	QOffscreenSurface *m_vrSurface;

	QTimer *m_pumpEventsTimer;

	// the widget we're drawing into the texture
	QWidget *m_widget;

	QPointF m_lastMousePoint;
	Qt::MouseButtons m_lastMouseButtons;
};


#endif // OPENVROVERLAYCONTROLLER_H

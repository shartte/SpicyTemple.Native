
#pragma once

#include "gl_headers.h"

#include <memory>

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickView>
#include <QSGRendererInterface>
#include <QScreen>
#include <utility>
#include "GameViewItem.h"
#include "GameViews.h"
#include "completion_source.h"
#include "events.h"

#include <qpa/qplatformnativeinterface.h>

#include <d3d11.h>
#include <QMap>

struct NativeWindowConfig {
  int minWidth;
  int minHeight;
  int width;
  int height;
  bool fullScreen;
};

class Ui : public QObject {
  Q_OBJECT
  Q_PROPERTY(QSize renderTargetSize READ renderTargetSize)
  Q_PROPERTY(QString baseUrl READ baseUrl WRITE setBaseUrl)
  Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle)
  Q_PROPERTY(void *app READ app)
  Q_PROPERTY(void *nativeHandle READ nativeHandle)
 private:
  std::unique_ptr<QGuiApplication> _app;
  std::unique_ptr<QQuickWindow> _window;
  std::unique_ptr<QQmlEngine> _engine;
  std::unique_ptr<QCursor> _transparentCursor;

 public:
  Q_INVOKABLE Ui();

  QQmlEngine *engine() const { return _engine.get(); }

  QQuickWindow *window() const { return _window.get(); }

  void *app() const { return _app.get(); }

  void *nativeHandle() const { return (void *)_window->winId(); }

  Q_INVOKABLE void loadItemAsync(const QString &path, QObjectCompletionSource *completionSource);

  Q_INVOKABLE QObject *loadItem(const QString &path);

  Q_INVOKABLE void queueUpdate();

  Q_INVOKABLE void show() { _window->show(); }

  Q_INVOKABLE void addToRoot(QQuickItem *item);
  Q_INVOKABLE void removeFromRoot(QQuickItem *item);

  Q_INVOKABLE void setStyle(const QString &style);

  QSize renderTargetSize() const;

  QString baseUrl() const { return _engine->baseUrl().toString(); }

  void setBaseUrl(const QString &baseUrl) const;

  QString windowTitle() const { return _window->title(); }

  void setWindowTitle(const QString &title) { _window->setTitle(title); }

  Q_INVOKABLE void setWindowIcon(const QString &path);

  void setCursor(QCursor &cursor) { _window->setCursor(cursor); }

  void hideCursor() { _window->setCursor(*_transparentCursor); }

  void setTitle(const QString &title) { _window->setTitle(title); }

  void setConfig(const NativeWindowConfig &config);

  void setBeforeRenderingCallback(std::function<void()> callback) {
    _beforeRenderingCallback = std::move(callback);
  }

  void setBeforeRenderPassRecordingCallback(std::function<void()> callback) {
    _beforeRenderPassRecordingCallback = std::move(callback);
  }

  void setAfterRenderPassRecordingCallback(std::function<void()> callback) {
    _afterRenderPassRecordingCallback = std::move(callback);
  }

  void setAfterRenderingCallback(std::function<void()> callback) {
    _afterRenderingCallback = std::move(callback);
  }

  void setMouseEventFilter(const std::function<bool(const MouseEvent &)> &mouseEventFilter) {
    _mouseEventFilter = mouseEventFilter;
  }

  void setWheelEventFilter(const std::function<bool(const WheelEvent &)> &wheelEventFilter) {
    _wheelEventFilter = wheelEventFilter;
  }

  void setKeyEventFilter(const std::function<bool(const KeyEvent &)> &keyEventFilter) {
    _keyEventFilter = keyEventFilter;
  }

  void setCloseCallback(const std::function<void()> &closeCallback) {
    _closeCallback = closeCallback;
  }

  void setDeviceCreatedCallback(const std::function<void(ID3D11Device *)> &deviceCreatedCallback) {
    _deviceCreatedCallback = deviceCreatedCallback;
  }

  void setDeviceDestroyedCallback(
      const std::function<void(ID3D11Device *)> &deviceDestroyedCallback) {
    _deviceDestroyedCallback = deviceDestroyedCallback;
  }

 protected:
  Q_SLOT void handleSceneGraphInitialized();

  Q_SLOT void handleSceneGraphInvalidated();

  bool eventFilter(QObject *object, QEvent *event) override {
    if (object != _window.get()) {
      return false;
    }

    auto type = event->type();
    if (_mouseEventFilter &&
        (type == QEvent::MouseButtonDblClick || type == QEvent::MouseButtonPress ||
         type == QEvent::MouseButtonRelease || type == QEvent::MouseMove ||
         type == QEvent::MouseTrackingChange)) {
      MouseEvent mouseEvent(*static_cast<QMouseEvent *>(event));
      return _mouseEventFilter(mouseEvent);
    } else if (_wheelEventFilter && type == QEvent::Wheel) {
      WheelEvent wheelEvent(*static_cast<QWheelEvent *>(event));
      return _wheelEventFilter(wheelEvent);
    } else if (_keyEventFilter && (type == QEvent::KeyPress || type == QEvent::KeyRelease)) {
      KeyEvent keyEvent(*static_cast<QKeyEvent *>(event));
      return _keyEventFilter(keyEvent);
    }
    return false;
  }

 private:
  EGLDisplay _eglDisplay = EGL_NO_DISPLAY;
  EGLConfig _eglConfig = EGL_NO_CONFIG_KHR;
  ID3D11Device *_d3d11device = nullptr;
  bool _openglContextCreated = false;
  std::function<void()> _beforeRenderingCallback;
  std::function<void()> _beforeRenderPassRecordingCallback;
  std::function<void()> _afterRenderPassRecordingCallback;
  std::function<void()> _afterRenderingCallback;
  std::function<bool(const MouseEvent &e)> _mouseEventFilter;
  std::function<bool(const WheelEvent &e)> _wheelEventFilter;
  std::function<bool(const KeyEvent &e)> _keyEventFilter;
  std::function<void()> _closeCallback;
  std::function<void(ID3D11Device *)> _deviceCreatedCallback;
  std::function<void(ID3D11Device *)> _deviceDestroyedCallback;

  // We must cache components so that the associated meta objects never get released
  QMap<QString, QQmlComponent *> _components;

  QQmlComponent *createComponent(const QString &path);
};

Q_DECLARE_METATYPE(Ui *);
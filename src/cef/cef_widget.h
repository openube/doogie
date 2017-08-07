#ifndef DOOGIE_CEF_WIDGET_H_
#define DOOGIE_CEF_WIDGET_H_

#include <QtWidgets>
#include <vector>

#include "bubble.h"
#include "cef/cef_base_widget.h"
#include "cef/cef_handler.h"

namespace doogie {

class CefWidget : public CefBaseWidget {
  Q_OBJECT

 public:
  struct NavEntry {
    QString url;
    QString title;
    bool current;
  };

  explicit CefWidget(const Cef& cef,
                     const Bubble& bubble,
                     const QString& url = "",
                     QWidget* parent = nullptr);
  ~CefWidget();

  std::vector<NavEntry> NavEntries() const;
  // If result is non-null, it needs to replace this widget
  QPointer<QWidget> OverrideWidget() const;
  void LoadUrl(const QString& url);
  QString CurrentUrl() const;
  void TryClose();

  // Basically just calls history.go
  void Go(int num);
  void Refresh(bool ignore_cache);
  void Stop();
  void Print();

  void Find(const QString& text,
            bool forward,
            bool match_case,
            bool continued);
  void CancelFind(bool clear_selection);

  void ExecJs(const QString& js);

  void ShowDevTools(CefBaseWidget* widg, const QPoint& inspect_at);
  void ExecDevToolsJs(const QString& js);
  void CloseDevTools();

  double ZoomLevel() const;
  void SetZoomLevel(double level);

  void SetJsDialogCallback(CefHandler::JsDialogCallback callback);

 signals:
  void Closed();
  void PreContextMenu(CefRefPtr<CefContextMenuParams> params,
                      CefRefPtr<CefMenuModel> model);
  void ContextMenuCommand(CefRefPtr<CefContextMenuParams> params,
                          int command_id,
                          CefContextMenuHandler::EventFlags event_flags);
  void UrlChanged(const QString& url);
  void TitleChanged(const QString& title);
  void StatusChanged(const QString& status);
  void FaviconChanged(const QIcon& icon);
  void LoadStateChanged(bool is_loading,
                        bool can_go_back,
                        bool can_go_forward);
  void PageOpen(CefHandler::WindowOpenType type,
                const QString& url,
                bool user_gesture);
  void DevToolsLoadComplete();
  void DevToolsClosed();
  void FindResult(int count, int index);
  void ShowBeforeUnloadDialog(const QString& message_text,
                              bool is_reload,
                              CefRefPtr<CefJSDialogCallback> callback);

 protected:
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void UpdateSize() override;

 private:
  class FaviconDownloadCallback : public CefDownloadImageCallback {
   public:
    explicit FaviconDownloadCallback(QPointer<CefWidget> cef_widg)
      : cef_widg_(cef_widg) {}
    void OnDownloadImageFinished(const CefString& image_url,
                                 int http_status_code,
                                 CefRefPtr<CefImage> image) override;
   private:
    QPointer<CefWidget> cef_widg_;
    IMPLEMENT_REFCOUNTING(FaviconDownloadCallback)
    DISALLOW_COPY_AND_ASSIGN(FaviconDownloadCallback);
  };

  class NavEntryVisitor : public CefNavigationEntryVisitor {
   public:
    bool Visit(CefRefPtr<CefNavigationEntry> entry,
               bool current, int, int) override {
      NavEntry nav_entry = {
        QString::fromStdString(entry->GetURL().ToString()),
        QString::fromStdString(entry->GetTitle().ToString()),
        current
      };
      entries_.push_back(nav_entry);
      return true;
    }
    std::vector<NavEntry> Entries() { return entries_; }

   private:
    std::vector<NavEntry> entries_;
    IMPLEMENT_REFCOUNTING(NavEntryVisitor)
  };

  void InitBrowser(const Bubble& bubble, const QString& url);

  CefRefPtr<CefHandler> handler_;
  CefRefPtr<CefBrowser> browser_;
  CefRefPtr<CefHandler> dev_tools_handler_;
  CefRefPtr<CefBrowser> dev_tools_browser_;
  QPointer<QWidget> override_widget_;
  bool download_favicon_;
};

}  // namespace doogie

#endif  // DOOGIE_CEF_WIDGET_H_

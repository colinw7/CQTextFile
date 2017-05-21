#ifndef CTEXT_FILE_KEY_H
#define CTEXT_FILE_KEY_H

class CTextFile;
class CTextFileSel;
class CTextFileUtil;
class CTextFileUndo;

#include <CKeyType.h>
#include <CEvent.h>
#include <CTextFileEd.h>

class CTextFileKeyNotifierMgr;

class CTextFileKeyNotifier {
 public:
  CTextFileKeyNotifier();

  virtual void enterCmdLineMode(const std::string &str);

  virtual void showOverlayMsg(const std::string &msg);

  virtual void showStatusMsg(const std::string &msg);

  virtual void notifyScrollTop();
  virtual void notifyScrollMiddle();
  virtual void notifyScrollBottom();

  virtual void notifyOverwrite(bool);

  virtual void notifyNumber(bool);

  virtual void notifyQuit();
};

//---

class CTextFileKey {
 public:
  CTextFileKey(CTextFile *file);
 ~CTextFileKey();

  virtual void processChar(CKeyType key, const std::string &text, CEventModifier modifier) = 0;

  virtual void execCmd(const std::string &cmd) = 0;

  CIPoint2D getPos() const;

  uint getRow() const;
  uint getCol() const;

  uint getPageLength() const;

  CTextFileUndo *getUndo() const { return undo_; }

  CTextFileSel *getSelection() const { return sel_; }

  bool getOverwrite() const { return overwrite_; }

  void setOverwrite(bool overwrite);

  uint getTabStop() const { return tab_stop_; }

  void addNotifier   (CTextFileKeyNotifier *notifier);
  void removeNotifier(CTextFileKeyNotifier *notifier);

  void enterCmdLineMode(const std::string &cmd);

  void showOverlayMsg(const std::string &cmd);

  void showStatusMsg(const std::string &msg);

  void notifyScrollTop();
  void notifyScrollMiddle();
  void notifyScrollBottom();

  void notifyOverwrite(bool);

  void notifyNumber(bool);

  void notifyQuit();

  void extendSelectLeft (int n=1);
  void extendSelectRight(int n=1);
  void extendSelectUp   (int n=1);
  void extendSelectDown (int n=1);

  bool findNext();
  bool findNext(const std::string &pattern);

  bool findPrev();
  bool findPrev(const std::string &pattern);

 protected:
  CTextFile               *file_ { nullptr };
  CTextFileUndo           *undo_ { nullptr };
  CTextFileSel            *sel_ { nullptr };
  CTextFileUtil           *util_ { nullptr };
  bool                     overwrite_ { false };
  uint                     tab_stop_ { 8 };
  CTextFileKeyNotifierMgr *notifyMgr_ { nullptr };
  std::string              findPattern_;
};

//---

class CTextFileKeyNotifierMgr {
 public:
  CTextFileKeyNotifierMgr(CTextFileKey *key);

  void addNotifier   (CTextFileKeyNotifier *notifier);
  void removeNotifier(CTextFileKeyNotifier *notifier);

  void enterCmdLineMode(const std::string &line);

  void showOverlayMsg(const std::string &msg);

  void showStatusMsg(const std::string &msg);

  void notifyScrollTop();
  void notifyScrollMiddle();
  void notifyScrollBottom();

  void notifyOverwrite(bool overwrite);

  void notifyNumber(bool number);

  void notifyQuit();

 private:
  typedef std::list<CTextFileKeyNotifier *> NotifierList;

  CTextFileKey *key_ { nullptr };
  NotifierList  notifierList_;
};

#endif

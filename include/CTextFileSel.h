#ifndef CTEXT_SELECTION_H
#define CTEXT_SELECTION_H

#include <CIBBox2D.h>
#include <CRGBA.h>
#include <list>

class CTextFile;

class CTextFileSelNotifierMgr;

class CTextFileSelNotifier {
 public:
  CTextFileSelNotifier();

  virtual ~CTextFileSelNotifier() { }

  virtual void selectionChanged(const std::string &str);
};

class CTextFileSel {
 public:
  enum SelMode {
    RANGE_SEL_MODE,
    RECT_SEL_MODE,
  };

  CTextFileSel(CTextFile *file);

  void addNotifier   (CTextFileSelNotifier *notifier);
  void removeNotifier(CTextFileSelNotifier *notifier);

  void clearSelection();

  void setMode(SelMode mode);

  bool isSelected() const { return selected_; }

  void selectBBox  (const CIBBox2D &bbox, bool clear);
  void selectInside(const CIBBox2D &bbox, bool clear);

  void rangeSelect(const CIBBox2D &bbox, bool clear);
  void rangeSelect(int row1, int col1, int row2, int col2, bool clear);
  void rangeSelect(const CIPoint2D &start, const CIPoint2D &end, bool clear);

  void selectChar(int row, int col);
  void selectLine(int row);

  void setSelectionColor(const CRGBA &color);

  void setSelectRange(const CIPoint2D &start, const CIPoint2D &end);

  const CIPoint2D &getSelectStart() const;
  const CIPoint2D &getSelectEnd  () const;

  std::string getSelectedText() const;

  bool isLineInside(uint row) const;

  bool isPartLineInside(uint row) const;

  bool isCharInside(uint row, int col) const;

  bool insideSelection(const CIPoint2D &pos) const;

  bool isValid() const;

  static int cmp(const CIPoint2D &p1, const CIPoint2D &p2);

 private:
  void notifySelectionChanged();

 private:
  CTextFile               *file_ { nullptr };
  bool                     selected_ { false };
  SelMode                  selMode_ { RANGE_SEL_MODE };
  CIPoint2D                start_;
  CIPoint2D                end_;
  CTextFileSelNotifierMgr *notifyMgr_ { nullptr };
};

//---

class CTextFileSelNotifierMgr {
 public:
  CTextFileSelNotifierMgr(CTextFileSel *sel);

  CTextFileSel *sel() const { return sel_; }

  void addNotifier   (CTextFileSelNotifier *notifier);
  void removeNotifier(CTextFileSelNotifier *notifier);

  void selectionChanged(const std::string &line);

 private:
  typedef std::list<CTextFileSelNotifier *> NotifierList;

  CTextFileSel *sel_ { nullptr };
  NotifierList  notifierList_;
};

#endif

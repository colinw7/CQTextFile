#ifndef CQTEXT_FILE_H
#define CQTEXT_FILE_H

#include <QWidget>
#include <CScrollType.h>
#include <CTextFileSel.h>
#include <CTextFileKey.h>

class CQTextFileCanvas;
class QScrollBar;
class CTextFile;
class CTextFileKey;
class CTextFileViKey;
class CTextFileNormalKey;
class CTextFileEd;

class CQTextFile : public QWidget, public CTextFileSelNotifier, public CTextFileKeyNotifier {
  Q_OBJECT

 public:
  enum EditMode {
    NORMAL_EDIT_MODE,
    VI_EDIT_MODE
  };

  enum SelMode {
    RANGE_SEL_MODE,
    RECT_SEL_MODE
  };

 public:
  CQTextFile(QWidget *parent=NULL);

  CQTextFileCanvas *getCanvas () const { return canvas_ ; }
  QScrollBar       *getHScroll() const { return hscroll_; }
  QScrollBar       *getVScroll() const { return vscroll_; }

  CTextFile *getFile() const { return file_; }

  CTextFileKey *getKey() const;

  CTextFileViKey     *getViKey    () const { return viKey_; }
  CTextFileNormalKey *getNormalKey() const { return normalKey_; }

  void setEditMode(EditMode mode);

  void setSelMode(SelMode mode);

  void notifyScrollTop   ();
  void notifyScrollMiddle();
  void notifyScrollBottom();

  void notifyNumber(bool number);

  void setNumber(bool number);
  bool getNumber() const { return number_; }

  void loadFile(const char *fileName);

  void scrollToPos(CScrollType type);

  void selectionChanged(const std::string &str);

 private slots:
  void hscrollSlot();
  void vscrollSlot();

 signals:
  void textEntered(const QString &text);

  void sizeChanged(int, int);

 private:
  CQTextFileCanvas*   canvas_ { nullptr };
  QScrollBar*         hscroll_ { nullptr };
  QScrollBar*         vscroll_ { nullptr };
  CTextFile*          file_ { nullptr };
  EditMode            editMode_ { NORMAL_EDIT_MODE };
  SelMode             selMode_ { RANGE_SEL_MODE };
  CTextFileNormalKey* normalKey_ { nullptr };
  CTextFileViKey*     viKey_ { nullptr };
  bool                number_ { false };
};

#endif

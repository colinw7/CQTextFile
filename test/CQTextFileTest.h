#include <CTextFile.h>
#include <CTextFileKey.h>

#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>

class QHBoxLayout;
class QLabel;
class QTextEdit;
class QStackedWidget;

class CTextFileEd;

class CQWinWidget;
class CQTextFileTest;
class CQTextFile;

class CQTextFileControl {
 public:
  CQTextFileControl(CQTextFileTest *test);

 protected:
  CQTextFileTest *test_ { nullptr };
  CQTextFile     *file_ { nullptr };
};

class CQTextFileToolBar : public QFrame, public CQTextFileControl {
  Q_OBJECT

 public:
  CQTextFileToolBar(CQTextFileTest *test);

  void addWidget(QWidget *w);

 private:
  QHBoxLayout *layout_ { nullptr };
  int          pos_ { 0 };
};

class CQTextFileStatusBar : public QFrame, public CQTextFileControl {
  Q_OBJECT

 public:
  CQTextFileStatusBar(CQTextFileTest *test);

  void addWidget(QWidget *w);

 private:
  QHBoxLayout *layout_ { nullptr };
  int          pos_ { 0 };
};

class CQTextFileSize : public QLabel {
  Q_OBJECT

 public:
  CQTextFileSize(CQTextFileTest *test);

 public slots:
  void sizeChanged(int rows, int cols);
};

class CQTextFilePos : public QLabel, public CQTextFileControl, public CTextFileNotifier {
  Q_OBJECT

 public:
  CQTextFilePos(CQTextFileTest *test);

  void positionChanged(uint x, uint y);
};

class CQTextFileOverwrite : public QLabel, public CQTextFileControl, public CTextFileKeyNotifier {
  Q_OBJECT

 public:
  CQTextFileOverwrite(CQTextFileTest *test);

  void notifyOverwrite(bool overwrite);
};

class CQTextFileEditMode : public QToolButton, public CQTextFileControl {
  Q_OBJECT

 public:
  CQTextFileEditMode(CQTextFileTest *test);

 private slots:
  void modeChanged(bool);
};

class CQTextFileSelMode : public QToolButton, public CQTextFileControl {
  Q_OBJECT

 public:
  CQTextFileSelMode(CQTextFileTest *test);

 private slots:
  void modeChanged(bool);
};

class CQTextFileNumMode :
 public QToolButton, public CQTextFileControl, public CTextFileKeyNotifier {
  Q_OBJECT

 public:
  CQTextFileNumMode(CQTextFileTest *test);

  void notifyNumber(bool);

 private slots:
  void modeChanged(bool);
};

class CQTextFileEdit : public QLineEdit, public CQTextFileControl {
  Q_OBJECT

 public:
  enum Mode {
    CMD_MODE,
    FIND_FORWARD_MODE,
    FIND_BACKWARD_MODE
  };

 public:
  CQTextFileEdit(CQTextFileTest *test);

  void startCmd(const std::string &str);
  void endCmd  ();

 private:
  void focusInEvent(QFocusEvent *e);
  void focusOutEvent(QFocusEvent *e);

  bool event(QEvent *e);

 private slots:
  void processCmd();

 private:
  Mode mode_;
};

class CQTextFileTest : public QWidget, public CTextFileKeyNotifier {
  Q_OBJECT

 public:
  CQTextFileTest();

  CQTextFile *getTextFile() const { return area_; }

  void loadFile(const char *fileName);

  void enterCmdLineMode(const std::string &str);

  void showOverlayMsg(const std::string &msg);

  void showStatusMsg(const std::string &msg);

  void notifyQuit();

  void resetStatus();

  QSize sizeHint() const { return QSize(600,800); }

 private:
  CQTextFile          *area_ { nullptr };
  CQTextFileToolBar   *toolbar_ { nullptr };
  QStackedWidget      *stack_ { nullptr };
  CQTextFileStatusBar *status_ { nullptr };
  CQTextFileEdit      *edit_ { nullptr };
  QLabel              *msg_ { nullptr };
  CQTextFileSize      *size_ { nullptr };
  CQTextFileOverwrite *overwrite_ { nullptr };
  CQTextFilePos       *pos_ { nullptr };
  CQTextFileEditMode  *editMode_ { nullptr };
  CQTextFileSelMode   *selMode_ { nullptr };
  CQTextFileNumMode   *numMode_ { nullptr };
  CQWinWidget         *msgWidget_ { nullptr };
  QTextEdit           *msgWidgetText_ { nullptr };
};

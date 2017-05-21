#include <CQTextFileTest.h>
#include <CQTextFile.h>
#include <CTextFileEd.h>
#include <CTextFileViKey.h>
#include <CTextFileNormalKey.h>
#include <CQApp.h>
#include <CQWinWidget.h>
#include <QVBoxLayout>
#include <QToolButton>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QTextEdit>

#include <svg/viMode_svg.h>
#include <svg/selArea_svg.h>
#include <svg/number_svg.h>

int
main(int argc, char **argv)
{
  CQApp app(argc, argv);

  CQTextFileTest *test = new CQTextFileTest;

  if (argc > 1)
    test->loadFile(argv[1]);

  test->show();

  return app.exec();
}

CQTextFileTest::
CQTextFileTest() :
 msgWidget_(0),
 msgWidgetText_(0)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(2); layout->setSpacing(2);

  //---

  area_    = new CQTextFile;
  toolbar_ = new CQTextFileToolBar(this);
  stack_   = new QStackedWidget;

  stack_->setObjectName("stack");

  layout->addWidget(toolbar_);
  layout->addWidget(area_);
  layout->addWidget(stack_);

  //---

  editMode_ = new CQTextFileEditMode(this);
  selMode_  = new CQTextFileSelMode (this);
  numMode_  = new CQTextFileNumMode (this);

  toolbar_->addWidget(editMode_);
  toolbar_->addWidget(selMode_ );
  toolbar_->addWidget(numMode_ );

  toolbar_->setFixedHeight(editMode_->sizeHint().height());

  //---

  status_ = new CQTextFileStatusBar(this);
  edit_   = new CQTextFileEdit(this);

  stack_->addWidget(status_);
  stack_->addWidget(edit_);

  msg_       = new QLabel;
  size_      = new CQTextFileSize(this);
  overwrite_ = new CQTextFileOverwrite(this);
  pos_       = new CQTextFilePos(this);

  status_->addWidget(msg_      );
  status_->addWidget(size_     );
  status_->addWidget(overwrite_);
  status_->addWidget(pos_      );

  stack_->setFixedHeight(msg_->sizeHint().height() + 8);

  connect(area_, SIGNAL(sizeChanged(int,int)), size_, SLOT(sizeChanged(int,int)));

  //---

  area_->getViKey    ()->addNotifier(this);
  area_->getNormalKey()->addNotifier(this);
}

void
CQTextFileTest::
loadFile(const char *fileName)
{
  area_->loadFile(fileName);
}

void
CQTextFileTest::
enterCmdLineMode(const std::string &str)
{
  stack_->setCurrentIndex(1);

  edit_->startCmd(str);
}

void
CQTextFileTest::
showOverlayMsg(const std::string &msg)
{
  if (! msgWidget_) {
    msgWidget_ = new CQWinWidget(area_);

    msgWidgetText_ = new QTextEdit;

    msgWidgetText_->setFont(area_->font());

    msgWidget_->setChild(msgWidgetText_);
  }

  msgWidgetText_->setText(msg.c_str());

  int w = 300;
  int h = 300;

  msgWidget_->setChildSize(QSize(w, h));

  msgWidget_->setPos((area_->width() - w)/2, (area_->height() - h)/2);

  msgWidget_->show();
}

void
CQTextFileTest::
showStatusMsg(const std::string &msg)
{
  msg_->setText(msg.c_str());
}

void
CQTextFileTest::
notifyQuit()
{
  exit(0);
}

void
CQTextFileTest::
resetStatus()
{
  stack_->setCurrentIndex(0);
}

//------

CQTextFileControl::
CQTextFileControl(CQTextFileTest *test) :
 test_(test)
{
  file_ = test_->getTextFile();
}

//------

CQTextFileToolBar::
CQTextFileToolBar(CQTextFileTest *test) :
 QFrame(0), CQTextFileControl(test), pos_(0)
{
  setObjectName("toolbar");

  setFrameStyle(QFrame::Panel | QFrame::Raised);
  setLineWidth(2);

  layout_ = new QHBoxLayout(this);
  layout_->setMargin(2); layout_->setSpacing(2);

  //----

  layout_->addStretch();
}

void
CQTextFileToolBar::
addWidget(QWidget *w)
{
  layout_->insertWidget(pos_, w);

  ++pos_;
}

//------

CQTextFileStatusBar::
CQTextFileStatusBar(CQTextFileTest *test) :
 QFrame(0), CQTextFileControl(test), pos_(0)
{
  setObjectName("status");

  setFrameStyle(QFrame::Panel | QFrame::Raised);
  setLineWidth(2);

  layout_ = new QHBoxLayout(this);
  layout_->setMargin(2); layout_->setSpacing(2);
}

void
CQTextFileStatusBar::
addWidget(QWidget *w)
{
  layout_->insertWidget(pos_, w);

  ++pos_;
}

//------

CQTextFileEdit::
CQTextFileEdit(CQTextFileTest *test) :
 QLineEdit(0), CQTextFileControl(test), mode_(CMD_MODE)
{
  setObjectName("cmd");

  connect(this, SIGNAL(returnPressed()), this, SLOT(processCmd()));

  focusOutEvent(0);
}

void
CQTextFileEdit::
startCmd(const std::string &str)
{
  std::string str1 = str;

  char c = (! str1.empty() ? str1[0] : '\0');

  if      (c == ':') { mode_ = CMD_MODE          ; str1 = str1.substr(1); }
  else if (c == '/') { mode_ = FIND_FORWARD_MODE ; str1 = str1.substr(1); }
  else if (c == '?') { mode_ = FIND_BACKWARD_MODE; str1 = str1.substr(1); }

  setFocus();

  setText(str1.c_str());
}

void
CQTextFileEdit::
endCmd()
{
  setText("");

  file_->setFocus();

  test_->resetStatus();
}

void
CQTextFileEdit::
processCmd()
{
  if      (mode_ == CMD_MODE) {
    file_->getFile()->startGroup();

    file_->getKey()->execCmd(text().toStdString());

    file_->getFile()->endGroup();
  }
  else if (mode_ == FIND_FORWARD_MODE)
    file_->getKey()->findNext(text().toStdString());
  else if (mode_ == FIND_BACKWARD_MODE)
    file_->getKey()->findPrev(text().toStdString());

  endCmd();
}

void
CQTextFileEdit::
focusInEvent(QFocusEvent *e)
{
  QPalette palette;

  palette.setColor(backgroundRole(), QColor(255,255,255));

  setPalette(palette);

  if (e) QLineEdit::focusInEvent(e);
}

void
CQTextFileEdit::
focusOutEvent(QFocusEvent *e)
{
  QPalette palette;

  palette.setColor(backgroundRole(), QColor(240,240,240));

  setPalette(palette);

  if (e) QLineEdit::focusOutEvent(e);
}

bool
CQTextFileEdit::
event(QEvent *e)
{
  if (e->type() == QEvent::KeyPress ||
      e->type() == QEvent::KeyRelease ||
      e->type() == QEvent::ShortcutOverride) {
    QKeyEvent *ke = dynamic_cast<QKeyEvent *>(e);

    if (ke == 0) return false;

    int key = ke->key();

    if (key == Qt::Key_Escape) {
      endCmd();

      return true;
    }
  }

  return QLineEdit::event(e);
}

//------

CQTextFileEditMode::
CQTextFileEditMode(CQTextFileTest *test) :
 QToolButton(0), CQTextFileControl(test)
{
  setObjectName("edit_mode");

  setIcon(CQPixmapCacheInst->getIcon("VIMODE"));

  setCheckable(true);

  setChecked(true);

  setFocusPolicy(Qt::NoFocus);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(modeChanged(bool)));
}

void
CQTextFileEditMode::
modeChanged(bool state)
{
  if (state)
    file_->setEditMode(CQTextFile::VI_EDIT_MODE);
  else
    file_->setEditMode(CQTextFile::NORMAL_EDIT_MODE);
}

//------

CQTextFileSelMode::
CQTextFileSelMode(CQTextFileTest *test) :
 QToolButton(0), CQTextFileControl(test)
{
  setObjectName("sel_mode");

  setIcon(CQPixmapCacheInst->getIcon("SELAREA"));

  setCheckable(true);

  setFocusPolicy(Qt::NoFocus);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(modeChanged(bool)));
}

void
CQTextFileSelMode::
modeChanged(bool state)
{
  if (state)
    file_->setSelMode(CQTextFile::RECT_SEL_MODE);
  else
    file_->setSelMode(CQTextFile::RANGE_SEL_MODE);
}

//------

CQTextFileNumMode::
CQTextFileNumMode(CQTextFileTest *test) :
 QToolButton(0), CQTextFileControl(test)
{
  setObjectName("file_num_mode");

  setIcon(CQPixmapCacheInst->getIcon("NUMBER"));

  setCheckable(true);

  setFocusPolicy(Qt::NoFocus);

  connect(this, SIGNAL(toggled(bool)), this, SLOT(modeChanged(bool)));

  file_->getViKey    ()->addNotifier(this);
  file_->getNormalKey()->addNotifier(this);
}

void
CQTextFileNumMode::
notifyNumber(bool number)
{
  setChecked(number);
}

void
CQTextFileNumMode::
modeChanged(bool state)
{
  file_->setNumber(state);
}

//------

CQTextFileSize::
CQTextFileSize(CQTextFileTest *test) :
 QLabel(test)
{
  setObjectName("file_size");

  QFontMetrics fm(font());

  setFixedHeight(fm.height() + 2);

  setFixedWidth(fm.width("XXXxXXX"));
}

void
CQTextFileSize::
sizeChanged(int rows, int cols)
{
  setText(QString("%1x%2").arg(rows).arg(cols));
}

//------

CQTextFilePos::
CQTextFilePos(CQTextFileTest *test) :
 QLabel(0), CQTextFileControl(test)
{
  setObjectName("file_pos");

  QFontMetrics fm(font());

  setFixedHeight(fm.height() + 2);

  setFixedWidth(fm.width("XXX,XXX"));

  file_->getFile()->addNotifier(this);
}

void
CQTextFilePos::
positionChanged(uint x, uint y)
{
  setText(QString("%1,%2").arg(x).arg(y));
}

//------

CQTextFileOverwrite::
CQTextFileOverwrite(CQTextFileTest *test) :
 QLabel(0), CQTextFileControl(test)
{
  setObjectName("overwrite");

  QFontMetrics fm(font());

  setFixedHeight(fm.height() + 2);

  setFixedWidth(fm.width("XXX"));

  setText("INS");

  file_->getViKey    ()->addNotifier(this);
  file_->getNormalKey()->addNotifier(this);
}

void
CQTextFileOverwrite::
notifyOverwrite(bool overwrite)
{
  setText(overwrite ? "OVR" : "INS");
}

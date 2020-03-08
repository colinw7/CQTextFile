#include <CQTextFile.h>
#include <CQTextFileCanvas.h>
#include <CTextFile.h>
#include <CTextFileNormalKey.h>
#include <CTextFileViKey.h>
#include <CTextFileEd.h>
#include <CTextFileUndo.h>
#include <CTextFileSel.h>
#include <CQUtil.h>
#include <CQWindow.h>
#include <CFileUtil.h>
#include <CStrUtil.h>

#include <QGridLayout>
#include <QScrollBar>
#include <QMouseEvent>
#include <QToolTip>
#include <QPainter>
#include <QApplication>
#include <QClipboard>

CQTextFile::
CQTextFile(QWidget *parent) :
 QWidget   (parent),
 canvas_   (0),
 hscroll_  (0),
 vscroll_  (0),
 file_     (0),
 editMode_ (VI_EDIT_MODE),
 selMode_  (RANGE_SEL_MODE),
 normalKey_(0),
 viKey_    (0),
 number_   (false)
{
  setObjectName("file");

  //setFont(QFont("Courier", 8));
  setFont(QFont("Inconsolata", 8));

  file_ = new CTextFile;

  QGridLayout *grid = new QGridLayout(this);
  grid->setMargin(2); grid->setSpacing(2);

  canvas_ = new CQTextFileCanvas(this);

  connect(canvas_, SIGNAL(sizeChanged(int,int)), this, SIGNAL(sizeChanged(int,int)));

  hscroll_ = new QScrollBar(Qt::Horizontal);
  vscroll_ = new QScrollBar(Qt::Vertical  );

  hscroll_->setObjectName("hscroll");
  vscroll_->setObjectName("vscroll");

  connect(hscroll_, SIGNAL(valueChanged(int)), this, SLOT(hscrollSlot()));
  connect(vscroll_, SIGNAL(valueChanged(int)), this, SLOT(vscrollSlot()));

  grid->addWidget(canvas_ , 0, 0);
  grid->addWidget(hscroll_, 1, 0);
  grid->addWidget(vscroll_, 0, 1);

  normalKey_ = new CTextFileNormalKey(file_);
  viKey_     = new CTextFileViKey(file_);

  normalKey_->addNotifier(this);
  viKey_    ->addNotifier(this);

  normalKey_->getSelection()->addNotifier(this);
  viKey_    ->getSelection()->addNotifier(this);

  setFocusProxy(canvas_);
}

CTextFileKey *
CQTextFile::
getKey() const
{
  if (editMode_ == VI_EDIT_MODE)
    return viKey_;
  else
    return normalKey_;
}

void
CQTextFile::
loadFile(const char *fileName)
{
  file_->read(fileName);

  getKey()->getUndo()->reset();
}

void
CQTextFile::
setEditMode(EditMode mode)
{
  editMode_ = mode;

  setSelMode(selMode_);
}

void
CQTextFile::
setSelMode(SelMode mode)
{
  selMode_ = mode;

  CTextFileSel *sel = getKey()->getSelection();

  if (mode == RANGE_SEL_MODE)
    sel->setMode(CTextFileSel::RANGE_SEL_MODE);
  else
    sel->setMode(CTextFileSel::RECT_SEL_MODE);

  canvas_->forceUpdate();
}

void
CQTextFile::
notifyScrollTop()
{
  scrollToPos(CSCROLL_TYPE_TOP);
}

void
CQTextFile::
notifyScrollMiddle()
{
  scrollToPos(CSCROLL_TYPE_CENTER);
}

void
CQTextFile::
notifyScrollBottom()
{
  scrollToPos(CSCROLL_TYPE_BOTTOM);
}

void
CQTextFile::
notifyNumber(bool number)
{
  setNumber(number);
}

void
CQTextFile::
setNumber(bool number)
{
  number_ = number;

  canvas_->forceUpdate();
}

void
CQTextFile::
scrollToPos(CScrollType type)
{
  canvas_->scrollTo(type);
}

void
CQTextFile::
hscrollSlot()
{
  canvas_->forceUpdate();
}

void
CQTextFile::
vscrollSlot()
{
  canvas_->forceUpdate();
}

void
CQTextFile::
selectionChanged(const std::string &str)
{
  QClipboard *clipboard = QApplication::clipboard();

  clipboard->setText(str.c_str(), QClipboard::Selection);
}

//----------

CQTextFileCanvas::
CQTextFileCanvas(CQTextFile *textFile) :
 textFile_     (textFile),
 x_offset_     (0),
 y_offset_     (0),
 char_rect_    (),
 char_width_   (1),
 char_height_  (1),
 char_ascent_  (1),
 maxLineLen_   (0),
 cursorX_      (0),
 cursorY_      (0),
 isSelected_   (false),
 pressed_      (false),
 press_xc_     (0),
 press_yc_     (0),
 motion_xc_    (0),
 motion_yc_    (0),
 draw_xmin_    (0),
 draw_ymin_    (0),
 draw_xmax_    (0),
 draw_ymax_    (0),
 last_x_       (0),
 last_y_       (0),
 scroll_update_(true),
 qimage_       (),
 painter_      (0),
 force_update_ (true),
 damaged_      (false),
 damage_y1_    (0),
 damage_y2_    (0)
{
  setObjectName("canvas");

  setMouseTracking(true);

  setFocusPolicy(Qt::StrongFocus);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  setFont(textFile->font());

  QFontMetrics fm(font());

  char_rect_   = fm.boundingRect("X");
  char_width_  = fm.width("X");
  char_ascent_ = fm.ascent();
  char_height_ = fm.ascent() + fm.descent() + 1;

  CTextFile *file = textFile_->getFile();

  file->addNotifier(this);

  //setFixedSize(100*char_width_, 60*char_height_);
}

void
CQTextFileCanvas::
paintEvent(QPaintEvent *)
{
  QColor bg = (hasFocus() ? QColor(255,255,255) : QColor(240,240,240));
  QColor fg = (hasFocus() ? QColor(0  ,0  ,0  ) : QColor( 40, 40, 40));
  QColor tc = (hasFocus() ? QColor(0  ,0  ,200) : QColor( 40, 40,200));

  if (! painter_) {
    painter_ = new QPainter(&qimage_);

    painter_->setFont(font());
  }

  if (force_update_)
    damaged_ = false;

  if (! damaged_)
    painter_->fillRect(rect(), bg);

  //------

  QScrollBar *hscroll = textFile_->getHScroll();
  QScrollBar *vscroll = textFile_->getVScroll();

  x_offset_ = hscroll->value();
  y_offset_ = vscroll->value();

  int x = -x_offset_;
  int y = -y_offset_;

  draw_xmin_ = -char_height_;
  draw_ymin_ = -char_width_ ;
  draw_xmax_ = width ();
  draw_ymax_ = height();

  CTextFile *file = textFile_->getFile();

  uint numLines = file->getNumLines();

  bool number = textFile_->getNumber();

  int xm = 0;

  if (number)
    xm = (int(log10(std::max(1U, numLines))) + 1)*char_width_ + 3;

  x += xm;

  file->getPos(&cursorX_, &cursorY_);

  CTextFileSel *sel = textFile_->getKey()->getSelection();

  isSelected_ = sel->isSelected();

  maxLineLen_ = 0;

  CTextFile::LineIterator pl1, pl2;

  for (pl1 = file->beginLine(), pl2 = file->endLine(); pl1 != pl2; ++pl1) {
    const std::string &line = *pl1;

    uint row = pl1.lineNum();

    bool draw = (y > draw_ymin_ && y < draw_ymax_);

    if (draw && damaged_)
      draw = (row >= damage_y1_ && row <= damage_y2_);

    if (draw) {
      if (damaged_) {
        QRect rect(0, y, width(), char_height_);

        painter_->fillRect(rect, bg);
      }

      painter_->setPen(fg);

      drawLine(painter_, x, y, row, line);

      if (number) {
        painter_->setPen(tc);

        std::string str = CStrUtil::strprintf("%d", row + 1);

        QRect numberRect(0, y, xm, char_height_);

        painter_->fillRect(numberRect, bg);

        painter_->drawText(0, y + char_ascent_, str.c_str());
      }
    }

    y += char_height_;

    maxLineLen_ = std::max(maxLineLen_, uint(line.size()));
  }

  if (number) {
    painter_->setPen(QColor(200,200,200));

    painter_->drawLine(xm - 2, 0, xm - 2, height() - 1);
  }

  //-----

  if (scroll_update_)
    updateScrollBars();

  //-----

  int y1 = y_offset_;
  int y2 = y1 + vscroll->pageStep();

  file->setPageTop   (y1 / char_height_);
  file->setPageBottom(y2 / char_height_ - 1);

  //-----

  QPainter painter(this);

  painter.drawImage(0, 0, qimage_);

  damaged_      = false;
  force_update_ = false;
}

void
CQTextFileCanvas::
drawLine(QPainter *painter, int x, int y, uint row, const std::string &line)
{
  CTextFileSel *sel = textFile_->getKey()->getSelection();

  bool line_part_sel = sel->isPartLineInside(row);
  bool line_sel      = sel->isLineInside(row);

  char cstr[2];

  cstr[1] = '\0';

  uint len = line.size();

  for (uint col = 0; col < len; ++col) {
    cstr[0] = line[col];

    if (x > draw_xmin_ && x < draw_xmax_) {
      int cs = 1;

      if (cstr[0] == '\t')
        cs = 8 - (col % 8);

      bool char_sel = line_sel;

      if (! line_sel && line_part_sel) {
        CIPoint2D pos(col, row);

        char_sel = sel->isCharInside(row, col);
      }

      if (char_sel)
        painter->fillRect(QRect(x, y, cs*char_width_, char_height_), QColor(255,255,0));

      if (row == cursorY_ && col == cursorX_) {
        QRect cursor_rect(x, y, char_width_, char_height_);

        painter_->fillRect(cursor_rect, CQUtil::rgbaToColor(CRGBA(0,1,0)));
      }

      if (cstr[0] >= ' ' && cstr[0] <= '~') {
        painter->drawText(x, y + char_ascent_, cstr);
      }
      else if (cstr[0] == '\t') {
        for (int i = 0; i < cs; ++i) {
          if (i > 0) x += char_width_;

          painter->drawText(x, y + char_ascent_, " ");
        }
      }
      else {
        cstr[0] = '?';

        painter->setPen(QColor(255,0,0));

        painter->drawText(x, y + char_ascent_, cstr);

        painter->setPen(QColor(0,0,0));
      }
    }

    x += char_width_;
  }

  if (row == cursorY_ && len == cursorX_) {
    QRect cursor_rect(x, y, char_width_, char_height_);

    painter_->fillRect(cursor_rect, CQUtil::rgbaToColor(CRGBA(0,1,0)));
  }
}

void
CQTextFileCanvas::
resizeEvent(QResizeEvent *)
{
  delete painter_;

  painter_ = 0;

  qimage_ = QImage(QSize(width(), height()), QImage::Format_ARGB32);

  damaged_      = false;
  force_update_ = false;

  scroll_update_ = true;

  int cols = width ()/char_width_;
  int rows = height()/char_height_;

  emit sizeChanged(rows, cols);
}

void
CQTextFileCanvas::
updateScrollBars()
{
  CTextFile *file = textFile_->getFile();

  QScrollBar *hscroll = textFile_->getHScroll();
  QScrollBar *vscroll = textFile_->getVScroll();

  int w = width ();
  int h = height();

  int aw = maxLineLen_*char_width_;
  int ah = file->getNumLines()*char_height_;

  if (textFile_->getNumber())
    aw += (int(log10(std::max(1U, file->getNumLines()))) + 1)*char_width_ + 3;

  int hs = std::min(w, aw);
  int vs = std::min(h, ah);

  hscroll->setMinimum(0);
  hscroll->setMaximum(aw - hs);
  hscroll->setPageStep(hs);
  hscroll->setSingleStep(char_width_);

  vscroll->setMinimum(0);
  vscroll->setMaximum(ah - vs);
  vscroll->setPageStep(vs);
  vscroll->setSingleStep(char_height_);

  scroll_update_ = false;
}

void
CQTextFileCanvas::
scrollTo(CScrollType type)
{
  CTextFile *file = textFile_->getFile();

  uint x, y;

  file->getPos(&x, &y);

  int x1 = x*char_width_  - x_offset_;
  int y1 = y*char_height_ - y_offset_;

  CIBBox2D bbox(x1, y1, x1 + char_width_, y1 + char_height_);

  QScrollBar *vscroll = textFile_->getVScroll();

  uint num_rows = height()/char_height_;

  // scroll so current line is on the top
  if      (type == CSCROLL_TYPE_TOP) {
    int y1 = y;

    if (y1 + num_rows > file->getNumLines())
      y1 = file->getNumLines() - num_rows;

    vscroll->setValue(y1*char_height_);

    forceUpdate();
  }
  // scroll so current line is on the bottom
  else if (type == CSCROLL_TYPE_BOTTOM) {
    int y1 = y - num_rows + 1;

    if (y1 < 0) y1 = 0;

    vscroll->setValue(y1*char_height_);

    forceUpdate();
  }
  // scroll so current line is visible
  else if (type == CSCROLL_TYPE_VISIBLE) {
    int py = bbox.getYMid();

    if (py <  0                  ) py = 0;
    if (py >= vscroll->pageStep()) py = vscroll->pageStep() - 1;

    int line_num = py/int(char_height_);

    int line_num1 = y/int(char_height_);
    int line_num2 = line_num1 + num_rows - 1;

    if      (line_num < line_num1) {
      vscroll->setValue(line_num*char_height_);

      forceUpdate();
    }
    else if (line_num > line_num2) {
      vscroll->setValue((num_rows - 1 - line_num)*char_height_);

      forceUpdate();
    }
  }
}

void
CQTextFileCanvas::
mousePressEvent(QMouseEvent *e)
{
  eventPosToChar(e->pos(), press_xc_, press_yc_);

  CTextFile *file = textFile_->getFile();

  file->moveTo(press_xc_, press_yc_);

  CTextFileSel *sel = textFile_->getKey()->getSelection();

  sel->rangeSelect(press_yc_, press_xc_, press_yc_, press_xc_, true);

  pressed_ = true;
}

void
CQTextFileCanvas::
mouseReleaseEvent(QMouseEvent *e)
{
  eventPosToChar(e->pos(), motion_xc_, motion_yc_);

  CTextFile *file = textFile_->getFile();

  file->moveTo(motion_xc_, motion_yc_);

  CTextFileSel *sel = textFile_->getKey()->getSelection();

  sel->rangeSelect(press_yc_, press_xc_, motion_yc_, motion_xc_, true);

  pressed_ = false;
}

void
CQTextFileCanvas::
mouseMoveEvent(QMouseEvent *e)
{
  if (! pressed_) return;

  eventPosToChar(e->pos(), motion_xc_, motion_yc_);

  CTextFile *file = textFile_->getFile();

  file->moveTo(motion_xc_, motion_yc_);

  CTextFileSel *sel = textFile_->getKey()->getSelection();

  sel->rangeSelect(press_yc_, press_xc_, motion_yc_, motion_xc_, true);
}

void
CQTextFileCanvas::
mouseDoubleClickEvent(QMouseEvent *)
{
}

void
CQTextFileCanvas::
keyPressEvent(QKeyEvent *e)
{
  CKeyEvent *event = CQUtil::convertEvent(e);

  textFile_->getKey()->processChar(event->getType(), event->getText(), event->getModifier());
}

void
CQTextFileCanvas::
eventPosToChar(const QPoint &pos, int &xc, int &yc)
{
  int x = pos.x();
  int y = pos.y();

  x += x_offset_;
  y += y_offset_;

  yc = y / char_height_; // all lines single char height

  CTextFile *file = textFile_->getFile();

  int nl = file->getNumLines();

  if (yc >= nl)
    yc = nl - 1;

  if (x < 0) {
    xc = 0;
    return;
  }

  const std::string &line = file->getLine(yc);

  uint nc = line.size();

  int x1 = 0;

  for (uint i = 0; i < nc; ++i) {
    int cs = 1;

    if (line[i] == '\t')
      cs = 8 - (i % 8);

    int x2 = x1 + cs*char_width_;

    if (x >= x1 && x < x2) {
      xc = i;
      return;
    }

    x1 = x2;
  }

  xc = nc - 1;
}

void
CQTextFileCanvas::
positionChanged(uint x, uint y)
{
  if (x == last_x_ && y == last_y_) return;

  uint xpos_min = x*char_width_;
  uint ypos_min = y*char_height_;
  uint xpos_max = xpos_min + char_width_;
  uint ypos_max = ypos_min + char_height_;

  QScrollBar *hscroll = textFile_->getHScroll();
  QScrollBar *vscroll = textFile_->getVScroll();

  int xmin = hscroll->value();
  int xmax = xmin + hscroll->pageStep();
  int ymin = vscroll->value();
  int ymax = ymin + vscroll->pageStep();

  if (int(xpos_min) <= xmin) hscroll->setValue(xpos_min);
  if (int(xpos_max) >= xmax) hscroll->setValue(xpos_max - hscroll->pageStep());
  if (int(ypos_min) <= ymin) vscroll->setValue(ypos_min);
  if (int(ypos_max) >= ymax) vscroll->setValue(ypos_max - vscroll->pageStep());

  if (! damaged_) {
    damaged_   = true;
    damage_y1_ = std::min(y, last_y_);
    damage_y2_ = std::max(y, last_y_);
  }
  else {
    damage_y1_ = std::min(y, damage_y1_);
    damage_y2_ = std::max(y, damage_y2_);
  }

  last_x_ = x;
  last_y_ = y;

  update();
}

void
CQTextFileCanvas::
linesCleared()
{
  forceUpdate();
}

void
CQTextFileCanvas::
lineAdded(const std::string &, uint)
{
  scroll_update_ = true;

  forceUpdate();
}

void
CQTextFileCanvas::
lineReplaced(const std::string &, const std::string &, uint)
{
  scroll_update_ = true;

  forceUpdate();
}

void
CQTextFileCanvas::
lineDeleted(const std::string &, uint)
{
  scroll_update_ = true;

  forceUpdate();
}

void
CQTextFileCanvas::
charAdded(char, uint, uint)
{
  scroll_update_ = true;

  forceUpdate();
}

void
CQTextFileCanvas::
charReplaced(char, char, uint, uint)
{
  scroll_update_ = true;

  forceUpdate();
}

void
CQTextFileCanvas::
charDeleted(char, uint, uint)
{
  scroll_update_ = true;

  forceUpdate();
}

void
CQTextFileCanvas::
forceUpdate()
{
  force_update_ = true;

  update();
}

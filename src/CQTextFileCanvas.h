#ifndef CQTextFileCanvas_H
#define CQTextFileCanvas_H

#include <CQWindow.h>
#include <CTextFile.h>
#include <CScrollType.h>

class CQTextFile;

class CQTextFileCanvas : public CQWindow, public CTextFileNotifier {
  Q_OBJECT

 public:
  CQTextFileCanvas(CQTextFile *textFile);

  void paintEvent(QPaintEvent *);

  void drawLine(QPainter *painter, int x, int y, uint row, const std::string &str);

  void resizeEvent(QResizeEvent *);

  void updateScrollBars();

  void scrollTo(CScrollType type);

  void mousePressEvent  (QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
  void mouseMoveEvent   (QMouseEvent *);

  void mouseDoubleClickEvent(QMouseEvent *);

  void keyPressEvent(QKeyEvent *e);

  void eventPosToChar(const QPoint &pos, int &xc, int &yc);

  void positionChanged(uint x, uint y);

  void linesCleared();

  void lineAdded   (const std::string &line, uint line_num);
  void lineDeleted (const std::string &line, uint line_num);
  void lineReplaced(const std::string &line1, const std::string &line2, uint line_num);
  void charAdded   (char c, uint line_num, uint char_num);
  void charDeleted (char c, uint line_num, uint char_num);
  void charReplaced(char c1, char c2, uint line_num, uint char_num);

  void forceUpdate();

 signals:
  void sizeChanged(int rows, int cols);

 private:
  CQTextFile *textFile_;
  uint        x_offset_, y_offset_;
  QRect       char_rect_;
  uint        char_width_, char_height_, char_ascent_;
  uint        maxLineLen_;
  uint        cursorX_, cursorY_;
  bool        isSelected_;
  bool        pressed_;
  int         press_xc_, press_yc_, motion_xc_, motion_yc_;
  int         draw_xmin_, draw_ymin_, draw_xmax_, draw_ymax_;
  uint        last_x_, last_y_;
  bool        scroll_update_;
  QImage      qimage_;
  QPainter   *painter_;
  bool        force_update_;
  bool        damaged_;
  uint        damage_y1_, damage_y2_;
};

#endif

#include <CTextFileSel.h>
#include <CTextFile.h>

CTextFileSel::
CTextFileSel(CTextFile *file) :
 file_    (file),
 selected_(false),
 selMode_ (RANGE_SEL_MODE),
 start_   (),
 end_     ()
{
  notifyMgr_ = new CTextFileSelNotifierMgr(this);
}

void
CTextFileSel::
addNotifier(CTextFileSelNotifier *notifier)
{
  notifyMgr_->addNotifier(notifier);
}

void
CTextFileSel::
removeNotifier(CTextFileSelNotifier *notifier)
{
  notifyMgr_->removeNotifier(notifier);
}

void
CTextFileSel::
clearSelection()
{
  if (selected_) {
    selected_ = false;

    notifySelectionChanged();
  }
}

void
CTextFileSel::
setMode(SelMode mode)
{
  if (mode != selMode_) {
    selMode_ = mode;

    notifySelectionChanged();
  }
}

void
CTextFileSel::
selectBBox(const CIBBox2D & /*bbox*/, bool /*clear*/)
{
}

void
CTextFileSel::
selectInside(const CIBBox2D & /*bbox*/, bool /*clear*/)
{
}

void
CTextFileSel::
rangeSelect(const CIBBox2D & /*bbox*/, bool /*clear*/)
{
}

void
CTextFileSel::
rangeSelect(int row1, int col1, int row2, int col2, bool clear)
{
  rangeSelect(CIPoint2D(col1, row1), CIPoint2D(col2, row2), clear);
}

void
CTextFileSel::
rangeSelect(const CIPoint2D &start, const CIPoint2D &end, bool clear)
{
  if (end.y < start.y || (end.y == start.y && end.x < start.x)) {
    rangeSelect(end, start, clear);
    return;
  }

  bool      oldSel   = selected_;
  CIPoint2D oldStart = start_;
  CIPoint2D oldEnd   = end_;

  if (clear) selected_ = false;

  start_    = start;
  end_      = end;
  selected_ = isValid();

  bool changed = false;

  if (selected_ != oldSel || start_ != oldStart || end_ != oldEnd)
    changed = true;

  if (changed)
    notifySelectionChanged();
}

void
CTextFileSel::
selectChar(int row, int col)
{
  CIPoint2D start(col, row);

  CIPoint2D end(start_.x + 1, start.y);

  rangeSelect(start, end, true);
}

void
CTextFileSel::
selectLine(int row)
{
  const std::string &line = file_->getLine(row);

  CIPoint2D start(              0, row);
  CIPoint2D end  (line.size() - 1, row);

  rangeSelect(start, end, true);
}

void
CTextFileSel::
setSelectionColor(const CRGBA & /*color*/)
{
}

void
CTextFileSel::
setSelectRange(const CIPoint2D &start, const CIPoint2D &end)
{
  rangeSelect(start, end, true);
}

const CIPoint2D &
CTextFileSel::
getSelectStart() const
{
  return start_;
}

const CIPoint2D &
CTextFileSel::
getSelectEnd() const
{
  return end_;
}

std::string
CTextFileSel::
getSelectedText() const
{
  std::string sel;

  if (selMode_ == RANGE_SEL_MODE) {
    const std::string &line1 = file_->getLine(start_.y);

    int len1 = line1.size();

    if (len1 > 0) {
      int x1 = (start_.x < len1 ? start_.x : 0);

      sel += line1.substr(x1);
    }

    for (int row = start_.y + 1; row <= end_.y - 1; ++row)
      sel += "\n" + file_->getLine(row);

    const std::string &line2 = file_->getLine(end_.y);

    int len2 = line2.size();

    if (len2 > 0) {
      int x2 = (end_.x + 1 < len2 ? end_.x + 1 : len2);

      sel += "\n" + line2.substr(0, x2);
    }
  }
  else {
    for (int row = start_.y; row <= end_.y; ++row) {
      const std::string &line = file_->getLine(row);

      int len = line.size();

      if (row > start_.y) sel += "\n";

      if (len <= 0) continue;

      int x1 = std::min(start_.x, end_.x);
      int x2 = std::max(start_.x, end_.x);

      if (x1 < 0) x1 = 0;

      if      (x1 >= len) { x1 = len - 1; x2 = x1; }
      else if (x2 >= len) x2 = len - 1;

      sel += line.substr(x1, x2 - x1 + 1);
    }
  }

  return sel;
}

bool
CTextFileSel::
isLineInside(uint row) const
{
  if (! selected_) return false;

  if (selMode_ == RANGE_SEL_MODE)
    return (int(row) > start_.y && int(row) < end_.y);
  else
    return false;
}

bool
CTextFileSel::
isPartLineInside(uint row) const
{
  if (! selected_) return false;

  if (selMode_ == RANGE_SEL_MODE)
    return (int(row) >= start_.y && int(row) <= end_.y);
  else
    return (int(row) >= start_.y && int(row) <= end_.y);
}

bool
CTextFileSel::
isCharInside(uint row, int col) const
{
  if (! selected_) return false;

  if (selMode_ == RANGE_SEL_MODE) {
    if (start_.y == end_.y)
      return (int(row) == start_.y &&
              int(col) >= start_.x && int(col) <= end_  .x);
    else
      return (int(row) == start_.y && int(col) >= start_.x) ||
             (int(row) == end_  .y && int(col) <= end_  .x);
  }
  else {
    return (int(row) >= start_.y && int(row) <= end_  .y &&
            int(col) >= start_.x && int(col) <= end_  .x);
  }
}

bool
CTextFileSel::
insideSelection(const CIPoint2D &pos) const
{
  if (! selected_) return false;

  if (selMode_ == RANGE_SEL_MODE) {
    if (start_.y == end_.y)
      return (pos.y == start_.y && pos.x >= start_.x && pos.x <= end_.x);
    else {
      if      (pos.y == start_.y)
        return pos.x >= start_.x;
      else if (pos.y == end_  .y)
        return pos.x <= end_  .x;
      else
        return (pos.y > start_.y && pos.y < end_.y);
    }
  }
  else {
    return (pos.y >= start_.y && pos.y <= end_.y &&
            pos.x >= start_.x && pos.x <= end_.x);
  }
}

bool
CTextFileSel::
isValid() const
{
  if (selMode_ == RANGE_SEL_MODE)
    return cmp(start_, end_) < 0;
  else
    return true;
}

int
CTextFileSel::
cmp(const CIPoint2D &p1, const CIPoint2D &p2)
{
  if (p1.y == p2.y)
    return (p1.x - p2.x);
  else
    return (p1.y - p2.y);
}

void
CTextFileSel::
notifySelectionChanged()
{
  notifyMgr_->selectionChanged(getSelectedText());

  //cerr << getSelectedText() << endl;
}

//------

CTextFileSelNotifierMgr::
CTextFileSelNotifierMgr(CTextFileSel *sel) :
 sel_(sel)
{
}

void
CTextFileSelNotifierMgr::
addNotifier(CTextFileSelNotifier *notifier)
{
  notifierList_.push_back(notifier);
}

void
CTextFileSelNotifierMgr::
removeNotifier(CTextFileSelNotifier *notifier)
{
  notifierList_.remove(notifier);
}

void
CTextFileSelNotifierMgr::
selectionChanged(const std::string &str)
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->selectionChanged(str);
}

//------

CTextFileSelNotifier::
CTextFileSelNotifier()
{
}

void
CTextFileSelNotifier::
selectionChanged(const std::string &)
{
}

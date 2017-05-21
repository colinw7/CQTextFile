#include <CTextFileKey.h>
#include <CTextFile.h>
#include <CTextFileUndo.h>
#include <CTextFileSel.h>
#include <CTextFileUtil.h>

CTextFileKey::
CTextFileKey(CTextFile *file) :
 file_       (file),
 undo_       (nullptr),
 overwrite_  (false),
 tab_stop_   (8),
 findPattern_("")
{
  undo_ = new CTextFileUndo(file);
  sel_  = new CTextFileSel (file);
  util_ = new CTextFileUtil(file);

  notifyMgr_ = new CTextFileKeyNotifierMgr(this);
}

CTextFileKey::
~CTextFileKey()
{
  delete undo_;
  delete sel_;
  delete util_;
  delete notifyMgr_;
}

void
CTextFileKey::
setOverwrite(bool overwrite)
{
  overwrite_ = overwrite;

  notifyOverwrite(overwrite_);
}

void
CTextFileKey::
addNotifier(CTextFileKeyNotifier *notifier)
{
  notifyMgr_->addNotifier(notifier);
}

void
CTextFileKey::
removeNotifier(CTextFileKeyNotifier *notifier)
{
  notifyMgr_->removeNotifier(notifier);
}

void
CTextFileKey::
enterCmdLineMode(const std::string &cmd)
{
  notifyMgr_->enterCmdLineMode(cmd);
}

void
CTextFileKey::
showOverlayMsg(const std::string &msg)
{
  notifyMgr_->showOverlayMsg(msg);
}

void
CTextFileKey::
showStatusMsg(const std::string &msg)
{
  notifyMgr_->showStatusMsg(msg);
}

void
CTextFileKey::
notifyScrollTop()
{
  notifyMgr_->notifyScrollTop();
}

void
CTextFileKey::
notifyScrollMiddle()
{
  notifyMgr_->notifyScrollMiddle();
}

void
CTextFileKey::
notifyScrollBottom()
{
  notifyMgr_->notifyScrollBottom();
}

void
CTextFileKey::
notifyOverwrite(bool overwrite)
{
  notifyMgr_->notifyOverwrite(overwrite);
}

void
CTextFileKey::
notifyNumber(bool number)
{
  notifyMgr_->notifyNumber(number);
}

void
CTextFileKey::
notifyQuit()
{
  notifyMgr_->notifyQuit();
}

void
CTextFileKey::
extendSelectLeft(int n)
{
  CIPoint2D pos1 = getPos();

  if (! sel_->insideSelection(pos1))
    sel_->clearSelection();

  file_->rmoveTo(-n, 0);

  CIPoint2D pos2 = getPos();

  if (sel_->isSelected()) {
    const CIPoint2D &start = sel_->getSelectStart();
    const CIPoint2D &end   = sel_->getSelectEnd  ();

    if      (CTextFileSel::cmp(pos2, start) < 0)
      sel_->setSelectRange(pos2 , end );
    else if (CTextFileSel::cmp(pos2, end  ) < 0)
      sel_->setSelectRange(start, pos2);
  }
  else
    sel_->setSelectRange(pos2, pos1);
}

void
CTextFileKey::
extendSelectRight(int n)
{
  CIPoint2D pos1 = getPos();

  if (! sel_->insideSelection(pos1))
    sel_->clearSelection();

  file_->rmoveTo(n, 0);

  CIPoint2D pos2 = getPos();

  if (sel_->isSelected()) {
    const CIPoint2D &start = sel_->getSelectStart();
    const CIPoint2D &end   = sel_->getSelectEnd  ();

    if      (CTextFileSel::cmp(pos2, end  ) > 0)
      sel_->setSelectRange(start, pos2);
    else if (CTextFileSel::cmp(pos2, start) > 0)
      sel_->setSelectRange(pos2 , end );
  }
  else
    sel_->setSelectRange(pos1, pos2);
}

void
CTextFileKey::
extendSelectUp(int n)
{
  CIPoint2D pos1 = getPos();

  if (! sel_->insideSelection(pos1))
    sel_->clearSelection();

  file_->rmoveTo(0, -n);

  CIPoint2D pos2 = getPos();

  if (sel_->isSelected()) {
    const CIPoint2D &start = sel_->getSelectStart();
    const CIPoint2D &end   = sel_->getSelectEnd  ();

    if      (CTextFileSel::cmp(pos2, start) < 0)
      sel_->setSelectRange(pos2 , end );
    else if (CTextFileSel::cmp(pos2, end  ) < 0)
      sel_->setSelectRange(start, pos2);
  }
  else
    sel_->setSelectRange(pos2, pos1);
}

void
CTextFileKey::
extendSelectDown(int n)
{
  CIPoint2D pos1 = getPos();

  if (! sel_->insideSelection(pos1))
    sel_->clearSelection();

  file_->rmoveTo(0, n);

  CIPoint2D pos2 = getPos();

  if (sel_->isSelected()) {
    const CIPoint2D &start = sel_->getSelectStart();
    const CIPoint2D &end   = sel_->getSelectEnd  ();

    if      (CTextFileSel::cmp(pos2, end  ) > 0)
      sel_->setSelectRange(start, pos2);
    else if (CTextFileSel::cmp(pos2, start) > 0)
      sel_->setSelectRange(pos2 , end );
  }
  else
    sel_->setSelectRange(pos1, pos2);
}

bool
CTextFileKey::
findNext()
{
  return findNext(findPattern_);
}

bool
CTextFileKey::
findNext(const std::string &pattern)
{
  uint fline_num, fchar_num;

  if (! util_->findNext(pattern, getRow(), getCol() + 1, file_->getNumLines() - 1, -1,
                        &fline_num, &fchar_num))
    return false;

  file_->moveTo(fchar_num, fline_num);

  findPattern_ = pattern;

  return true;
}

bool
CTextFileKey::
findPrev()
{
  return findPrev(findPattern_);
}

bool
CTextFileKey::
findPrev(const std::string &pattern)
{
  uint fline_num, fchar_num;

  if (! util_->findPrev(pattern, getRow(), getCol() - 1, 0, 0, &fline_num, &fchar_num))
    return false;

  file_->moveTo(fchar_num, fline_num);

  findPattern_ = pattern;

  return true;
}

CIPoint2D
CTextFileKey::
getPos() const
{
  uint x, y;

  file_->getPos(&x, &y);

  return CIPoint2D(x, y);
}

uint
CTextFileKey::
getRow() const
{
  uint x, y;

  file_->getPos(&x, &y);

  return y;
}

uint
CTextFileKey::
getCol() const
{
  uint x, y;

  file_->getPos(&x, &y);

  return x;
}

uint
CTextFileKey::
getPageLength() const
{
  int len = file_->getPageBottom() - file_->getPageTop() + 1;

  return std::max(0, len);
}

//------

CTextFileKeyNotifierMgr::
CTextFileKeyNotifierMgr(CTextFileKey *key) :
 key_(key)
{
}

void
CTextFileKeyNotifierMgr::
addNotifier(CTextFileKeyNotifier *notifier)
{
  notifierList_.push_back(notifier);
}

void
CTextFileKeyNotifierMgr::
removeNotifier(CTextFileKeyNotifier *notifier)
{
  notifierList_.remove(notifier);
}

void
CTextFileKeyNotifierMgr::
enterCmdLineMode(const std::string &msg)
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->enterCmdLineMode(msg);
}

void
CTextFileKeyNotifierMgr::
showOverlayMsg(const std::string &cmd)
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->showOverlayMsg(cmd);
}

void
CTextFileKeyNotifierMgr::
showStatusMsg(const std::string &cmd)
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->showStatusMsg(cmd);
}

void
CTextFileKeyNotifierMgr::
notifyScrollTop()
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->notifyScrollTop();
}

void
CTextFileKeyNotifierMgr::
notifyScrollMiddle()
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->notifyScrollMiddle();
}

void
CTextFileKeyNotifierMgr::
notifyScrollBottom()
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->notifyScrollBottom();
}

void
CTextFileKeyNotifierMgr::
notifyOverwrite(bool overwrite)
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->notifyOverwrite(overwrite);
}

void
CTextFileKeyNotifierMgr::
notifyNumber(bool number)
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->notifyNumber(number);
}

void
CTextFileKeyNotifierMgr::
notifyQuit()
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->notifyQuit();
}

//------

CTextFileKeyNotifier::
CTextFileKeyNotifier()
{
}

void
CTextFileKeyNotifier::
enterCmdLineMode(const std::string &)
{
}

void
CTextFileKeyNotifier::
showOverlayMsg(const std::string &)
{
}

void
CTextFileKeyNotifier::
showStatusMsg(const std::string &)
{
}

void
CTextFileKeyNotifier::
notifyScrollTop()
{
}

void
CTextFileKeyNotifier::
notifyScrollMiddle()
{
}

void
CTextFileKeyNotifier::
notifyScrollBottom()
{
}

void
CTextFileKeyNotifier::
notifyOverwrite(bool)
{
}

void
CTextFileKeyNotifier::
notifyNumber(bool)
{
}

void
CTextFileKeyNotifier::
notifyQuit()
{
}

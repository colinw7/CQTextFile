#include <CTextFileMarks.h>
#include <CTextFile.h>

void
CTextFileMarks::
markReturn()
{
  setMarkPos("");
}

void
CTextFileMarks::
setMarkPos(const std::string &mark)
{
  uint x, y;

  file_->getPos(&x, &y);

  setMarkPos(mark, y, x);
}

void
CTextFileMarks::
setMarkPos(const std::string &mark, uint line_num, uint char_num)
{
  marks_[mark] = CIPoint2D(char_num, line_num);
}

bool
CTextFileMarks::
getMarkPos(const std::string &mark, uint *line_num, uint *char_num) const
{
  MarkList::const_iterator p = marks_.find(mark);

  if (p == marks_.end())
    return false;

  CIPoint2D pos = (*p).second;

  if (pos.x < 0 && pos.y < 0)
    return false;

  *line_num = pos.y;
  *char_num = pos.x;

  return true;
}

void
CTextFileMarks::
unsetMarkPos(const std::string &mark)
{
  marks_[mark] = CIPoint2D(-1, -1);
}

void
CTextFileMarks::
clearLineMarks(uint line_num)
{
  MarkList::iterator p1 = marks_.begin();
  MarkList::iterator p2 = marks_.end  ();

  for ( ; p1 != p2; ++p1) {
    if ((*p1).second.y == (int) line_num)
      (*p1).second = CIPoint2D(-1, -1);
  }
}

void
CTextFileMarks::
displayMarks()
{
}

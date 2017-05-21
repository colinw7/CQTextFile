#include <CTextFileBuffer.h>
#include <CTextFile.h>
#include <CTextFileUtil.h>

CTextFileBuffer::
CTextFileBuffer(CTextFile *file) :
 file_(file)
{
  util_ = new CTextFileUtil(file_);
}

CTextFileBuffer::
~CTextFileBuffer()
{
  delete util_;
}

void
CTextFileBuffer::
yankLines(char id, uint n)
{
  uint x, y;

  file_->getPos(&x, &y);

  yankLines(id, y, n);
}

void
CTextFileBuffer::
yankLines(char id, uint line_num, uint n)
{
  yankClear(id);

  for (uint i = 0; i < n; ++i) {
    uint line_num1 = line_num + i;

    const std::string &line = file_->getLine(line_num1);

    uint len = line.size();

    subYankTo(id, line_num1, 0, line_num1, std::max(int(len) - 1, 0), true);
  }
}

void
CTextFileBuffer::
yankWords(char id, uint n)
{
  uint x, y;

  file_->getPos(&x, &y);

  yankWords(id, y, x, n);
}

void
CTextFileBuffer::
yankWords(char id, uint line_num, uint char_num, uint n)
{
  yankClear(id);

  uint line_num1 = line_num;
  uint char_num1 = char_num;

  for (uint i = 0; i < n; ++i)
    util_->endWord(&line_num1, &char_num1);

  subYankTo(id, line_num, char_num, line_num1, char_num1, false);
}

void
CTextFileBuffer::
yankChars(char id, uint n)
{
  uint x, y;

  file_->getPos(&x, &y);

  yankChars(id, y, x, n);
}

void
CTextFileBuffer::
yankChars(char id, uint line_num, uint char_num, uint n)
{
  yankClear(id);

  subYankTo(id, line_num, char_num, line_num, char_num + n - 1, false);
}

void
CTextFileBuffer::
yankTo(char id, uint line_num, uint char_num, bool is_line)
{
  uint x, y;

  file_->getPos(&x, &y);

  yankClear(id);

  subYankTo(id, y, x, line_num, char_num, is_line);
}

void
CTextFileBuffer::
yankTo(char id, uint line_num1, uint char_num1, uint line_num2, uint char_num2, bool is_line)
{
  yankClear(id);

  subYankTo(id, line_num1, char_num1, line_num2, char_num2, is_line);
}

void
CTextFileBuffer::
yankClear(char id)
{
  Buffer &buffer = getBuffer(id);

  buffer.clear();
}

void
CTextFileBuffer::
subYankTo(char id, uint line_num1, uint char_num1, uint line_num2, uint char_num2, bool is_line)
{
  std::vector<BufferLine> lines;

  if      (line_num1 < line_num2) {
    const std::string &line1 = file_->getLine(line_num1);

    lines.push_back(BufferLine(line1.substr(char_num1), is_line));

    for (uint i = line_num1 + 1; i < line_num2; ++i) {
      const std::string &line = file_->getLine(i);

      lines.push_back(BufferLine(line, true));
    }

    const std::string &line2 = file_->getLine(line_num2);

    lines.push_back(BufferLine(line2.substr(0, char_num2), is_line));
  }
  else if (line_num2 < line_num1) {
    const std::string &line2 = file_->getLine(line_num2);

    lines.push_back(BufferLine(line2.substr(char_num2), is_line));

    for (uint i = line_num2 + 1; i < line_num1; ++i) {
      const std::string &line = file_->getLine(i);

      lines.push_back(BufferLine(line, true));
    }

    const std::string &line1 = file_->getLine(line_num1);

    lines.push_back(BufferLine(line1.substr(0, char_num1), is_line));
  }
  else {
    const std::string &line1 = file_->getLine(line_num1);

    std::string line2;

    if (char_num1 < char_num2)
      line2 = line1.substr(char_num1, char_num2 - char_num1 + 1);
    else
      line2 = line1.substr(char_num2, char_num1 - char_num2 + 1);

    lines.push_back(BufferLine(line2, is_line));
  }

  std::vector<BufferLine>::iterator p1 = lines.begin();
  std::vector<BufferLine>::iterator p2 = lines.end  ();

  Buffer &buffer = getBuffer(id);

  for ( ; p1 != p2; ++p1)
    buffer.addLine((*p1).line, (*p1).newline);
}

void
CTextFileBuffer::
pasteAfter(char id)
{
  uint x, y;

  file_->getPos(&x, &y);

  pasteAfter(id, y, x);
}

void
CTextFileBuffer::
pasteAfter(char id, uint line_num, uint char_num)
{
  Buffer &buffer = getBuffer(id);

  uint num_lines = buffer.lines.size();

  if (num_lines == 0)
    return;

  BufferLine *sline = &buffer.lines[0];
  BufferLine *eline = NULL;

  if (num_lines > 1)
    eline = &buffer.lines[num_lines - 1];

  if (! sline->newline) {
    if (eline)
      splitLine(line_num, char_num);

    const std::string &line = file_->getLine(line_num);

    if (char_num < line.size())
      addChars(line_num, char_num + 1, sline->line);
    else
      addChars(line_num, char_num, sline->line);
  }
  else {
    ++line_num;

    addLine(line_num, sline->line);

    file_->rmoveTo(0, 1);

    cursorToLeft();

    ++line_num;
  }

  for (uint i = 1; i < num_lines - 1; ++i) {
    BufferLine *mline = &buffer.lines[i];

    addLine(line_num, mline->line);

    file_->rmoveTo(0, 1);

    cursorToLeft();

    ++line_num;
  }

  if (eline) {
    if (! eline->newline) {
      file_->rmoveTo(0, 1);

      cursorToLeft();

      ++line_num;

      addChars(line_num, 0, eline->line);
    }
    else {
      addLine(line_num, eline->line);

      file_->rmoveTo(0, 1);

      cursorToLeft();
    }
  }
}

void
CTextFileBuffer::
pasteBefore(char id)
{
  uint x, y;

  file_->getPos(&x, &y);

  pasteBefore(id, y, x);
}

void
CTextFileBuffer::
pasteBefore(char id, uint line_num, uint char_num)
{
  Buffer &buffer = getBuffer(id);

  uint num_lines = buffer.lines.size();

  if (num_lines == 0)
    return;

  BufferLine *sline = &buffer.lines[0];
  BufferLine *eline = NULL;

  if (num_lines > 1)
    eline = &buffer.lines[num_lines - 1];

  if (! sline->newline) {
    if (eline)
      splitLine(line_num, char_num);

    addChars(line_num, char_num, sline->line);
  }
  else
    addLine(line_num, sline->line);

  for (uint i = 1; i < num_lines - 1; ++i) {
    BufferLine *mline = &buffer.lines[i];

    addLine(line_num + i, mline->line);
  }

  if (eline) {
    if (! eline->newline)
      addChars(line_num + num_lines - 1, 0, eline->line);
    else
      addLine(line_num + num_lines - 1, eline->line);
  }
}

void
CTextFileBuffer::
cursorToLeft()
{
  uint x, y;

  file_->getPos(&x, &y);

  file_->moveTo(0, y);
}

void
CTextFileBuffer::
splitLine(uint line_num, uint char_num)
{
  uint x, y;

  file_->getPos(&x, &y);

  const std::string &line = file_->getLine(line_num);

  std::string line1 = line.substr(0, char_num);
  std::string line2 = line.substr(char_num);

  file_->moveTo(0, line_num);

  file_->replaceLine(line1);

  file_->addLineBefore(line2);

  file_->moveTo(x, y);
}

void
CTextFileBuffer::
addChars(uint line_num, uint char_num, const std::string &chars)
{
  uint x, y;

  file_->getPos(&x, &y);

  const std::string &line = file_->getLine(line_num);

  file_->moveTo(0, line_num);

  std::string line1 = line.substr(0, char_num);
  std::string line2 = line.substr(char_num);

  file_->replaceLine(line1 + chars + line2);

  file_->moveTo(x, y);
}

void
CTextFileBuffer::
addLine(uint line_num, const std::string &line)
{
  uint x, y;

  file_->getPos(&x, &y);

  file_->moveTo(0, line_num);

  file_->addLineBefore(line);

  file_->moveTo(x, y);
}

CTextFileBuffer::Buffer &
CTextFileBuffer::
getBuffer(char id)
{
  return buffer_map_[id];
}

#include <CTextFileUtil.h>
#include <CTextFile.h>
#include <CRegExp.h>
#include <CStrUtil.h>
#include <cstring>

CTextFileUtil::
CTextFileUtil(CTextFile *file) :
 file_(file), shiftWidth_(2)
{
}

void
CTextFileUtil::
deleteWord()
{
  uint line_num, char_num;

  file_->getPos(&char_num, &line_num);

  deleteWord(line_num, char_num);
}

void
CTextFileUtil::
deleteWord(uint line_num, uint char_num)
{
  const std::string &line = file_->getLine(line_num);

  if (line.empty() || char_num >= line.size() - 1)
    return;

  uint num = 0;

  if (isWordChar(line[char_num])) {
    while (int(char_num + num) < (int) line.size() - 1 &&
           isWordChar(line[char_num + num]))
      ++num;
  }
  else {
    while (int(char_num + num) < (int) line.size() - 1 &&
           ! isWordChar(line[char_num + num]))
      ++num;
  }

  if (num > 0)
    deleteChars(line_num, char_num, num);
}

void
CTextFileUtil::
deleteEOL()
{
  uint line_num, char_num;

  file_->getPos(&char_num, &line_num);

  deleteEOL(line_num, char_num);
}

void
CTextFileUtil::
deleteEOL(uint line_num, uint char_num)
{
  const std::string &line = file_->getLine(line_num);

  uint num = std::max(int(line.size()) - int(char_num), 0);

  if (num > 0)
    deleteChars(line_num, char_num, num);
}

void
CTextFileUtil::
shiftLeft(uint line_num1, uint line_num2)
{
  if (line_num1 > line_num2)
    std::swap(line_num1, line_num2);

  uint n = getShiftWidth();

  for (uint line_num = line_num1; line_num <= line_num2; ++line_num) {
    const std::string &line = file_->getLine(line_num);

    uint len = line.size();

    uint n1 = 0;

    for (uint j = 0; j < n && j < len; ++j) {
      char c = line[j];

      if (! isspace(c)) break;

      ++n1;
    }

    deleteChars(line_num, 0, n1);
  }
}

void
CTextFileUtil::
shiftRight(uint line_num1, uint line_num2)
{
  if (line_num1 > line_num2)
    std::swap(line_num1, line_num2);

  std::string chars;

  uint n = getShiftWidth();

  for (uint j = 0; j < n; ++j)
    chars += " ";

  for (uint line_num = line_num1; line_num <= line_num2; ++line_num) {
    const std::string &line = file_->getLine(line_num);

    if (line.empty()) continue;

    addChars(line_num, 0, chars);
  }
}

void
CTextFileUtil::
nextWord()
{
  uint x, y;

  file_->getPos(&x, &y);

  nextWord(&y, &x);

  file_->moveTo(x, y);
}

// a word a series of alphanumeric or _ characters OR
// a series of non-blank characters
void
CTextFileUtil::
nextWord(uint *line_num, uint *char_num)
{
  const std::string &line = file_->getLine(*line_num);

  // skip current word
  bool found = false;

  if (*char_num < line.size()) {
    if      (isWordChar(line[*char_num])) {
      while (*char_num < line.size() && isWordChar(line[*char_num]))
        ++(*char_num);
    }
    else if (! isspace(line[*char_num])) {
      while (*char_num < line.size() && ! isWordChar(line[*char_num]) &&
             ! isspace(line[*char_num]))
        ++(*char_num);
    }

    // skip space
    while (*char_num < line.size() && isspace(line[*char_num]))
      ++(*char_num);

    if (*char_num < line.size() && ! isspace(line[*char_num]))
      found = true;
  }

  // if no next word found then skip to next line
  while (! found) {
    if (*line_num >= file_->getNumLines() - 1)
      break;

    ++(*line_num);

    const std::string &fileLine = file_->getLine(*line_num);

    *char_num = 0;

    // empty line is a word
    if (fileLine.empty()) {
      found = true;
      break;
    }

    // skip space
    while (*char_num < fileLine.size() && isspace(fileLine[*char_num]))
      ++(*char_num);

    // if not at end of line we have found a word
    if (*char_num < fileLine.size()) {
      found = true;
      break;
    }
  }
}

void
CTextFileUtil::
nextWORD()
{
  uint x, y;

  file_->getPos(&x, &y);

  nextWORD(&y, &x);

  file_->moveTo(x, y);
}

// a WORD is a series of non-blank characters
void
CTextFileUtil::
nextWORD(uint *line_num, uint *char_num)
{
  const std::string &line = file_->getLine(*line_num);

  // skip current word
  bool found = false;

  if (*char_num < line.size()) {
    while (*char_num < line.size() && ! isspace(line[*char_num]))
      ++(*char_num);

    // skip space
    while (*char_num < line.size() && isspace(line[*char_num]))
      ++(*char_num);

    if (*char_num < line.size() && ! isspace(line[*char_num]))
      found = true;
  }

  // if no next word found then skip to next line
  while (! found) {
    if (*line_num >= file_->getNumLines() - 1)
      break;

    ++(*line_num);

    const std::string &fileLine = file_->getLine(*line_num);

    *char_num = 0;

    // empty line is a word
    if (fileLine.empty()) {
      found = true;
      break;
    }

    // skip space
    while (*char_num < fileLine.size() && isspace(fileLine[*char_num]))
      ++(*char_num);

    // if not at end of line we have found a word
    if (*char_num < fileLine.size()) {
      found = true;
      break;
    }
  }
}

void
CTextFileUtil::
prevWord()
{
  uint x, y;

  file_->getPos(&x, &y);

  prevWord(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
prevWord(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  // skip previous character
  if (*char_num > 0)
    --(*char_num);

  // skip spaces
  while (*char_num > 0 && isspace(line[*char_num]))
    --(*char_num);

  // if at start of line and more lines go back a line
  while (*char_num == 0 && *line_num > 0) {
    --(*line_num);

    line = file_->getLine(*line_num);

    // blank line is a word so we're done
    if (line.empty())
      return;

    *char_num = line.size() - 1;

    // skip spaces
    while (*char_num > 0 && isspace(line[*char_num]))
      --(*char_num);

    if (! isspace(line[*char_num]))
      break;
  }

  // skip to start of word
  if (*char_num > 0) {
    if      (isWordChar(line[*char_num])) {
      while (*char_num > 0 && isWordChar(line[*char_num - 1]))
        --(*char_num);
    }
    else if (! isspace(line[*char_num])) {
      while (*char_num > 0 && ! isWordChar(line[*char_num - 1]) &&
             ! isspace(line[*char_num - 1]))
        --(*char_num);
    }
  }
}

void
CTextFileUtil::
prevWORD()
{
  uint x, y;

  file_->getPos(&x, &y);

  prevWORD(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
prevWORD(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  // skip previous character
  if (*char_num > 0)
    --(*char_num);

  // skip spaces
  while (*char_num > 0 && isspace(line[*char_num]))
    --(*char_num);

  // if at start of line and more lines go back a line
  while (*char_num == 0 && *line_num > 0) {
    --(*line_num);

    line = file_->getLine(*line_num);

    // blank line is a word so we're done
    if (line.empty())
      return;

    *char_num = line.size() - 1;

    // skip spaces
    while (*char_num > 0 && isspace(line[*char_num]))
      --(*char_num);

    if (! isspace(line[*char_num]))
      break;
  }

  // skip to start of WORD
  if (*char_num > 0) {
    while (*char_num > 0 && ! isspace(line[*char_num - 1]))
      --(*char_num);
  }
}

void
CTextFileUtil::
endWord()
{
  uint x, y;

  file_->getPos(&x, &y);

  endWord(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
endWord(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  // if already at word end, increment to next char
  if      (*char_num < line.size() - 1) {
    if      (  isWordChar(line[*char_num]) && ! isWordChar(line[*char_num + 1])) {
      ++(*char_num);

      // start of new word so we're done
      if (! isspace(line[*char_num]))
        return;
    }
    else if (! isspace(line[*char_num]) &&
             (isspace(line[*char_num + 1]) || isWordChar(line[*char_num + 1]))) {
      ++(*char_num);
    }
  }
  else if (*char_num == line.size() - 1) {
    if (! isspace(line[*char_num])) {
      // new line
      if (*line_num < file_->getNumLines() - 1) {
        ++(*line_num);

        line = file_->getLine(*line_num);

        *char_num = 0;
      }
    }
  }
  else {
    // new line
    if (*line_num < file_->getNumLines() - 1) {
      ++(*line_num);

      line = file_->getLine(*line_num);

      *char_num = 0;
    }
  }

  // skip to next line with non-space
  if ((*char_num < line.size() && isspace(line[*char_num])) || line.empty()) {
    while (*char_num < line.size() && isspace(line[*char_num]))
      ++(*char_num);

    // if end of line goto next line and skip non-word again
    while (*char_num >= line.size()) {
      // new line
      if (*line_num < file_->getNumLines() - 1) {
        ++(*line_num);

        line = file_->getLine(*line_num);

        *char_num = 0;
      }

      // skip spaces
      while (*char_num < line.size() && isspace(line[*char_num]))
        ++(*char_num);
    }
  }

  // skip to end of word
  if (*char_num < line.size()) {
    if      (isWordChar(line[*char_num])) {
      while (*char_num < line.size() - 1 && isWordChar(line[*char_num + 1]))
        ++(*char_num);
    }
    else if (! isspace(line[*char_num])) {
      while (*char_num < line.size() - 1 &&
             ! isspace(line[*char_num + 1]) && ! isWordChar(line[*char_num + 1]))
        ++(*char_num);
    }
  }
}

void
CTextFileUtil::
endWORD()
{
  uint x, y;

  file_->getPos(&x, &y);

  endWORD(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
endWORD(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  // if already at word end, increment to next char
  if      (*char_num < line.size() - 1) {
    if (! isspace(line[*char_num]) && isspace(line[*char_num + 1]))
      ++(*char_num);
  }
  else if (*char_num == line.size() - 1) {
    if (! isspace(line[*char_num])) {
      // new line
      if (*line_num < file_->getNumLines() - 1) {
        ++(*line_num);

        line = file_->getLine(*line_num);

        *char_num = 0;
      }
    }
  }
  else {
    // new line
    if (*line_num < file_->getNumLines() - 1) {
      ++(*line_num);

      line = file_->getLine(*line_num);

      *char_num = 0;
    }
  }

  // skip to next line with non-space
  if ((*char_num < line.size() && isspace(line[*char_num])) || line.empty()) {
    while (*char_num < line.size() && isspace(line[*char_num]))
      ++(*char_num);

    // if end of line goto next line and skip non-word again
    while (*char_num >= line.size()) {
      // new line
      if (*line_num < file_->getNumLines() - 1) {
        ++(*line_num);

        line = file_->getLine(*line_num);

        *char_num = 0;
      }

      // skip spaces
      while (*char_num < line.size() && isspace(line[*char_num]))
        ++(*char_num);
    }
  }

  // skip to end of word
  if (*char_num < line.size()) {
    while (*char_num < line.size() - 1 && ! isspace(line[*char_num + 1]))
      ++(*char_num);
  }
}

bool
CTextFileUtil::
getWord(std::string &word)
{
  uint x, y;

  file_->getPos(&x, &y);

  return getWord(y, x, word);
}

bool
CTextFileUtil::
getWord(uint line_num, uint char_num, std::string &word)
{
  const std::string &line = file_->getLine(line_num);

  if (! isWordChar(line[char_num]))
    return false;

  int char_num1 = char_num;

  // find start of word
  while (char_num1 > 0 &&
         isWordChar(line[char_num1]) && isWordChar(line[char_num1 - 1]))
    --char_num1;

  int char_num2 = char_num1;

  // find end of word
  while (char_num2 < (int) line.size() - 1 &&
         isWordChar(line[char_num2]) && isWordChar(line[char_num2 + 1]))
    ++char_num2;

  for (int i = char_num1; i <= char_num2; ++i)
    word += line[i];

  return true;
}

void
CTextFileUtil::
nextSentence()
{
  uint x, y;

  file_->getPos(&x, &y);

  nextSentence(&y, &x);

  file_->moveTo(x, y);
}

// a sentence is the first non-blank after a blank line
// of the first non-blank after a '. ' '? ' '! '
void
CTextFileUtil::
nextSentence(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  uint num = 0;

  // empty line - go to next line
  if      (line.empty()) {
    if (! nextLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }
  // blank line - go to first non-blank
  else if (isBlank(line)) {
    ;
  }
  // sentence end - skip to sentence start
  else if (isSentenceEnd(line, *char_num, &num)) {
    *char_num += num;

    if (*char_num >= line.size()) {
      if (! nextLine(line_num, char_num))
        return;

      line = file_->getLine(*line_num);
    }
  }
  // in sentence - skip sentence and skip to next sentence start
  else {
    while (true) {
      while (*char_num < line.size() && ! isSentenceEnd(line, *char_num, &num))
        ++(*char_num);

      if (*char_num < line.size() && isSentenceEnd(line, *char_num, &num)) {
        *char_num += num;

        if (*char_num >= line.size()) {
          if (! nextLine(line_num, char_num))
            return;

          line = file_->getLine(*line_num);
        }

        break;
      }

      // next line
      if (*char_num >= line.size()) {
        if (! nextLine(line_num, char_num))
          return;

        line = file_->getLine(*line_num);

        if (isBlank(line))
          break;
      }
    }
  }

  while (! line.empty() && isBlank(line)) {
    if (! nextLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }

  while (*char_num < line.size() && isspace(line[*char_num]))
    ++(*char_num);
}

void
CTextFileUtil::
prevSentence()
{
  uint x, y;

  file_->getPos(&x, &y);

  prevSentence(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
prevSentence(uint *line_num, uint *char_num)
{
  uint num = 0;

  std::string line = file_->getLine(*line_num);

  bool sentence = false;

  // blank line is a sentence
  if (line.empty()) {
    // skip empty lines
    while (line.empty()) {
      // go to previous line
      if (! prevLine(line_num, char_num))
        return;

      line = file_->getLine(*line_num);
    }

    sentence = true;
  }

  // skip blank lines
  while (! line.empty() && isBlank(line)) {
    // go to previous line
    if (! prevLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }

  // empty line is a sentence so we are done
  if (line.empty())
    return;

  // go to last non-blank on non-blank line
  while (*char_num > 0 && isspace(line[*char_num]))
    --(*char_num);

  if (*char_num > 0 && isSentenceEnd(line, *char_num, &num)) {
    sentence = true;

    --(*char_num);
  }

  while (true) {
    // find previous sentence end or start of line
    while (*char_num > 0 && ! isSentenceEnd(line, *char_num, &num))
      --(*char_num);

    // if find previous sentence end then done
    if (*char_num > 0 && isSentenceEnd(line, *char_num, &num)) {
      if (sentence) {
        nextSentence(line_num, char_num);
        return;
      }

      sentence = true;

      --(*char_num);

      continue;
    }

    // go to previous line
    if (! prevLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);

    if (line.empty()) {
      if (sentence)
        nextSentence(line_num, char_num);

      return;
    }
  }
}

void
CTextFileUtil::
nextParagraph()
{
  uint x, y;

  file_->getPos(&x, &y);

  nextParagraph(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
nextParagraph(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  while (line.empty()) {
    if (! nextLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }

  while (! line.empty()) {
    if (! nextLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }
}

void
CTextFileUtil::
prevParagraph()
{
  uint x, y;

  file_->getPos(&x, &y);

  prevParagraph(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
prevParagraph(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  while (line.empty()) {
    if (! prevLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }

  while (! line.empty()) {
    if (! prevLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }
}

void
CTextFileUtil::
nextSection()
{
  uint x, y;

  file_->getPos(&x, &y);

  nextSection(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
nextSection(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  uint pos;

  if (! nextLine(line_num, char_num))
    return;

  line = file_->getLine(*line_num);

  while (! isSection(line, *char_num, &pos)) {
    if (! nextLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }
}

void
CTextFileUtil::
prevSection()
{
  uint x, y;

  file_->getPos(&x, &y);

  prevSection(&y, &x);

  file_->moveTo(x, y);
}

void
CTextFileUtil::
prevSection(uint *line_num, uint *char_num)
{
  std::string line = file_->getLine(*line_num);

  uint pos;

  if (! prevLine(line_num, char_num))
    return;

  line = file_->getLine(*line_num);

  while (! isSection(line, *char_num, &pos)) {
    if (! prevLine(line_num, char_num))
      return;

    line = file_->getLine(*line_num);
  }
}

bool
CTextFileUtil::
nextLine(uint *line_num, uint *char_num)
{
  const std::string &line = file_->getLine(*line_num);

  *char_num = line.size() - 1;

  if (*line_num >= file_->getNumLines() - 1)
    return false;

  ++(*line_num);

  *char_num = 0;

  return true;
}

bool
CTextFileUtil::
prevLine(uint *line_num, uint *char_num)
{
  *char_num = 0;

  if (*line_num <= 0)
    return false;

  --(*line_num);

  const std::string &line = file_->getLine(*line_num);

  *char_num = std::max((int) line.size() - 1, 0);

  return true;
}

void
CTextFileUtil::
addChars(uint line_num, uint char_num, const std::string &chars)
{
  file_->moveTo(char_num, line_num);

  uint len = chars.size();

  for (uint i = 0; i < len; ++i)
    file_->addCharAfter(chars[i]);
}

bool
CTextFileUtil::
isWordChar(char c) const
{
  return (isalnum(c) || c == '_');
}

//----------

bool
CTextFileUtil::
isBlank(const std::string &line) const
{
  uint len = line.size();

  for (uint i = 0; i < len; ++i) {
    char c = line[i];

    if (! isspace(c))
      return false;
  }

  return true;
}

bool
CTextFileUtil::
isSentenceEnd(const std::string &line, uint pos, uint *n) const
{
  *n = 0;

  uint len = line.size();

  if (pos + *n >= len)
    return false;

  char c = line[pos + *n];

  // check for . ! ?
  if (strchr(".?!", c) == NULL)
    return false;

  (*n)++;

  // skip (, ], " and ' after . ! ?
  while (pos + *n < len - 1) {
    char c1 = line[pos + *n];

    if (strchr(")]\"\'", c1) == NULL)
      break;

    (*n)++;
  }

  // EOL is ok
  if (pos + *n >= len)
    return true;

  c = line[pos + *n];

  // space is ok
  if (isspace(c))
    return true;

  return false;
}

bool
CTextFileUtil::
isSection(const std::string &line, uint, uint *n) const
{
  uint len = line.size();

  if (len > 0 && line[0] == '{') {
    *n = 0;
    return true;
  }

  return false;
}

bool
CTextFileUtil::
findNext(const std::string &pattern, uint line_num1, int char_num1,
         int line_num2, int char_num2, uint *fline_num, uint *fchar_num)
{
  const std::string &line1 = file_->getLine(line_num1);

  if (lineFindNext(line1, pattern, char_num1, -1, fchar_num)) {
    *fline_num = line_num1;
    return true;
  }

  for (int i = (int) line_num1 + 1; i <= line_num2 - 1; ++i) {
    const std::string &line = file_->getLine(i);

    if (lineFindNext(line, pattern, 0, -1, fchar_num)) {
      *fline_num = i;
      return true;
    }
  }

  const std::string &line2 = file_->getLine(line_num2);

  if (lineFindNext(line2, pattern, 0, char_num2, fchar_num)) {
    *fline_num = line_num2;
    return true;
  }

  return false;
}

bool
CTextFileUtil::
findNext(const CRegExp &pattern, uint line_num1, int char_num1,
         int line_num2, int char_num2, uint *fline_num, uint *fchar_num, uint *len)
{
  uint spos, epos;

  const std::string &line1 = file_->getLine(line_num1);

  if (lineFindNext(line1, pattern, char_num1, -1, &spos, &epos)) {
    *fline_num = line_num1;
    *fchar_num = spos;
    if (len) *len = epos - spos + 1;
    return true;
  }

  for (int i = (int) line_num1 + 1; i <= line_num2 - 1; ++i) {
    const std::string &line = file_->getLine(i);

    if (lineFindNext(line, pattern, 0, -1, &spos, &epos)) {
      *fline_num = i;
      *fchar_num = spos;
      if (len) *len = epos - spos + 1;
      return true;
    }
  }

  const std::string &line2 = file_->getLine(line_num2);

  if (lineFindNext(line2, pattern, 0, char_num2, &spos, &epos)) {
    *fline_num = line_num2;
    *fchar_num = spos;
    if (len) *len = epos - spos + 1;
    return true;
  }

  return false;
}

bool
CTextFileUtil::
lineFindNext(const std::string &line, const std::string &pattern,
            int char_num1, int char_num2, uint *char_num) const
{
  if (line.empty())
    return false;

  uint num_chars = line.size();

  if (char_num1 >= (int) num_chars)
    return false;

  if (char_num2 < 0)
    char_num2 = num_chars - 1;

  const char *cline = line.c_str();

  const char *pattern1 = pattern.c_str();

  char *p = CStrUtil::strstr(&cline[char_num1], &cline[char_num2], pattern1);

  if (p == NULL)
    return false;

  if (char_num)
    *char_num = p - cline;

  return true;
}

bool
CTextFileUtil::
lineFindNext(const std::string &line, const CRegExp &pattern,
             int char_num1, int char_num2, uint *spos, uint *epos) const
{
  if (line.empty())
    return false;

  uint num_chars = line.size();

  if (char_num1 >= (int) num_chars)
    return false;

  if (char_num2 < 0)
    char_num2 = num_chars - 1;

  std::string line1 = line.substr(char_num1, char_num2 - char_num1 + 1);

  if (! pattern.find(line1))
    return false;

  int spos1, epos1;

  if (! pattern.getMatchRange(&spos1, &epos1))
    return false;

  if (spos) *spos = spos1 + char_num1;
  if (epos) *epos = epos1 + char_num1;

  return true;
}

bool
CTextFileUtil::
findPrev(const std::string &pattern, uint line_num1, int char_num1,
         int line_num2, int char_num2, uint *fline_num, uint *fchar_num)
{
  const std::string &line1 = file_->getLine(line_num1);

  if (lineFindPrev(line1, pattern, char_num1, 0, fchar_num)) {
    *fline_num = line_num1;
    return true;
  }

  for (int i = line_num1 - 1; i >= line_num2 + 1; --i) {
    const std::string &line = file_->getLine(i);

    if (lineFindPrev(line, pattern, -1, 0, fchar_num)) {
      *fline_num = i;
      return true;
    }
  }

  const std::string &line2 = file_->getLine(line_num2);

  if (lineFindPrev(line2, pattern, -1, char_num2, fchar_num)) {
    *fline_num = line_num2;
    return true;
  }

  return false;
}

bool
CTextFileUtil::
findPrev(const CRegExp &pattern, uint line_num1, int char_num1,
         int line_num2, int char_num2, uint *fline_num, uint *fchar_num, uint *len)
{
  uint spos, epos;

  const std::string &line1 = file_->getLine(line_num1);

  if (lineFindPrev(line1, pattern, char_num1, 0, &spos, &epos)) {
    *fline_num = line_num1;
    *fchar_num = spos;
    if (len) *len = epos - spos + 1;
    return true;
  }

  for (int i = line_num1 - 1; i >= line_num2 + 1; --i) {
    const std::string &line = file_->getLine(i);

    if (lineFindPrev(line, pattern, -1, 0, &spos, &epos)) {
      *fline_num = i;
      *fchar_num = spos;
      if (len) *len = epos - spos + 1;
      return true;
    }
  }

  const std::string &line2 = file_->getLine(line_num2);

  if (lineFindPrev(line2, pattern, -1, char_num2, &spos, &epos)) {
    *fline_num = line_num2;
    *fchar_num = spos;
    if (len) *len = epos - spos + 1;
    return true;
  }

  return false;
}

bool
CTextFileUtil::
lineFindPrev(const std::string &line, const std::string &pattern,
             int char_num1, int char_num2, uint *char_num) const
{
  if (line.empty())
    return false;

  uint num_chars = line.size();

  if (char_num1 < 0)
    char_num1 = num_chars - 1;

  if (char_num2 >= (int) num_chars)
    return false;

  const char *cline = line.c_str();

  const char *pattern1 = pattern.c_str();

  char *p = CStrUtil::strrstr(&cline[char_num1], &cline[char_num2], pattern1);

  if (p == NULL)
    return false;

  if (char_num)
    *char_num = p - cline;

  return true;
}

bool
CTextFileUtil::
lineFindPrev(const std::string &line, const CRegExp &pattern,
             int char_num1, int char_num2, uint *spos, uint *epos) const
{
  if (line.empty())
    return false;

  uint num_chars = line.size();

  if (char_num1 < 0)
    char_num1 = num_chars - 1;

  if (char_num2 >= (int) num_chars)
    return false;

  std::string line1 = line.substr(char_num2, char_num1 - char_num2 + 1);

  if (! pattern.find(line1))
    return false;

  int spos1, epos1;

  if (! pattern.getMatchRange(&spos1, &epos1))
    return false;

  if (spos) *spos = spos1 + char_num2;
  if (epos) *epos = epos1 + char_num2;

  return true;
}

bool
CTextFileUtil::
findNextChar(uint line_num, int char_num, char c, bool multiline)
{
  ++char_num;

  while (true) {
    const std::string &line = file_->getLine(line_num);

    for (uint i = char_num; i < line.size(); ++i) {
      if (line[i] == c) {
        file_->moveTo(i, line_num);
        return true;
      }
    }

    if (! multiline)
      break;

    if (line_num == file_->getNumLines() - 1)
      break;

    ++line_num;

    char_num = 0;
  }

  return false;
}

bool
CTextFileUtil::
findNextChar(uint line_num, int char_num, const std::string &str, bool multiline)
{
  ++char_num;

  while (true) {
    const std::string &line = file_->getLine(line_num);

    const char *str1 = str.c_str();

    for (uint i = char_num; i < line.size(); ++i) {
      if (strchr(str1, line[i])) {
        file_->moveTo(i, line_num);
        return true;
      }
    }

    if (! multiline)
      break;

    if (line_num == file_->getNumLines() - 1)
      break;

    ++line_num;

    char_num = 0;
  }

  return false;
}

bool
CTextFileUtil::
findPrevChar(uint line_num, int char_num, char c, bool multiline)
{
  --char_num;

  std::string line = file_->getLine(line_num);

  while (true) {
    for (int i = char_num; i >= 0; --i) {
      if (line[i] == c) {
        file_->moveTo(i, line_num);
        return true;
      }
    }

    if (! multiline)
      break;

    if (line_num == 0)
      break;

    --line_num;

    line = file_->getLine(line_num);

    char_num = line.size() - 1;
  }

  return false;
}

bool
CTextFileUtil::
findPrevChar(uint line_num, int char_num, const std::string &str, bool multiline)
{
  --char_num;

  std::string line = file_->getLine(line_num);

  while (true) {
    const char *str1 = str.c_str();

    for (int i = char_num; i >= 0; --i) {
      if (strchr(str1, line[i])) {
        file_->moveTo(i, line_num);
        return true;
      }
    }

    if (! multiline)
      break;

    if (line_num == 0)
      break;

    --line_num;

    line = file_->getLine(line_num);

    char_num = line.size() - 1;
  }

  return false;
}

void
CTextFileUtil::
joinLine()
{
  uint line_num, char_num;

  file_->getPos(&char_num, &line_num);

  joinLine(line_num);
}

void
CTextFileUtil::
joinLine(uint line_num)
{
  uint line_num1, char_num;

  file_->getPos(&char_num, &line_num1);

  const std::string &line1 = file_->getLine(line_num);
  const std::string &line2 = file_->getLine(line_num + 1);

  file_->moveTo(0, line_num + 1);

  file_->deleteLineAt();

  file_->moveTo(char_num, line_num);

  file_->replaceLine(line1 + line2);

  file_->moveTo(line1.size(), line_num);
}

void
CTextFileUtil::
splitLine()
{
  uint line_num, char_num;

  file_->getPos(&char_num, &line_num);

  const std::string &line = file_->getLine(line_num);

  std::string str1 = line.substr(0, char_num);
  std::string str2 = line.substr(char_num);

  file_->replaceLine(str1);

  file_->addLineAfter(str2);

  file_->moveTo(char_num, line_num);
}

void
CTextFileUtil::
moveLine(uint line_num1, int line_num2)
{
  const std::string &line1 = file_->getLine(line_num1);

  file_->moveTo(0, line_num1);

  file_->deleteLineAt();

  file_->moveTo(0, line_num2);

  file_->addLineAfter(line1);
}

void
CTextFileUtil::
copyLine(uint line_num1, uint line_num2)
{
  const std::string &line1 = file_->getLine(line_num1);

  file_->moveTo(0, line_num2);

  file_->addLineAfter(line1);
}

void
CTextFileUtil::
replace(uint line_num, uint char_num1, uint char_num2, const std::string &replaceStr)
{
  const std::string &line = file_->getLine(line_num);

  std::string line1 = line.substr(0, char_num1);
  std::string line2 = line.substr(char_num2 + 1);

  file_->moveTo(char_num1, line_num);

  file_->replaceLine(line1 + replaceStr + line2);

  file_->moveTo(char_num1, line_num);
}

void
CTextFileUtil::
deleteTo(uint line_num1, uint char_num1, uint line_num2, uint char_num2)
{
  if      (line_num1 < line_num2) {
    const std::string &line = file_->getLine(line_num1);

    int num = line.size() - char_num1;

    deleteChars(line_num1, char_num1, num);

    for (uint i = 0; i < line_num2 - line_num1 - 1; ++i) {
      file_->moveTo(0, line_num1 + i + 1);

      file_->deleteLineAt();
    }

    file_->rmoveTo(0, 1);

    line_num2 = line_num1 + 1;

    deleteChars(line_num2, 0, char_num2);
  }
  else if (line_num2 < line_num1) {
    deleteChars(line_num1, 0, char_num1);

    for (uint i = 0; i < line_num1 - line_num2 - 1; ++i) {
      file_->moveTo(0, line_num2 + i + 1);

      file_->deleteLineAt();
    }

    file_->rmoveTo(0, -1);

    line_num2 = line_num1 - 1;

    const std::string &line = file_->getLine(line_num2);

    int num = line.size() - char_num2;

    deleteChars(line_num2, char_num2, num);
  }
  else {
    if (char_num1 < char_num2) {
      int num = char_num2 - char_num1 + 1;

      deleteChars(line_num1, char_num1, num);
    }
    else {
      int num = char_num1 - char_num2 + 1;

      deleteChars(line_num1, char_num2, num);
    }
  }
}

#if 0
void
CTextFileUtil::
deleteChars(uint line_num, uint char_num, int num)
{
  file_->moveTo(char_num, line_num);

  for (int i = 0; i < num; ++i)
    file_->deleteCharAt();
}
#endif

void
CTextFileUtil::
deleteChars(uint line_num, uint char_num, uint n)
{
  assert(line_num < file_->getNumLines());

  const std::string &line = file_->getLine(line_num);

  assert(char_num + n <= line.size());

  std::string str1 = line.substr(0, char_num);
  std::string str2 = line.substr(char_num + n);

  file_->moveTo(char_num, line_num);

  file_->replaceLine(str1 + str2);

  file_->moveTo(char_num, line_num);
}

void
CTextFileUtil::
moveToFirstNonBlankUp()
{
  file_->rmoveTo(0, -1);

  moveToFirstNonBlank();
}

void
CTextFileUtil::
moveToFirstNonBlankDown()
{
  file_->rmoveTo(0, 1);

  moveToFirstNonBlank();
}

void
CTextFileUtil::
moveToFirstNonBlank()
{
  uint x, y;

  file_->getPos(&x, &y);

  x = 0;

  const std::string &line = file_->getLine(y);

  uint len = line.size();

  while (x < len && isspace(line[x]))
    ++x;

  file_->moveTo(x, y);
}

void
CTextFileUtil::
moveToNonBlank()
{
  uint x, y;

  file_->getPos(&x, &y);

  const std::string &line = file_->getLine(y);

  uint len = line.size();

  while (x < len && isspace(line[x]))
    ++x;

  file_->moveTo(x, y);
}

void
CTextFileUtil::
swapChar()
{
  uint x, y;

  file_->getPos(&x, &y);

  swapChar(y, x);
}

void
CTextFileUtil::
swapChar(uint line_num, uint char_num)
{
  std::string line = file_->getLine(line_num);

  char c = line[char_num];

  if      (islower(c)) c = toupper(c);
  else if (isupper(c)) c = tolower(c);

  line[char_num] = c;

  file_->replaceLine(line);
}

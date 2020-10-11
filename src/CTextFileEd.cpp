#include <CTextFileEd.h>
#include <CTextFile.h>
#include <CTextFileUtil.h>
#include <CTextFileMarks.h>
#include <CTextFileUndo.h>
#include <COptVal.h>
#include <CFile.h>
#include <CRegExp.h>
#include <CStrUtil.h>
#include <CStrParse.h>
#include <CCommand.h>

CTextFileEd::
CTextFileEd(CTextFile *file) :
 file_          (file),
 util_          (NULL),
 marks_         (NULL),
 alt_marks_     (NULL),
 undo_          (NULL),
 alt_undo_      (NULL),
 mode_          (COMMAND),
 cur_line_      (0),
 cur_char_      (0),
 input_data_    (),
 findPattern_   (),
 ex_            (false),
 case_sensitive_(true),
 quit_          (false)
{
  util_  = new CTextFileUtil(file);
  marks_ = new CTextFileMarks(file);
  undo_  = new CTextFileUndo(file);

  notifyMgr_ = new CTextFileEdNotifierMgr(this);
}

CTextFileEd::
~CTextFileEd()
{
  delete util_;
  delete marks_;
  delete undo_;
}

void
CTextFileEd::
setUndo(CTextFileUndo *undo)
{
  delete undo_;

  undo_     = NULL;
  alt_undo_ = undo;
}

void
CTextFileEd::
setMarks(CTextFileMarks *marks)
{
  delete marks_;

  marks_     = NULL;
  alt_marks_ = marks;
}

void
CTextFileEd::
addNotifier(CTextFileEdNotifier *notifier)
{
  notifyMgr_->addNotifier(notifier);
}

void
CTextFileEd::
removeNotifier(CTextFileEdNotifier *notifier)
{
  notifyMgr_->removeNotifier(notifier);
}

void
CTextFileEd::
init()
{
  // move to first character on last line
  setPos(CIPoint2D(0, std::max(0, (int) file_->getNumLines() - 1)));
}

bool
CTextFileEd::
execFile(const std::string &fileName)
{
  std::vector<std::string> lines;

  CFile file(fileName);

  if (! file.exists() || ! file.isRegular())
    return false;

  file.toLines(lines);

  std::vector<std::string>::const_iterator p1, p2;

  for (p1 = lines.begin(), p2 = lines.end(); p1 != p2; ++p1) {
    std::string line = CStrUtil::stripSpaces(*p1);

    if (line.empty() || line[0] == '#')
      continue;

    execCmd(line);
  }

  return true;
}

bool
CTextFileEd::
execCmd(const std::string &cmd)
{
  if (mode_ == INPUT) {
    if (cmd == ".") {
      file_->startGroup();

      int start;

      if (input_data_.isCmd('a') || input_data_.isCmd('i'))
        start = input_data_.getStartLine();
      else
        start = input_data_.getStartLine() - 1;

      if (input_data_.isCmd('c')) {
        for (int i = input_data_.getStartLine(); i <= input_data_.getEndLine(); ++i)
          deleteLine(input_data_.getStartLine() - 1);
      }

      int i = start;

      StringList::const_iterator p1, p2;

      for (p1 = input_data_.getLines().begin(), p2 = input_data_.getLines().end(); p1 != p2; ++p1)
        addLine(i++, *p1);

      file_->endGroup();

      mode_ = COMMAND;

      input_data_.clearLines();

      setPos(CIPoint2D(0, i));
    }
    else
      input_data_.addLine(cmd);

    return true;
  }

  //----------

  CStrParse parse(cmd);

  num_lines_ = 0;

  bool has_range = false;
  int  line_num, char_num;
  bool all;

  while (parseLineNum(parse, line_num, char_num, all)) {
    has_range = true;

    if (all) {
      num_lines_ = 2;

      line_num1_ =                    1; char_num1_ = 0;
      line_num2_ = file_->getNumLines(); char_num2_ = 0;

      break;
    }

    ++num_lines_;

    line_num1_ = line_num2_;
    char_num1_ = char_num2_;

    line_num2_ = line_num;
    char_num2_ = char_num;

    if (! parse.isChar(',') && ! parse.isChar(';'))
      break;

    if (parse.isChar(';'))
      cur_line_ = line_num;

    parse.skipChar();
  }

  //-----

  if (has_range) {
    if (num_lines_ == 0) {
      line_num2_ = cur_line_;
      char_num2_ = 0;
    }

    if (num_lines_ <= 1) {
      line_num1_ = line_num2_;
      char_num1_ = char_num2_;
    }

    // ensure valid range
    if (line_num1_ < 1 || line_num1_ > (int) file_->getNumLines() + 1 ||
        line_num2_ < 1 || line_num2_ > (int) file_->getNumLines() + 1) {
      error("Invalid range: " + CStrUtil::toString(line_num1_) +
            " to " + CStrUtil::toString(line_num2_));
      return false;
    }
  }

  //-----

  parse.skipSpace();

  if (! parseCmd(parse))
    return false;

  //setPos(CIPoint2D(0, line_num2_ - 1));

  return true;
}

bool
CTextFileEd::
parseCmd(CStrParse &parse)
{
  // read command char
  char c = '\0';

  if (! parse.readChar(&c))
    c = '\0';

  switch (c) {
    case 'a':   // (.)a - add line and enter input mode
    case 'c':   // (.,.)c - change line and enter input mode
    case 'i': { // (.)i - insert line and enter input mode
      mode_ = INPUT;

      input_data_.setStartLine(line_num1_);
      input_data_.setEndLine  (line_num2_);

      input_data_.setCmd(c);

      break;
    }
    case 'd': { // (.,.)d - delete line
      doDelete(line_num1_, line_num2_);

      break;
    }
    case 'e': { // e [<file>|!<command>] - edit file
      if (! parse.isSpace() && ! parse.eof()) {
        std::string str;

        parse.readString(str);

        error("Not an editor command: e" + str);

        return false;
      }

      parse.skipSpace();

      std::string fileName;

      parse.readNonSpace(fileName);

      if (fileName.empty())
        fileName = file_->getFileName();

      file_->startGroup();

      if (! fileName.empty() && fileName[0] == '!') {
        CCommand cmd(fileName.substr(1));

        std::string dest;

        cmd.addStringDest(dest);

        cmd.start();

        std::vector<std::string> lines;

        file_->removeAllLines();

        CStrUtil::addLines(dest, lines);

        int numLines = lines.size();

        for (int l = 0; l < numLines; ++l)
          addLine(l, lines[l]);

        setPos(CIPoint2D(0, file_->getNumLines() - 1));
      }
      else {
        if (! fileName.empty())
          file_->read(fileName.c_str());
      }

      file_->endGroup();

      break;
    }
    case 'E': { // e [<file>|!<command>] - force edit file
      error("E: Unimplemented");
      break;
    }
    case 'f': { // f <file> - set default filename
      parse.skipSpace();

      std::string fileName;

      parse.readNonSpace(fileName);

      if (! fileName.empty())
        file_->setFileName(fileName);

      break;
    }
    case 'g': { // (.,.)g/<regexp>/<cmd>... - apply cmds to matching lines
      // read separator char
      char sep;

      if (! parse.readChar(&sep)) {
        error("Regular expression missing from global");
        return false;
      }

      // get regexp
      std::string find;

      while (! parse.eof() && ! parse.isChar(sep)) {
        char c1;

        parse.readChar(&c1);

        find += c1;
      }

      // use previous find if empty
      if (find.empty())
        find = getFindPattern();

      if (parse.isChar(sep))
        parse.skipChar();

      // get command
      std::string cmd = "p";

      while (! parse.eof()) {
        parse.readString(cmd);

        parse.skipSpace();
      }

      if (! parse.eof()) {
        error("Trailing characters");
        return false;
      }

      doGlob(line_num1_, line_num2_, find, cmd);

      break;
    }
    case 'G': { // (.,.)G/<regexp>/ - edit each matching line
      error("G: Unimplemented");
      break;
    }
    case 'H': { // H - toggle help
      error("H: Unimplemented");
      break;
    }
    case 'h': { // h - display help
      error("h: Unimplemented");
      break;
    }
    case 'j': { // (.,.)j - join lines
      doJoin(line_num1_, line_num2_);

      break;
    }
    case 'k': { // (.)k<c> - mark line with char
      char c1;

      if (! parse.readChar(&c1))
        return false;

      doMark(line_num1_, c1);

      break;
    }
    case 'l': { // (.,.)l - print lines unambiguously ($ at end)
      doPrint(line_num1_, line_num2_, /*numbered*/false, /*eol*/true);

      break;
    }
    case 'm': { // (.,.)m(.) - move lines
      COptValT<int> line_num3;

      parse.skipSpace();

      // read dest
      if      (parse.isDigit()) {
        int i;

        parse.readInteger(&i);

        line_num3 = i;
      }
      else if (parse.isChar('.')) {
        parse.skipChar();

        line_num3 = cur_line_ + 1;
      }
      else if (parse.isChar('$')) {
        parse.skipChar();

        line_num3 = file_->getNumLines();
      }

      parse.skipSpace();

      if (! line_num3.isValid())
        line_num3 = cur_line_ + 1;

      doMove(line_num1_, line_num2_, line_num3.getValue());

      break;
    }
    case 'n': { // (.,.)n - print numbered lines
      doPrint(line_num1_, line_num2_, /*numbered*/true, /*eol*/false);

      break;
    }
    case 'p': { // (.,.)p - print lines
      doPrint(line_num1_, line_num2_, /*numbered*/false, /*eol*/false);

      setPos(CIPoint2D(0, line_num2_ - 1));

      break;
    }
    case 'P': { // P - toggle prompt
      error("P: Unimplemented");
      break;
    }
    case 'q': { // q - quit
      edNotifyQuit(false);

      break;
    }
    case 'Q': { // Q - force quit
      edNotifyQuit(true);

      break;
    }
    case 'r': { // (.)r [<file>|!<command>] - read file after line
      parse.skipSpace();

      std::string fileName;

      parse.readNonSpace(fileName);

      if (fileName.empty())
        fileName = file_->getFileName();

      if (fileName.empty())
        return false;

      file_->startGroup();

      if (fileName[0] == '!') {
        CCommand cmd(fileName.substr(1));

        std::string dest;

        cmd.addStringDest(dest);

        cmd.start();

        std::vector<std::string> lines;

        CStrUtil::addLines(dest, lines);

        int numLines = lines.size();

        for (int l = 0; l < numLines; ++l)
          addLine(line_num1_ + l, lines[l]);

        // TODO: fix line pos after read
        setPos(CIPoint2D(0, file_->getNumLines() - 1));
      }
      else {
        CFile file(fileName);

        if (! file.exists() || ! file.isRegular())
          return false;

        std::vector<std::string> lines;

        file.toLines(lines);

        uint numLines = lines.size();

        for (uint l = 0; l < numLines; ++l)
          addLine(line_num1_ + l, lines[l]);

        file_->moveTo(line_num1_ + 1, 0);
      }

      file_->endGroup();

      break;
    }
    case 's': { // (.,.)s[/<regexp>/<replace>/[g|<n>]] - substitute
      // read separator char
      char sep;

      if (! parse.readChar(&sep))
        return false;

      // get regexp
      std::string find, replace;

      while (! parse.eof() && ! parse.isChar(sep)) {
        char c1;

        parse.readChar(&c1);

        find += c1;
      }

      // use previous find if empty
      if (find.empty())
        find = getFindPattern();

      if (parse.isChar(sep))
        parse.skipChar();

      // get replace string
      while (! parse.eof() && ! parse.isChar(sep)) {
        char c1;

        parse.readChar(&c1);

        replace += c1;
      }

      if (parse.isChar(sep))
        parse.skipChar();

      // get optional modifier char
      // TODO: support <n>
      char mod = '\0';

      if (! parse.eof()) {
        parse.readChar(&mod);

        if (mod != 'g')
          return false;
      }

      parse.skipSpace();

      if (! parse.eof())
        return false;

      doSubstitute(line_num1_, line_num2_, find, replace, mod);

      break;
    }
    case 't': { // (.,.)t(.) - copy lines
      COptValT<int> line_num3;

      parse.skipSpace();

      // read dest
      if      (parse.isDigit()) {
        int i;

        parse.readInteger(&i);

        line_num3 = i;
      }
      else if (parse.isChar('.')) {
        parse.skipChar();

        line_num3 = cur_line_ + 1;
      }
      else if (parse.isChar('$')) {
        parse.skipChar();

        line_num3 = file_->getNumLines();
      }

      parse.skipSpace();

      if (! line_num3.isValid())
        line_num3 = cur_line_ + 1;

      doCopy(line_num1_, line_num2_, line_num3.getValue());

      break;
    }
    case 'u': { // u - undo
      doUndo();

      break;
    }
    case 'v': { // (.,.)v/<regexp>/<cmd>... - apply cmds to non-matching lines
      error("v: Unimplemented");
      break;
    }
    case 'V': { // (.,.)V/<regexp>/ - edit each non-matching line
      error("V: Unimplemented");
      break;
    }
    case 'w': { // (.,.)w[q] [<file>|!<command>] - write to file
      bool quit = false;

      if (parse.isChar('q')) {
        parse.skipChar();

        quit = true;
      }

      parse.skipSpace();

      std::string fileName;

      parse.readNonSpace(fileName);

      if (fileName.empty())
        fileName = file_->getFileName();

      if (! fileName.empty() && fileName[0] == '!') {
        error("w!: Unimplemented");
        break;
      }
      else {
        if (! fileName.empty())
          file_->write(fileName.c_str());
      }

      if (quit)
        edNotifyQuit(false);

      break;
    }
    case 'W': { // (.,.)W[q] [<file>|!<command>] - append to file
      error("W: Unimplemented");
      break;
    }
    case 'x': { // (.)x - paste cut buffer
      doPaste(line_num1_);

      break;
    }
    case 'y': { // (.,.)y - copy to buffer
      doCopy(line_num1_, line_num2_);

      break;
    }
    case 'z': { // (.)z[<n>] - scroll to line
      error("z: Unimplemented");
      break;
    }
    case '!': { // ![!]<command> - execute command
      std::string str = parse.getAt();

      doExecute(line_num1_, line_num2_, str);

      break;
    }
    case '#': { // (.,.)# - comment
      break;
    }
    case '=': { // (.)= - print line number
      output(CStrUtil::toString(line_num1_));

      break;
    }
    case '/': { // / - find next
      // get regexp
      std::string find;

      while (! parse.eof() && ! parse.isChar('/')) {
        char c1;

        parse.readChar(&c1);

        find += c1;
      }

      doFindNext(line_num1_, line_num2_, find);

      break;
    }
    case '?': { // / - find prev
      // get regexp
      std::string find;

      while (! parse.eof() && ! parse.isChar('/')) {
        char c1;

        parse.readChar(&c1);

        find += c1;
      }

      doFindPrev(line_num1_, line_num2_, find);

      break;
    }
    case '@': { // @ - output text
      std::string str = parse.getAt();

      output(str);

      break;
    }
    case '\0': { // move to line
      if (! getEx())
        doPrint(line_num1_, line_num2_, /*numbered*/false, /*eol*/false);

      if (num_lines_ == 0)
        setPos(CIPoint2D(char_num2_, line_num2_));
      else
        setPos(CIPoint2D(char_num2_, line_num2_ - 1));

      break;
    }
    default:
      return false;
  }

  return true;
}

bool
CTextFileEd::
parseLineNum(CStrParse &parse, int &line_num, int &char_num, bool &all)
{
  line_num = cur_line_;
  char_num = 0;
  all      = false;

  parse.skipSpace();

  // + offset
  if      (parse.isChar('+')) {
    parse.skipChar();

    parse.skipSpace();

    int d = 0;

    if (parse.isDigit())
      parse.readInteger(&d);
    else {
      d = 1;

      while (parse.isChar('+')) {
        ++d;

        parse.skipChar();

        parse.skipSpace();
      }
    }

    line_num = cur_line_ + d;

    return true;
  }
  // - offset
  else if (parse.isChar('-') || parse.isChar('^')) {
    parse.skipChar();

    parse.skipSpace();

    int d = 0;

    if (parse.isDigit())
      parse.readInteger(&d);
    else {
      d = 1;

      while (parse.isChar('-')) {
        ++d;

        parse.skipChar();

        parse.skipSpace();
      }
    }

    line_num = cur_line_ - d;

    return true;
  }
  // absolute value
  else if (parse.isDigit()) {
    int i;

    parse.readInteger(&i);

    parse.skipSpace();

    line_num = i;

    while (parse.isChar('+') || parse.isChar('-')) {
      int s = (parse.isChar('+') ? 1 : -1);

      parse.skipChar();

      parse.skipSpace();

      int d = 0;

      if (parse.isDigit()) {
        parse.readInteger(&d);

        parse.skipSpace();
      }

      line_num += d*s;
    }

    return true;
  }
  // current
  else if (parse.isChar('.')) {
    parse.skipChar();

    parse.skipSpace();

    line_num = cur_line_;

    // offset
    while (parse.isChar('+') || parse.isChar('-')) {
      int s = (parse.isChar('+') ? 1 : -1);

      parse.skipChar();

      parse.skipSpace();

      int d = 0;

      if (parse.isDigit()) {
        parse.readInteger(&d);

        parse.skipSpace();
      }

      line_num += d*s;
    }

    return true;
  }
  // last line
  else if (parse.isChar('$')) {
    parse.skipChar();

    parse.skipSpace();

    line_num = file_->getNumLines();

    // offset
    while (parse.isChar('+') || parse.isChar('-')) {
      int s = (parse.isChar('+') ? 1 : -1);

      parse.skipChar();

      parse.skipSpace();

      int d = 0;

      if (parse.isDigit()) {
        parse.readInteger(&d);

        parse.skipSpace();
      }

      line_num += d*s;
    }

    return true;
  }
  // mark
  else if (parse.isChar('\'')) {
    parse.skipChar();

    char c;

    if (! parse.readChar(&c))
      return true;

    uint line_num1, char_num1;

    if (! getMarks()->getMarkPos(std::string(&c, 1), &line_num1, &char_num1)) {
      error("Mark not set");
      return false;
    }

    line_num = line_num1 + 1;
    char_num = char_num1 + 1;

    return true;
  }
  // all lines
  else if (parse.isChar('%')) {
    parse.skipChar();

    line_num = 1;

    all = true;

    return true;
  }
  // search forward or backward
  else if (parse.isChar('/') || parse.isChar('?')) {
    char sc = parse.getCharAt();

    std::string str;

    parse.skipChar();

    while (! parse.eof() && ! parse.isChar(sc)) {
      if (parse.isChar('\\'))
        parse.skipChar();

      char c;

      if (parse.readChar(&c))
        str += c;
    }

    if (parse.eof() && ! getEx()) {
      error("Missing terminating '" + std::string(&sc, 1) + "'");
      return false;
    }

    parse.skipChar();

    int fline_num, fchar_num;

    if (sc == '/') {
      if (findNext(str, &fline_num, &fchar_num)) {
        line_num = fline_num;
        char_num = fchar_num;
      }
    }
    else {
      if (findPrev(str, &fline_num, &fchar_num)) {
        line_num = fline_num;
        char_num = fchar_num;
      }
    }

    return true;
  }
  else
    return false;
}

bool
CTextFileEd::
findNext(const std::string &str, int *line_num, int *char_num)
{
  CRegExp regexp(str);

  regexp.setCaseSensitive(case_sensitive_);

  uint fline_num, fchar_num;

  uint num_lines = file_->getNumLines();

  if (! getEx() && cur_line_ >= int(num_lines)) {
    if (util_->findNext(regexp, 0, 0, num_lines - 1, -1, &fline_num, &fchar_num, NULL)) {
      *line_num = fline_num + 1;
      *char_num = fchar_num;
      return true;
    }

    return false;
  }

  int row1, col1, row2, col2;

  if (getEx()) {
    row1 = cur_line_;
    col1 = cur_char_ + 1;
    row2 = cur_line_;
    col2 = cur_char_ - 1;
  }
  else {
    row1 = cur_line_ + 1;
    col1 = 0;
    row2 = cur_line_;
    col2 = -1;
  }

  if (util_->findNext(regexp, row1, col1, num_lines - 1, -1, &fline_num, &fchar_num, NULL) ||
      util_->findNext(regexp, 0, 0, row2, col2, &fline_num, &fchar_num, NULL)) {
    *line_num = fline_num + 1;
    *char_num = fchar_num;
    return true;
  }

  return false;
}

bool
CTextFileEd::
findPrev(const std::string &str, int *line_num, int *char_num)
{
  CRegExp regexp(str);

  regexp.setCaseSensitive(case_sensitive_);

  uint fline_num, fchar_num;

  uint num_lines = file_->getNumLines();

  if (! getEx() && cur_line_ == 0) {
    if (util_->findPrev(regexp, num_lines - 1, -1, 0, 0, &fline_num, &fchar_num, NULL)) {
      *line_num = fline_num + 1;
      *char_num = fchar_num;
      return true;
    }

    return false;
  }

  int row1, col1, row2, col2;

  if (getEx()) {
    row1 = cur_line_;
    col1 = cur_char_ - 1;
    row2 = cur_line_;
    col2 = cur_char_ + 1;
  }
  else {
    row1 = cur_line_ - 1;
    col1 = -1;
    row1 = cur_line_;
    col1 = 0;
  }

  if (util_->findPrev(regexp, row1, col1, 0, 0, &fline_num, &fchar_num, NULL) ||
      util_->findPrev(regexp, num_lines - 1, -1, row2, col2, &fline_num, &fchar_num, NULL)) {
    *line_num = fline_num + 1;
    *char_num = fchar_num;
    return true;
  }

  return false;
}

void
CTextFileEd::
doSubstitute(int line_num1, int line_num2, const std::string &find,
             const std::string &replace, char mod)
{
  bool global = (mod == 'g');

  file_->startGroup();

  CRegExp regexp(find);

  regexp.setCaseSensitive(case_sensitive_);

  for (int i = line_num1; i <= line_num2; ++i) {
    uint fline_num, fchar_num, len;

    if (util_->findNext(regexp, i - 1, 0, i - 1, -1, &fline_num, &fchar_num, &len)) {
      int spos = fchar_num;
      int epos = spos + len - 1;

      util_->replace(i - 1, spos, epos, replace);

      if (global) {
        while (util_->findNext(regexp, i - 1, epos + 1, i - 1, -1, &fline_num, &fchar_num, &len)) {
          int spos1 = fchar_num;
          int epos1 = spos1 + len - 1;

          util_->replace(i - 1, spos1, epos1, replace);
        }
      }
    }
  }

  file_->endGroup();
}

void
CTextFileEd::
doFindNext(int line_num1, int line_num2, const std::string &find)
{
  CRegExp regexp(find);

  regexp.setCaseSensitive(case_sensitive_);

  uint fline_num, fchar_num;

  util_->findNext(regexp, line_num1 - 1, 0, line_num2 - 1, -1, &fline_num, &fchar_num, NULL);
}

void
CTextFileEd::
doFindPrev(int line_num1, int line_num2, const std::string &find)
{
  CRegExp regexp(find);

  regexp.setCaseSensitive(case_sensitive_);

  uint fline_num, fchar_num;

  util_->findPrev(regexp, line_num1 - 1, -1, line_num2 - 1, 0, &fline_num, &fchar_num, NULL);
}

void
CTextFileEd::
doGlob(int line_num1, int line_num2, const std::string &find, const std::string &cmd)
{
  file_->startGroup();

  CRegExp regexp(find);

  regexp.setCaseSensitive(case_sensitive_);

  for (int i = line_num1; i <= line_num2; ++i) {
    const std::string &line = file_->getLine(i - 1);

    if (! util_->lineFindNext(line, regexp, 0, -1, NULL, NULL))
      continue;

    if      (cmd == "d")
      deleteLine(--i);
    else if (cmd == "p")
      output(line);
    else {
      error("Not an editor command: " + cmd);
      break;
    }
  }

  file_->endGroup();
}

void
CTextFileEd::
doJoin(int line_num1, int line_num2)
{
  file_->startGroup();

  for (int i = line_num1; i < line_num2; ++i)
    util_->joinLine(line_num1 - 1);

  file_->endGroup();
}

void
CTextFileEd::
doMove(int line_num1, int line_num2, int line_num3)
{
  file_->startGroup();

  if      (line_num3 < line_num1) {
    for (int i = line_num2; i >= line_num1; --i)
      util_->moveLine(i - 1, line_num3 - 1);
  }
  else if (line_num3 > line_num2) {
    for (int i = line_num1; i <= line_num2; ++i)
      util_->moveLine(line_num1 - 1, line_num3 - 1);
  }

  file_->endGroup();
}

void
CTextFileEd::
doCopy(int line_num1, int line_num2, int line_num3)
{
  file_->startGroup();

  if      (line_num3 < line_num1) {
    for (int i = line_num2; i >= line_num1; --i)
      util_->copyLine(line_num2 - 1, line_num3 - 1);
  }
  else if (line_num3 > line_num2) {
    for (int i = line_num2; i >= line_num1; --i)
      util_->copyLine(i - 1, line_num3 - 1);
  }

  file_->endGroup();
}

void
CTextFileEd::
doDelete(int line_num1, int line_num2)
{
  file_->startGroup();

  for (int i = line_num1; i <= line_num2; ++i)
    deleteLine(line_num1 - 1);

  file_->endGroup();

  setPos(CIPoint2D(0, line_num1 - 1));
}

void
CTextFileEd::
doCopy(int /*line_num1*/, int /*line_num2*/)
{
}

void
CTextFileEd::
doPaste(int /*line_num1*/)
{
}

void
CTextFileEd::
doUndo()
{
  getUndo()->undo();
}

void
CTextFileEd::
doMark(int i, char c)
{
  getMarks()->setMarkPos(std::string(&c, 1), i - 1, 0);
}

void
CTextFileEd::
doExecute(int line_num1, int line_num2, const std::string &cmdStr)
{
  file_->startGroup();

  std::string src;

  for (int i = line_num1; i <= line_num2; ++i) {
    if (i > line_num1) src += "\n";

    src += file_->getLine(line_num1 - 1);

    deleteLine(line_num1 - 1);
  }

  CCommand cmd(cmdStr);

  std::string dest;

  cmd.addStringSrc (src);
  cmd.addStringDest(dest);

  cmd.start();

  cmd.wait();

  std::vector<std::string> lines;

  CStrUtil::addLines(dest, lines);

  int numLines = lines.size();

  for (int l = 0; l < numLines; ++l)
    addLine(line_num1 + l - 1, lines[l]);

  setPos(CIPoint2D(0, line_num1 - 1));

  file_->endGroup();
}

void
CTextFileEd::
doPrint(int line_num1, int line_num2, bool numbered, bool eol)
{
  for (int i = line_num1; i <= line_num2; ++i) {
    const std::string &line = file_->getLine(i - 1);

    std::string str;

    if (numbered) {
      str += CStrUtil::toString(i);
      str += "\t";
    }

    str += line;

    if (eol)
      str += "$";

    output(str);
  }
}

void
CTextFileEd::
output(const std::string &msg)
{
  std::cout << msg << std::endl;
}

void
CTextFileEd::
error(const std::string &msg)
{
  std::cerr << msg << std::endl;
}

void
CTextFileEd::
addLine(uint row, const std::string &line)
{
  uint x, y;

  file_->getPos(&x, &y);

  file_->moveTo(x, row);

  file_->addLineAfter(line);
}

void
CTextFileEd::
deleteLine(uint row)
{
  file_->moveTo(0, row);

  file_->deleteLineAt();
}

void
CTextFileEd::
setPos(const CIPoint2D &p)
{
  int x = p.x;
  int y = p.y;

  int num = file_->getNumLines();

  if (num > 0) {
    while (y <  0  ) y += num;
    while (y >= num) y -= num;
  }
  else
    y = 0;

  file_->moveTo(x, y);

  cur_line_ = y + 1;
  cur_char_ = x;
}

uint
CTextFileEd::
getRow() const
{
  uint x, y;

  file_->getPos(&x, &y);

  return y;
}

uint
CTextFileEd::
getCol() const
{
  uint x, y;

  file_->getPos(&x, &y);

  return x;
}

void
CTextFileEd::
edNotifyQuit(bool force)
{
  quit_ = true;

  notifyMgr_->edNotifyQuit(force);
}

//------

CTextFileEdNotifierMgr::
CTextFileEdNotifierMgr(CTextFileEd *ed) :
 ed_(ed)
{
}

void
CTextFileEdNotifierMgr::
addNotifier(CTextFileEdNotifier *notifier)
{
  notifierList_.push_back(notifier);
}

void
CTextFileEdNotifierMgr::
removeNotifier(CTextFileEdNotifier *notifier)
{
  notifierList_.remove(notifier);
}

void
CTextFileEdNotifierMgr::
edNotifyQuit(bool force)
{
  NotifierList::const_iterator p1, p2;

  for (p1 = notifierList_.begin(), p2 = notifierList_.end(); p1 != p2; ++p1)
    (*p1)->edNotifyQuit(force);
}

//------

CTextFileEdNotifier::
CTextFileEdNotifier()
{
}

void
CTextFileEdNotifier::
edNotifyQuit(bool)
{
}

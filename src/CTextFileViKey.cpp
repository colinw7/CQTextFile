#include <CTextFileViKey.h>
#include <CTextFile.h>
#include <CTextFileMarks.h>
#include <CTextFileBuffer.h>
#include <CTextFileSel.h>
#include <CTextFileUtil.h>
#include <CTextFileUndo.h>
#include <CStrUtil.h>
#include <cstring>

CTextFileViKey::
CTextFileViKey(CTextFile *file) :
 CTextFileKey(file),
 lastKey_    (CKEY_TYPE_NUL),
 count_      (0),
 insertMode_ (false),
 cmdLineMode_(false),
 register_   ('\0'),
 lastCommand_(file),
 findChar_   ('\0'),
 findForward_(true),
 findTill_   (false),
 visual_     (false)
{
  marks_  = new CTextFileMarks(file);
  buffer_ = new CTextFileBuffer(file);
  ed_     = new CTextFileEd(file);

  ed_->setEx(true);
  ed_->setUndo (undo_);
  ed_->setMarks(marks_);

  ed_->addNotifier(this);

  file_->addNotifier(this);
}

CTextFileViKey::
~CTextFileViKey()
{
  delete marks_;
  delete buffer_;
  delete ed_;
}

void
CTextFileViKey::
fileOpened(const std::string &)
{
  std::string msg =
    CStrUtil::strprintf("\"%s\" %d lines", file_->getFileName().c_str(),
                        file_->getNumLines());

  showStatusMsg(msg);
}

void
CTextFileViKey::
processChar(CKeyType key, const std::string &text, CEventModifier modifier)
{
  if (getInsertMode())
    processInsertChar(key, text, modifier);
  else
    processCommandChar(key, text, modifier);
}

void
CTextFileViKey::
execCmd(const std::string &cmd)
{
  std::vector<std::string> words;

  CStrUtil::toWords(cmd, words);

  uint num_words = uint(words.size());

  std::string cmdName = (num_words > 0 ? words[0] : "");

  if      (cmdName == "set") {
    std::string name, arg;

    if (num_words > 1) {
      for (uint i = 1; i < num_words; ++i) {
        std::string::size_type pos = words[i].find('=');

        if (pos != std::string::npos) {
          name = words[i].substr(0, pos);
          arg  = words[i].substr(pos + 1);
        }
        else {
          name = words[i];
          arg  = "";
        }

        setOptionString(name, arg);
      }
    }
    else {
      std::string status;

      status += std::string("ignorecase ") + CStrUtil::toString(options_.ignorecase) + "\n";
      status += std::string("list       ") + CStrUtil::toString(options_.list      ) + "\n";
      status += std::string("number     ") + CStrUtil::toString(options_.number    ) + "\n";
      status += std::string("showmatch  ") + CStrUtil::toString(options_.showmatch ) + "\n";
      status += std::string("shiftwidth ") + CStrUtil::toString(options_.shiftwidth) + "\n";

      showOverlayMsg(status);
    }
  }
  else {
    ed_->setPos(getPos());

    ed_->execCmd(cmd);
  }
}

void
CTextFileViKey::
processCommandChar(CKeyType key, const std::string &text, CEventModifier modifier)
{
  switch (key) {
    case CKEY_TYPE_Shift_L  : case CKEY_TYPE_Shift_R:
    case CKEY_TYPE_Control_L: case CKEY_TYPE_Control_R:
    case CKEY_TYPE_Caps_Lock: case CKEY_TYPE_Shift_Lock:
    case CKEY_TYPE_Meta_L   : case CKEY_TYPE_Meta_R:
    case CKEY_TYPE_Alt_L    : case CKEY_TYPE_Alt_R:
    case CKEY_TYPE_Super_L  : case CKEY_TYPE_Super_R:
    case CKEY_TYPE_Hyper_L  : case CKEY_TYPE_Hyper_R:
      return;
    default:
      break;
  }

  //------

  if (lastKey_ != CKEY_TYPE_NUL) {
    char c = CEvent::keyTypeChar(key);

    switch (lastKey_) {
      case CKEY_TYPE_QuoteLeft: { // goto mark line and char
        marks_->markReturn();

        uint line_num, char_num;

        if (marks_->getMarkPos(std::string(&c, 1), &line_num, &char_num))
          moveTo(char_num, line_num);

        goto done;
      }
      case CKEY_TYPE_QuoteDbl: { // set register
        if      (key == CKEY_TYPE_TAB ||
                 key == CKEY_TYPE_Tab ||
                 key == CKEY_TYPE_KP_Tab) {
          //file_->displayRegisters();
        }
        else if (isalnum(c) || strchr("#/", c))
          register_ = c;
        else
          error("Invalid register name '" + std::string(&c, 1) + "'");

        goto done;
      }
      case CKEY_TYPE_Apostrophe: { // goto mark line
        if (key == CKEY_TYPE_TAB ||
            key == CKEY_TYPE_Tab ||
            key == CKEY_TYPE_KP_Tab)
          marks_->displayMarks();
        else {
          marks_->markReturn();

          uint line_num, char_num;

          if (marks_->getMarkPos(std::string(&c, 1), &line_num, &char_num))
            moveTo(0, line_num);
        }

        goto done;
      }
      case CKEY_TYPE_c: { // change
        if      (key == CKEY_TYPE_c) {
          setInsertMode(true);

          CIPoint2D pos = getPos();

          moveTo(0, pos.y);

          util_->deleteEOL();
        }
        else if (key == CKEY_TYPE_w) {
          setInsertMode(true);

          util_->deleteWord();
        }
        else if (key == CKEY_TYPE_l) {
          setInsertMode(true);

          for (uint i = 0; i < std::max(count_, 1U); ++i)
            file_->deleteCharAt();
        }
        else {
          CIPoint2D start = getPos();
          CIPoint2D end   = start;

          bool rc = processMoveChar(key, text, modifier, end);

          if (rc) {
            setInsertMode(true);

            util_->deleteTo(start.y, start.x, end.y, end.x);
          }
        }

        lastCommand_.clear();
        lastCommand_.addCount(count_);
        lastCommand_.addKey(lastKey_);
        lastCommand_.addKey(key);

        break;
      }
      case CKEY_TYPE_d: { // delete
        uint count = std::max(count_, 1U);

        if      (key == CKEY_TYPE_d) {
          buffer_->yankLines(register_, count);

          file_->startGroup();

          for (uint i = 0; i < count; ++i)
            file_->deleteLineAt();

          file_->endGroup();
        }
        else if (key == CKEY_TYPE_w) {
          buffer_->yankWords(register_, count);

          file_->startGroup();

          for (uint i = 0; i < count; ++i)
            util_->deleteWord();

          file_->endGroup();
        }
        else if (key == CKEY_TYPE_l) {
          buffer_->yankChars(register_, count);

          file_->startGroup();

          for (uint i = 0; i < count; ++i)
            file_->deleteCharAt();

          file_->endGroup();
        }
        else {
          CIPoint2D start = getPos();
          CIPoint2D end   = start;

          bool rc = processMoveChar(key, text, modifier, end);

          if (rc) {
            buffer_->yankTo(register_, start.y, start.x, end.y, end.x, false);

            util_->deleteTo(start.y, start.x, end.y, end.x);
          }
        }

        lastCommand_.clear();
        lastCommand_.addKey(lastKey_);
        lastCommand_.addKey(key);

        break;
      }
      case CKEY_TYPE_f: { // find next char
        doFindChar(c, std::max(count_, 1U), true, false);

        break;
      }
      case CKEY_TYPE_F: { // find prev char
        doFindChar(c, std::max(count_, 1U), false, false);

        break;
      }
      case CKEY_TYPE_g: { // test code !!!
        sel_->setSelectRange(getPos(), getPos());

        CIPoint2D select_end = sel_->getSelectEnd();

        bool rc = processMoveChar(key, text, modifier, select_end);

        if (rc)
          sel_->rangeSelect(sel_->getSelectStart(), select_end, true);
        else
          sel_->clearSelection();

        break;
      }
      case CKEY_TYPE_m: { // mark
        marks_->setMarkPos(std::string(&c, 1));

        goto done;
      }
      case CKEY_TYPE_r: { // replace char
        file_->replaceChar(c);

        //file_->cursorLeft(1);

        lastCommand_.clear();
        lastCommand_.addKey(lastKey_);
        lastCommand_.addKey(key);

        break;
      }
      case CKEY_TYPE_t: { // till next char
        doFindChar(c, std::max(count_, 1U), true, true);

        break;
      }
      case CKEY_TYPE_T: { // till prev char
        doFindChar(c, std::max(count_, 1U), false, true);

        break;
      }
      case CKEY_TYPE_y: { // yank
        if      (key == CKEY_TYPE_y)
          buffer_->yankLines(register_, std::max(count_, 1U));
        else if (key == CKEY_TYPE_w)
          buffer_->yankWords(register_, std::max(count_, 1U));
        else {
          CIPoint2D start = getPos();
          CIPoint2D end   = start;

          bool rc = processMoveChar(key, text, modifier, end);

          if (rc)
            buffer_->yankTo(register_, start.y, start.x, end.y, end.x, false);
        }

        break;
      }
      case CKEY_TYPE_z: { // scroll to
        if      (key == CKEY_TYPE_Return ||
                 key == CKEY_TYPE_Plus)
          notifyScrollTop();
        else if (key == CKEY_TYPE_Period ||
                 key == CKEY_TYPE_z)
          notifyScrollMiddle();
        else if (key == CKEY_TYPE_b ||
                 key == CKEY_TYPE_Minus)
          notifyScrollBottom();
        else if (key == CKEY_TYPE_t)
          notifyScrollTop();

        break;
      }
      case CKEY_TYPE_Z: {
        if      (key == CKEY_TYPE_Q)
          notifyQuit();
        else if (key == CKEY_TYPE_Z) {
          file_->write(file_->getFileName().c_str());

          notifyQuit();
        }

        break;
      }
      case CKEY_TYPE_Exclam: {
        std::string str;

        if      (key == CKEY_TYPE_Exclam)
          str = ":.!";
        else {
          CIPoint2D pos = getPos();

          sel_->setSelectRange(pos, pos);

          bool rc = processMoveChar(key, text, modifier, pos);

          if (rc) {
            int d = std::abs(sel_->getSelectStart().y - pos.y);

            if (d != 0) {
              moveTo(pos.x, sel_->getSelectEnd().y);

              str = ":.,+" + CStrUtil::toString(d) + "!";
            }
            else
              str = ":.!";
          }
        }

        enterCmdLineMode(str);

        break;
      }
      case CKEY_TYPE_Less: {
        CIPoint2D start = getPos();
        CIPoint2D end   = start;

        if      (key == CKEY_TYPE_Less)
          util_->shiftLeft(start.y, end.y);
        else {
          bool rc = processMoveChar(key, text, modifier, end);

          if (rc)
            util_->shiftRight(start.y, end.y);
        }

        break;
      }
      case CKEY_TYPE_Greater: {
        CIPoint2D start = getPos();
        CIPoint2D end   = start;

        if      (key == CKEY_TYPE_Greater)
          util_->shiftRight(start.y, end.y);
        else {
          bool rc = processMoveChar(key, text, modifier, end);

          if (rc)
            util_->shiftRight(start.y, end.y);
        }

        break;
      }
      case CKEY_TYPE_BracketLeft: {
        if (key == CKEY_TYPE_BracketLeft)
          util_->prevSection();

        break;
      }
      case CKEY_TYPE_BracketRight: {
        if (key == CKEY_TYPE_BracketRight)
          util_->nextSection();

        break;
      }
      default:
        break;
    }

    goto done;
  }

  //------

  if      (key == CKEY_TYPE_0) {
    if (count_ == 0) {
      CIPoint2D pos = getPos();

      moveTo(0, pos.y);

      goto done;
    }
    else {
      count_ = count_*10 + (key - CKEY_TYPE_0);

      //count_str_ += CEvent::keyTypeChar(key);

      return;
    }
  }
  else if (key >= CKEY_TYPE_1 && key <= CKEY_TYPE_9) {
    count_ = count_*10 + (key - CKEY_TYPE_0);

    //count_str_ += CEvent::keyTypeChar(key);

    return;
  }

  //------

  if ((modifier & CMODIFIER_CONTROL) || (modifier & CMODIFIER_META))
    processControlChar(key, text, modifier);
  else
    processNormalChar(key, text, modifier);

  return;

 done:
  count_    = 0;
  lastKey_ = CKEY_TYPE_NUL;
}

void
CTextFileViKey::
processNormalChar(CKeyType key, const std::string &, CEventModifier modifier)
{
  switch (key) {
    // cursor movement
    case CKEY_TYPE_h:
    case CKEY_TYPE_BackSpace: {
      uint d = std::max(count_, 1U);

      if (getVisual())
        extendSelectLeft(d);
      else
        rmoveTo(-d, 0);

      break;
    }
    case CKEY_TYPE_Left: {
      uint d = std::max(count_, 1U);

      if ((modifier & CMODIFIER_SHIFT)) {
        for (uint i = 0; i < d; ++i) {
          util_->prevWord();
        }
      }
      else {
        if (getVisual())
          extendSelectLeft(d);
        else
          rmoveTo(-d, 0);
      }

      break;
    }
    case CKEY_TYPE_j: {
      uint d = std::max(count_, 1U);

      if (getVisual())
        extendSelectDown(d);
      else
        rmoveTo(0, d);

      break;
    }
    case CKEY_TYPE_Down: {
      uint d = std::max(count_, 1U);

      if ((modifier & CMODIFIER_SHIFT)) {
        int num = getPageLength();

        rmoveTo(0, num);

        notifyScrollTop();
      }
      else {
        if (getVisual())
          extendSelectDown(d);
        else
          rmoveTo(0, d);
      }

      break;
    }
    case CKEY_TYPE_k: {
      uint d = std::max(count_, 1U);

      if (getVisual())
        extendSelectUp(d);
      else
        rmoveTo(0, -d);

      break;
    }
    case CKEY_TYPE_Up: {
      if ((modifier & CMODIFIER_SHIFT)) {
        int num = getPageLength();

        rmoveTo(0, -num);

        notifyScrollBottom();
      }
      else {
        uint d = std::max(count_, 1U);

        if (getVisual())
          extendSelectUp(d);
        else
          rmoveTo(0, -d);
      }

      break;
    }
    case CKEY_TYPE_l:
    case CKEY_TYPE_Space:
    case CKEY_TYPE_FF: {
      uint d = std::max(count_, 1U);

      if (getVisual())
        extendSelectRight(d);
      else
        rmoveTo(d, 0);

      break;
    }
    case CKEY_TYPE_Right: {
      uint d = std::max(count_, 1U);

      if ((modifier & CMODIFIER_SHIFT)) {
        for (uint i = 0; i < d; ++i) {
          util_->nextWord();
        }
      }
      else {
        if (getVisual())
          extendSelectRight(d);
        else
          rmoveTo(d, 0);
      }

      break;
    }
    case CKEY_TYPE_Insert: {
      setOverwrite(! getOverwrite());

      break;
    }
    case CKEY_TYPE_Home: {
      moveTo(0, 0);

      break;
    }
    case CKEY_TYPE_End: {
      moveTo(0, file_->getNumLines() - 1);

      break;
    }
    case CKEY_TYPE_b: { // back to beginning of word
      uint d = std::max(count_, 1U);

      for (uint i = 0; i < d; ++i)
        util_->prevWord();

      break;
    }
    case CKEY_TYPE_B: { // back to beginning of word (ignoring punctuation)
      uint d = std::max(count_, 1U);

      for (uint i = 0; i < d; ++i)
        util_->prevWORD();

      break;
    }
    case CKEY_TYPE_w: {
      uint d = std::max(count_, 1U);

      for (uint i = 0; i < d; ++i)
        util_->nextWord();

      break;
    }
    case CKEY_TYPE_W: {
      uint d = std::max(count_, 1U);

      for (uint i = 0; i < d; ++i)
        util_->nextWORD();

      break;
    }
    case CKEY_TYPE_e: { // move to end of word
      uint d = std::max(count_, 1U);

      for (uint i = 0; i < d; ++i)
        util_->endWord();

      break;
    }
    case CKEY_TYPE_E: { // move to end of word (ignore punctuation)
      uint d = std::max(count_, 1U);

      for (uint i = 0; i < d; ++i)
        util_->endWORD();

      break;
    }

    //----------

    // scroll
    case CKEY_TYPE_Page_Down:
    case CKEY_TYPE_ACK: {
      int num = getPageLength();

      CIPoint2D pos = getPos();

      if (pos.y + num < int(file_->getNumLines())) {
        rmoveTo(0, num);

        notifyScrollTop();
      }

      break;
    }
    case CKEY_TYPE_Page_Up:
    case CKEY_TYPE_STX: {
      int num = getPageLength();

      CIPoint2D pos = getPos();

      if (pos.y - num >= 0) {
        rmoveTo(0, -num);

        notifyScrollBottom();
      }

      break;
    }
    case CKEY_TYPE_Sys_Req: {
      int num = getPageLength() / 2;

      rmoveTo(0, -num);

      notifyScrollBottom();

      break;
    }
    case CKEY_TYPE_EOT: {
      int num = getPageLength() / 2;

      rmoveTo(0, num);

      notifyScrollTop();

      break;
    }
    case CKEY_TYPE_ENQ: {
      int row1 = file_->getPageTop();
      int row2 = getRow();

      int num = row2 - row1 - 1;

      if (num > 0) {
        rmoveTo(0, -num);

        notifyScrollTop();

        rmoveTo(0, num);
      }
      else {
        rmoveTo(0, 1);

        notifyScrollTop();
      }

      break;
    }
    case CKEY_TYPE_EM: {
      int row1 = file_->getPageBottom();
      int row2 = getRow();

      int num = row1 - row2 - 1;

      if (num > 0) {
        rmoveTo(0, num);

        notifyScrollBottom();

        rmoveTo(0, -num);
      }
      else {
        rmoveTo(0, -1);

        notifyScrollBottom();
      }

      break;
    }

    //----------

    case CKEY_TYPE_BEL: {
      std::cout << file_->getFileName() << " " << file_->getNumLines() << " lines" << std::endl;

      break;
    }
    case CKEY_TYPE_DC2: {
      undo_->redo();

      break;
    }

    //----------

    // Multi-char commands
    case CKEY_TYPE_c:
    case CKEY_TYPE_d:
    case CKEY_TYPE_f:
    case CKEY_TYPE_g:
    case CKEY_TYPE_m:
    case CKEY_TYPE_r:
    case CKEY_TYPE_t:
    case CKEY_TYPE_y:
    case CKEY_TYPE_z:
    case CKEY_TYPE_F:
    case CKEY_TYPE_T:
    case CKEY_TYPE_Z:
    case CKEY_TYPE_QuoteLeft:
    case CKEY_TYPE_QuoteDbl:
    case CKEY_TYPE_Apostrophe:
    case CKEY_TYPE_Less:
    case CKEY_TYPE_Greater:
    case CKEY_TYPE_BracketLeft:
    case CKEY_TYPE_BracketRight:
    case CKEY_TYPE_Exclam: {
      lastKey_ = key;
      return;
    }

    //----------

    case CKEY_TYPE_Minus: // first non-blank up
      util_->moveToFirstNonBlankUp();
      break;

    case CKEY_TYPE_Plus: // first non-blank down
    case CKEY_TYPE_Return:
      util_->moveToFirstNonBlankDown();
      break;

    case CKEY_TYPE_Underscore: // first non-blank
    case CKEY_TYPE_AsciiCircum:
      util_->moveToFirstNonBlank();
      break;

    case CKEY_TYPE_AsciiTilde:
      util_->swapChar();

      rmoveTo(1, 0);

      break;

    case CKEY_TYPE_NumberSign: { // find string under cursor backward
      std::string word;

      if (util_->getWord(word)) {
        uint fline_num, fchar_num;

        util_->findPrev(word, getRow(), getCol() - 1, 0, 0, &fline_num, &fchar_num);
      }

      break;
    }

    case CKEY_TYPE_Asterisk: { // find string under cursor forward
      std::string word;

      if (util_->getWord(word)) {
        uint fline_num, fchar_num;

        util_->findNext(word, getRow(), getCol() + 1, file_->getNumLines() - 1, -1,
                        &fline_num, &fchar_num);
      }

      break;
    }

    case CKEY_TYPE_Dollar: {
      uint num = std::max(count_, 1U) - 1;

      rmoveTo(0, num);

      CIPoint2D pos = getPos();

      moveTo(file_->getLineLength() - 1, pos.y);

      break;
    }

    case CKEY_TYPE_Percent: { // percent of file (count)
                              // find matching pair (no count)
      if (count_ > 0) {
        if (count_ <= 100) {
          int pos = (count_*(file_->getNumLines() - 1))/100;

          moveTo(0, pos);
        }
      }
      else {
        char c = file_->getChar();

        if (! strchr("([{}])", c)) {
          if (util_->findNextChar(getRow(), getCol(), "([{}])", false))
            c = file_->getChar();
          else
            break;
        }

        if      (c == '(')
          util_->findNextChar(getRow(), getCol(), ')', true);
        else if (c == '[')
          util_->findNextChar(getRow(), getCol(), ']', true);
        else if (c == '{')
          util_->findNextChar(getRow(), getCol(), '}', true);
        else if (c == '}')
          util_->findPrevChar(getRow(), getCol(), '{', true);
        else if (c == ']')
          util_->findPrevChar(getRow(), getCol(), '[', true);
        else if (c == ')')
          util_->findPrevChar(getRow(), getCol(), '(', true);
      }

      break;
    }

    case CKEY_TYPE_Ampersand: // repeat last search/replace
      error("Unimplemented");
      break;

    case CKEY_TYPE_ParenLeft: // sentence backward
      util_->prevSentence();

      break;
    case CKEY_TYPE_ParenRight: // sentence forward
      util_->nextSentence();

      break;
    case CKEY_TYPE_BraceLeft: // paragraph backward
      util_->prevParagraph();

      break;
    case CKEY_TYPE_BraceRight: // paragraph forward
      util_->nextParagraph();

      break;

    case CKEY_TYPE_At: // execute command in buffer
      error("Unimplemented");
      break;

    case CKEY_TYPE_Equal: // filter through format command
      error("Unimplemented");
      break;

    case CKEY_TYPE_p: {
      file_->startGroup();

      for (uint i = 0; i < std::max(count_, 1U); ++i)
        buffer_->pasteAfter(register_);

      file_->endGroup();

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;
    }
    case CKEY_TYPE_P: {
      file_->startGroup();

      for (uint i = 0; i < std::max(count_, 1U); ++i)
        buffer_->pasteBefore(register_);

      file_->endGroup();

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;
    }

    case CKEY_TYPE_Q: // ex mode
      error("Unimplemented");
      break;

    case CKEY_TYPE_R: // replace mode
      setInsertMode(true);

      setOverwrite(true);

      break;

    case CKEY_TYPE_Y:
      buffer_->yankLines(register_, count_);

      break;

    case CKEY_TYPE_Bar: { // to column
      CIPoint2D pos = getPos();

      moveTo(0, pos.y);

      int num = count_ - 1;

      rmoveTo(num, 0);

      break;
    }

    case CKEY_TYPE_S: // 'cc' - TODO: count
      setInsertMode(true);

      file_->deleteLineAt();

      file_->addLineBefore("");

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;

    case CKEY_TYPE_D: // delete to end of line
      util_->deleteEOL();

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;

    case CKEY_TYPE_G:
      marks_->markReturn();

      // goto line
      if (count_ == 0) {
        moveTo(0, file_->getNumLines() - 1);

        CIPoint2D pos = getPos();

        moveTo(file_->getLineLength() - 1, pos.y);
      }
      else
        moveTo(0, count_ - 1);

      break;
    case CKEY_TYPE_H: {
      int row = file_->getPageTop();

      moveTo(0, row);

      break;
    }
    case CKEY_TYPE_M: {
      int row = (file_->getPageBottom() + file_->getPageTop())/2;

      moveTo(0, row);

      break;
    }
    case CKEY_TYPE_L: {
      int row = file_->getPageBottom();

      moveTo(0, row);

      break;
    }

    case CKEY_TYPE_J:
      util_->joinLine();

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;

    case CKEY_TYPE_Colon:
      enterCmdLineMode(":");

      break;

    case CKEY_TYPE_Slash:
      enterCmdLineMode("/");

      break;
    case CKEY_TYPE_Question:
      enterCmdLineMode("?");

      break;
    case CKEY_TYPE_n:
      findNext();

      break;
    case CKEY_TYPE_N:
      findPrev();

      break;

    case CKEY_TYPE_K:
      error("Unimplemented");
      break;

    case CKEY_TYPE_C: // change to end of line
      setInsertMode(true);

      util_->deleteEOL();

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;

    case CKEY_TYPE_q:
      error("Unimplemented");
      break;

    case CKEY_TYPE_u: // undo last change
      undo_->undo();

      break;

    case CKEY_TYPE_U: // undo line
      //undo_->undoLine();

      break;

    case CKEY_TYPE_a: // append text after cursor
      setInsertMode(true);

      rmoveTo(1, 0);

      break;
    case CKEY_TYPE_A: { // Append text at end of line
      setInsertMode(true);

      CIPoint2D pos = getPos();

      moveTo(file_->getLineLength(), pos.y);

      break;
    }

    case CKEY_TYPE_i:
      setInsertMode(true);

      break;

    case CKEY_TYPE_I: {
      setInsertMode(true);

      CIPoint2D pos = getPos();

      moveTo(0, pos.y);

      util_->moveToNonBlank();

      break;
    }

    case CKEY_TYPE_o:
      setInsertMode(true);

      file_->addLineAfter("");

      rmoveTo(0, 1);

      break;
    case CKEY_TYPE_O:
      setInsertMode(true);

      file_->addLineBefore("");

      break;

    case CKEY_TYPE_Backslash:
      error("Unimplemented");
      break;

    case CKEY_TYPE_s: // Synonym for 'cl'
      error("Unimplemented");
      break;

#if 0
    case CKEY_TYPE_g:
      error("Unimplemented");
      break;
#endif

    case CKEY_TYPE_Semicolon:
      if (findChar_ != '\0')
        doFindChar(findChar_, count_, findForward_, findTill_);

      break;
    case CKEY_TYPE_Comma:
      if (findChar_ != '\0') {
        bool saveFindForward = findForward_;

        doFindChar(findChar_, count_, ! findForward_, findTill_);

        findForward_ = saveFindForward;
      }

      break;

    case CKEY_TYPE_x:
    case CKEY_TYPE_DEL:
      file_->deleteCharAt();

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;
    case CKEY_TYPE_X:
      rmoveTo(-1, 0);
      file_->deleteCharAt();

      lastCommand_.clear();
      lastCommand_.addKey(key);

      break;

    case CKEY_TYPE_v: // char visual
    case CKEY_TYPE_V: // line visual
      setVisual(! getVisual());

      if (! getVisual())
        sel_->clearSelection();
      else
        sel_->setSelectRange(getPos(), sel_->getSelectEnd());

      break;

    case CKEY_TYPE_Period: {
      lastCommand_.exec();

      break;
    }

    case CKEY_TYPE_TAB:
    case CKEY_TYPE_Tab:
    case CKEY_TYPE_KP_Tab: {
      rmoveTo(getTabStop(), 0);

      break;
    }

    case CKEY_TYPE_Escape:
      break;

    default:
      error("Unsupported key " + CEvent::keyTypeName(key));
      goto done;
  }

 done:
  count_    = 0;
  lastKey_ = CKEY_TYPE_NUL;
  register_ = '\0';
}

void
CTextFileViKey::
processControlChar(CKeyType key, const std::string &, CEventModifier )
{
  switch (key) {
    case CKEY_TYPE_a: // unused
      break;
    case CKEY_TYPE_c: // unused
      break;

    case CKEY_TYPE_b: // scroll back one window
    case CKEY_TYPE_STX: {
      int num = getPageLength();

      rmoveTo(0, -num);

      notifyScrollBottom();

      break;
    }
    case CKEY_TYPE_d: // scroll down half a page
    case CKEY_TYPE_EOT: {
      int num = getPageLength() / 2;

      rmoveTo(0, num);

      notifyScrollTop();

      break;
    }
    case CKEY_TYPE_e: // scroll down one more line
    case CKEY_TYPE_ENQ: {
      int row1 = file_->getPageTop();
      int row2 = getRow();

      int num = row2 - row1 - 1;

      if (num > 0) {
        rmoveTo(0, -num);

        notifyScrollTop();

        rmoveTo(0, num);
      }
      else {
        rmoveTo(0, 1);

        notifyScrollTop();
      }

      break;
    }
    case CKEY_TYPE_f:
    case CKEY_TYPE_ACK: {
      int num = getPageLength();

      rmoveTo(0, num);

      notifyScrollTop();

      break;
    }
    case CKEY_TYPE_g:
    case CKEY_TYPE_BEL: {
      std::string msg =
        CStrUtil::strprintf("\"%s\" %d lines", file_->getFileName().c_str(),
                            file_->getNumLines());

      showStatusMsg(msg);

      break;
    }
    case CKEY_TYPE_h:
    case CKEY_TYPE_BackSpace:
      rmoveTo(-1, 0);

      break;
    case CKEY_TYPE_l:
    case CKEY_TYPE_FF: { // mark all lines changed ?
      //file_->setIgnoreChanged(true);

      //file_->updateSyntax();

      break;
    }
    case CKEY_TYPE_r:
    case CKEY_TYPE_DC2: {
      undo_->redo();

      break;
    }
    case CKEY_TYPE_u:
    case CKEY_TYPE_Sys_Req: {
      int num = getPageLength() / 2;

      rmoveTo(0, -num);

      notifyScrollBottom();

      break;
    }
    case CKEY_TYPE_v: // block visual
      // TODO
      break;
    case CKEY_TYPE_y:
    case CKEY_TYPE_EM: {
      int row1 = file_->getPageBottom();
      int row2 = getRow();

      int num = row1 - row2 - 1;

      if (num > 0) {
        rmoveTo(0, num);

        notifyScrollBottom();

        rmoveTo(0, -num);
      }
      else {
        rmoveTo(0, -1);

        notifyScrollBottom();
      }

      break;
    }
    default:
      break;
  }
}

void
CTextFileViKey::
processInsertChar(CKeyType key, const std::string &text, CEventModifier modifier)
{
  lastCommand_.addKey(key);

  if (CEvent::keyTypeIsAlpha(key) ||
      CEvent::keyTypeIsDigit(key)) {
    normalInsertChar(key, text, modifier);

    return;
  }

  switch (key) {
    case CKEY_TYPE_QuoteLeft:
    case CKEY_TYPE_Minus:
    case CKEY_TYPE_Equal:
      normalInsertChar(key, text, modifier);

      break;

    case CKEY_TYPE_BackSpace:
      if (getOverwrite()) {
        if      (rmoveTo(-1, 0)) {
          // TODO: revert char
        }
        else if (rmoveTo(0, -1)) {
          // TODO: revert char
        }
      }
      else {
        if      (rmoveTo(-1, 0))
          file_->deleteCharAt();
        else if (rmoveTo(0, -1)) {
          CIPoint2D pos = getPos();

          moveTo(file_->getLineLength() - 1, pos.y);

          util_->joinLine();
        }
      }

      break;
    case CKEY_TYPE_DEL:
      if (getOverwrite()) {
        if      (rmoveTo(-1, 0)) {
          // TODO: revert char
        }
        else if (rmoveTo(0, -1)) {
          // TODO: revert char
        }
      }
      else
        file_->deleteCharAt();

      break;

    case CKEY_TYPE_AsciiTilde:
    case CKEY_TYPE_Exclam:
    case CKEY_TYPE_At:
    case CKEY_TYPE_NumberSign:
    case CKEY_TYPE_Dollar:
    case CKEY_TYPE_Percent:
    case CKEY_TYPE_AsciiCircum:
    case CKEY_TYPE_Ampersand:
    case CKEY_TYPE_Asterisk:
    case CKEY_TYPE_ParenLeft:
    case CKEY_TYPE_ParenRight:
    case CKEY_TYPE_Underscore:
    case CKEY_TYPE_Plus:

    case CKEY_TYPE_BraceLeft:
    case CKEY_TYPE_BraceRight:
    case CKEY_TYPE_Bar:

    case CKEY_TYPE_Colon:
    case CKEY_TYPE_QuoteDbl:

    case CKEY_TYPE_Less:
    case CKEY_TYPE_Greater:
    case CKEY_TYPE_Question:

    case CKEY_TYPE_BracketLeft:
    case CKEY_TYPE_BracketRight:
    case CKEY_TYPE_Backslash:

    case CKEY_TYPE_Semicolon:
    case CKEY_TYPE_Apostrophe:

    case CKEY_TYPE_Comma:
    case CKEY_TYPE_Period:
    case CKEY_TYPE_Slash:
      normalInsertChar(key, text, modifier);

      break;

    case CKEY_TYPE_Escape:
      setInsertMode(false);

      rmoveTo(-1, 0);

      break;

    case CKEY_TYPE_Space:
      normalInsertChar(key, text, modifier);

      break;

    case CKEY_TYPE_Return:
      util_->splitLine();

      break;

    case CKEY_TYPE_Left:
      rmoveTo(-1, 0);
      break;
    case CKEY_TYPE_Up:
      rmoveTo(0, -1);
      break;
    case CKEY_TYPE_Right:
      rmoveTo(1, 0);
      break;
    case CKEY_TYPE_Down:
      rmoveTo(0, 1);
      break;

    case CKEY_TYPE_TAB:
    case CKEY_TYPE_Tab:
    case CKEY_TYPE_KP_Tab: {
      for (uint i = 0; i < getTabStop(); ++i)
        file_->addCharBefore(' ');

      break;
    }

    case CKEY_TYPE_Insert: {
      setOverwrite(! getOverwrite());

      break;
    }

    case CKEY_TYPE_Shift_L  : case CKEY_TYPE_Shift_R:
    case CKEY_TYPE_Control_L: case CKEY_TYPE_Control_R:
    case CKEY_TYPE_Caps_Lock: case CKEY_TYPE_Shift_Lock:
    case CKEY_TYPE_Meta_L   : case CKEY_TYPE_Meta_R:
    case CKEY_TYPE_Alt_L    : case CKEY_TYPE_Alt_R:
    case CKEY_TYPE_Super_L  : case CKEY_TYPE_Super_R:
    case CKEY_TYPE_Hyper_L  : case CKEY_TYPE_Hyper_R:
      break;

    default:
      error("Unsupported key " + CEvent::keyTypeName(key));
      break;
  }

  count_    = 0;
  lastKey_ = CKEY_TYPE_NUL;
  register_ = '\0';
}

void
CTextFileViKey::
normalInsertChar(CKeyType , const std::string &text, CEventModifier )
{
  if (getOverwrite())
    file_->replaceChar(text[0]);
  else
    file_->addCharBefore(text[0]);

  rmoveTo(1, 0);

  count_    = 0;
  lastKey_ = CKEY_TYPE_NUL;
  register_ = '\0';
}

bool
CTextFileViKey::
processMoveChar(CKeyType key, const std::string &, CEventModifier , CIPoint2D &new_pos)
{
  uint x = new_pos.x;
  uint y = new_pos.y;

  bool rc = true;

  switch (key) {
    case CKEY_TYPE_b:
      util_->prevWord(&y, &x);

      break;
    case CKEY_TYPE_B:
      util_->prevWORD(&y, &x);

      break;
    case CKEY_TYPE_w:
      util_->nextWord(&y, &x);

      break;
    case CKEY_TYPE_W:
      util_->endWord(&y, &x);

      break;
    case CKEY_TYPE_e:
      util_->endWord(&y, &x);

      break;
    case CKEY_TYPE_E:
      util_->endWORD(&y, &x);

      break;

    case CKEY_TYPE_h:
    case CKEY_TYPE_Left:
    case CKEY_TYPE_BackSpace:
      //rc = file_->cursorLeft (1, &y, &x);
      break;
    case CKEY_TYPE_j:
    case CKEY_TYPE_Down:
      //rc = file_->cursorDown (1, &y, &x);
      break;
    case CKEY_TYPE_k:
    case CKEY_TYPE_Up:
      //rc = file_->cursorUp   (1, &y, &x);
      break;
    case CKEY_TYPE_l:
    case CKEY_TYPE_Right:
    case CKEY_TYPE_Space:
      //rc = file_->cursorRight(1, &y, &x);

      break;
    case CKEY_TYPE_0: {
      CIPoint2D pos = getPos();

      moveTo(0, pos.y);

      break;
    }
    case CKEY_TYPE_Dollar: {
      CIPoint2D pos = getPos();

      moveTo(file_->getLineLength() - 1, pos.y);

      break;
    }
    case CKEY_TYPE_Minus:
      //util_->moveToFirstNonBlankUp(&y, &x);
      break;
    case CKEY_TYPE_Plus:
    case CKEY_TYPE_Return:
      //util_->moveToFirstNonBlankDown(&y, &x);
      break;
    case CKEY_TYPE_Underscore:
    case CKEY_TYPE_AsciiCircum:
      //util_->moveToFirstNonBlank(&y, &x);
      break;
    case CKEY_TYPE_BraceLeft:
      util_->prevParagraph(&y, &x);

      break;
    case CKEY_TYPE_BraceRight:
      util_->nextParagraph(&y, &x);

      break;
    default:
      rc = false;
      break;
  }

  if (rc)
    new_pos = CIPoint2D(x, y);

  return rc;
}

bool
CTextFileViKey::
doFindChar(char c, uint count, bool forward, bool till)
{
  findChar_    = c;
  findForward_ = forward;
  findTill_    = till;

  bool rc = true;

  if (forward) {
    for (uint i = 0; i < std::max(count, 1U); ++i) {
      if (! (rc = util_->findNextChar(getRow(), getCol(), c, false)))
        break;
    }

    if (till && rc)
      rmoveTo(-1, 0);
  }
  else {
    bool rc1 = true;

    for (uint i = 0; i < std::max(count, 1U); ++i) {
      if (! (rc1 = util_->findPrevChar(getRow(), getCol(), c, false)))
        break;
    }

    if (till && rc1)
      rmoveTo(1, 0);
  }

  return rc;
}

void
CTextFileViKey::
setOptionString(const std::string &name, const std::string &arg)
{
  std::string name1 = name;
  std::string arg1  = arg;

  if (name1.size() > 2 && name1.substr(0, 2) == "no")  {
    if (arg1 == "")
      arg1 = "0";

    name1 = name.substr(2);
  }
  else {
    if (arg1 == "")
      arg1 = "1";
  }

  optionMap_[name1] = arg1;

  if      (name1 == "ignorecase") {
    options_.ignorecase = CStrUtil::toBool(arg1);

    ed_->setCaseSensitive(! options_.ignorecase);
  }
  else if (name1 == "list")
    options_.list = CStrUtil::toBool(arg1);
  else if (name1 == "number") {
    options_.number = CStrUtil::toBool(arg1);

    notifyNumber(options_.number);
  }
  else if (name1 == "shiftwidth")
    options_.shiftwidth = int(CStrUtil::toInteger(arg1));
  else if (name1 == "showmatch")
    options_.showmatch = CStrUtil::toBool(arg1);
}

void
CTextFileViKey::
setInsertMode(bool insert_mode)
{
  if (insert_mode == insertMode_)
    return;

  insertMode_ = insert_mode;

  setOverwrite(false);

  //file_->stateChanged();

  if (insert_mode) {
    file_->startGroup();

    //file_->setExtraLineChar(true);
  }
  else {
    file_->endGroup();

    //file_->setExtraLineChar(false);
  }
}

bool
CTextFileViKey::
rmoveTo(int dx, int dy)
{
  uint x, y;

  file_->getPos(&x, &y);

  int y1 = int(y) + dy;

  int ymax = file_->getNumLines() - 1;

  y1 = std::min(std::max(y1, 0), ymax);

  file_->moveTo(x, y1);

  int x1 = int(x) + dx;

  int xmax = file_->getLineLength() - (getInsertMode() ? 0 : 1);

  x1 = std::min(std::max(x1, 0), xmax);

  file_->moveTo(x1, y1);

  return true;
}

bool
CTextFileViKey::
moveTo(uint x, uint y)
{
  uint x1, y1;

  file_->getPos(&x1, &y1);

  int ymax = file_->getNumLines() - 1;

  y = std::min(std::max(int(y), 0), ymax);

  file_->moveTo(x1, y);

  int xmax = file_->getLineLength() - (getInsertMode() ? 0 : 1);

  x = std::min(std::max(int(x), 0), xmax);

  file_->moveTo(x, y);

  return true;
}

void
CTextFileViKey::
edNotifyQuit(bool)
{
  notifyQuit();
}

void
CTextFileViKey::
error(const std::string &msg) const
{
  std::cerr << msg << std::endl;
}

//-------

CTextFileViKey::LastCommand::
LastCommand(CTextFile *file) :
 file_(file)
{
}

void
CTextFileViKey::LastCommand::
clear()
{
  keys_.clear();
}

void
CTextFileViKey::LastCommand::
addCount(uint n)
{
  if (n == 0) return;

  std::string str = CStrUtil::toString(n);

  uint len = uint(str.size());

  for (uint i = 0; i < len; ++i) {
    CKeyType key = CEvent::charKeyType(str[i]);

    addKey(key);
  }
}

void
CTextFileViKey::LastCommand::
addKey(CKeyType key)
{
  keys_.push_back(key);
}

void
CTextFileViKey::LastCommand::
exec()
{
#if 0
  static char text[2];

  uint len = keys_.size();

  for (uint i = 0; i < len; ++i) {
    char c = CEvent::keyTypeChar(keys_[i]);

    text[0] = c;

    file_->keyPress(keys_[i], text);
  }
#endif
}

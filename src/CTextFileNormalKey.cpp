#include <CTextFileNormalKey.h>
#include <CTextFile.h>
#include <CTextFileSel.h>
#include <CTextFileUtil.h>
#include <CTextFileUndo.h>

CTextFileNormalKey::
CTextFileNormalKey(CTextFile *file) :
 CTextFileKey(file)
{
}

CTextFileNormalKey::
~CTextFileNormalKey()
{
}

void
CTextFileNormalKey::
processChar(CKeyType key, const std::string &text, CEventModifier modifier)
{
  if ((modifier & CMODIFIER_CONTROL) || (modifier & CMODIFIER_META))
    processControlChar(key, text, modifier);
  else
    processNormalChar(key, text, modifier);
}

void
CTextFileNormalKey::
execCmd(const std::string &)
{
}

void
CTextFileNormalKey::
processControlChar(CKeyType key, const std::string &, CEventModifier)
{
  switch (key) {
    case CKEY_TYPE_c: { // copy
      break;
    }
    case CKEY_TYPE_f: { // find
      break;
    }
    case CKEY_TYPE_q: { // quit
      notifyQuit();

      break;
    }
    case CKEY_TYPE_s: { // save
      file_->write(file_->getFileName().c_str());

      break;
    }
    case CKEY_TYPE_v: { // paste
      break;
    }
    case CKEY_TYPE_y: { // redo
      undo_->redo();

      break;
    }
    case CKEY_TYPE_z: { // undo
      undo_->undo();

      break;
    }
    default:
      break;
  }
}

void
CTextFileNormalKey::
processNormalChar(CKeyType key, const std::string &text, CEventModifier modifier)
{
  if (CEvent::keyTypeIsAlpha(key) || CEvent::keyTypeIsDigit(key)) {
    if (getOverwrite())
      file_->replaceChar(text[0]);
    else {
      clearSelection();

      file_->addCharBefore(text[0]);
    }

    file_->rmoveTo(1, 0);

    return;
  }

  switch (key) {
    case CKEY_TYPE_Space:
    case CKEY_TYPE_Exclam:
    case CKEY_TYPE_QuoteDbl:
    case CKEY_TYPE_NumberSign:
    case CKEY_TYPE_Dollar:
    case CKEY_TYPE_Percent:
    case CKEY_TYPE_Ampersand:
    case CKEY_TYPE_Apostrophe:
    case CKEY_TYPE_ParenLeft:
    case CKEY_TYPE_ParenRight:
    case CKEY_TYPE_Asterisk:
    case CKEY_TYPE_Plus:
    case CKEY_TYPE_Comma:
    case CKEY_TYPE_Minus:
    case CKEY_TYPE_Period:
    case CKEY_TYPE_Slash:

    case CKEY_TYPE_Colon:
    case CKEY_TYPE_Semicolon:
    case CKEY_TYPE_Less:
    case CKEY_TYPE_Equal:
    case CKEY_TYPE_Greater:
    case CKEY_TYPE_Question:
    case CKEY_TYPE_At:

    case CKEY_TYPE_BracketLeft:
    case CKEY_TYPE_Backslash:
    case CKEY_TYPE_BracketRight:
    case CKEY_TYPE_AsciiCircum:
    case CKEY_TYPE_Underscore:
    case CKEY_TYPE_QuoteLeft:
    case CKEY_TYPE_BraceLeft:
    case CKEY_TYPE_Bar:
    case CKEY_TYPE_BraceRight:
    case CKEY_TYPE_AsciiTilde:
      if (getOverwrite())
        file_->replaceChar(text[0]);
      else {
        clearSelection();

        file_->addCharBefore(text[0]);
      }

      file_->rmoveTo(1, 0);

      break;

    case CKEY_TYPE_Escape:
      break;

    case CKEY_TYPE_BackSpace: {
      CIPoint2D pos = getPos();

      if (sel_->insideSelection(pos))
        clearSelection();
      else {
        if      (pos.x > 0) {
          file_->rmoveTo(-1, 0);
          file_->deleteCharAt();
        }
        else if (pos.y > 0) {
          file_->rmoveTo(0, -1);

          uint len = file_->getLineLength();

          util_->joinLine();

          file_->moveTo(len, pos.y - 1);
        }
      }

      break;
    }
    case CKEY_TYPE_DEL: {
      CIPoint2D pos = getPos();

      if (sel_->insideSelection(pos))
        clearSelection();
      else
        file_->deleteCharAt();

      break;
    }

    case CKEY_TYPE_KP_Enter:
    case CKEY_TYPE_Return:
      util_->splitLine();

      file_->moveTo(0, getRow() + 1);

      break;

    case CKEY_TYPE_Left: {
      if      (getCol() > 0) {
        if (modifier & CMODIFIER_SHIFT)
          extendSelectLeft();
        else
          file_->rmoveTo(-1, 0);
      }
      else if (getRow() > 0) {
        file_->moveTo(0, getRow() - 1);

        file_->rmoveTo(file_->getLineLength(), 0);
      }

      break;
    }

    case CKEY_TYPE_Up: {
      if (modifier & CMODIFIER_SHIFT)
        extendSelectUp();
      else
        file_->rmoveTo(0, -1);

      break;
    }

    case CKEY_TYPE_Right: {
      if      (getCol() < file_->getLineLength()) {
        if (modifier & CMODIFIER_SHIFT)
          extendSelectRight();
        else
          file_->rmoveTo(1, 0);
      }
      else if (getRow() < file_->getNumLines()) {
        file_->moveTo(0, getRow() + 1);
      }

      break;
    }

    case CKEY_TYPE_Down: {
      if (modifier & CMODIFIER_SHIFT)
        extendSelectDown();
      else
        file_->rmoveTo(0, 1);

      break;
    }

    case CKEY_TYPE_Page_Down: {
      int num = getPageLength();

      file_->rmoveTo(0, num);

      notifyScrollTop();

      break;
    }
    case CKEY_TYPE_Page_Up: {
      int num = getPageLength();

      file_->rmoveTo(0, -num);

      notifyScrollBottom();

      break;
    }

    case CKEY_TYPE_Home: {
      uint x, y;

      file_->getPos(&x, &y);

      file_->moveTo(0, y);

      notifyScrollTop();

      break;
    }
    case CKEY_TYPE_End: {
      uint x, y;

      file_->getPos(&x, &y);

      file_->moveTo(file_->getLineLength() - 1, y);

      notifyScrollBottom();

      break;
    }

    case CKEY_TYPE_Insert: {
      setOverwrite(! getOverwrite());

      break;
    }

    case CKEY_TYPE_TAB:
    case CKEY_TYPE_Tab:
    case CKEY_TYPE_KP_Tab: {
      uint tab_stop = getTabStop();

      for (uint i = 0; i < tab_stop; ++i)
        file_->addCharBefore(' ');

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

    case CKEY_TYPE_SOH: // Select All (Ctrl+A)
      // TODO

      break;
    case CKEY_TYPE_EOT: // Delete Line (Ctrl+D)
      // TODO

      break;
    case CKEY_TYPE_Pause: // Save (Ctrl+S)
      file_->write(file_->getFileName().c_str());

      break;
    case CKEY_TYPE_DC1: // Quit (Ctrl+Q)
      notifyQuit();

      break;
    case CKEY_TYPE_EM: // Undo (Ctrl+Y)
      undo_->redo();

      break;
    case CKEY_TYPE_SUB: // Undo (Ctrl+Z)
      undo_->undo();

      break;
    default:
      //file_->getVi()->error("Unsupported key " + CEvent::keyTypeName(key));

      break;
  }
}

void
CTextFileNormalKey::
clearSelection()
{
  if (! sel_->isSelected()) return;

  const CIPoint2D &start = sel_->getSelectStart();
  const CIPoint2D &end   = sel_->getSelectEnd  ();

  for (int row = end.y; row >= start.y; --row) {
    if      (sel_->isLineInside(row)) {
      file_->moveTo(0, row);

      file_->deleteLineAt();
    }
    else if (sel_->isPartLineInside(row)) {
      const std::string &line = file_->getLine(row);

      int col1 = 0;

      while (col1 < int(line.size()) && ! sel_->isCharInside(row, col1))
        ++col1;

      if (col1 >= int(line.size())) continue;

      int col2 = col1;
      int col3 = col2;

      while (col3 < int(line.size()) && sel_->isCharInside(row, col3)) {
        col2 = col3;

        ++col3;
      }

      util_->deleteChars(row, col1, col2 - col1 + 1);
    }
  }

  file_->moveTo(start.x, start.y);

  sel_->clearSelection();
}

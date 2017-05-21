#include <CTextFileUndo.h>
#include <CTextFile.h>
#include <cstdlib>

CTextFileUndo::
CTextFileUndo(CTextFile *file) :
 file_ (file),
 undo_ (),
 debug_(false)
{
  if (getenv("CTEXT_FILE_UNDO_DEBUG") != NULL)
    debug_ = true;

  file_->addNotifier(this);
}

CTextFileUndo::
~CTextFileUndo()
{
  file_->removeNotifier(this);
}

void
CTextFileUndo::
reset()
{
  undo_.clear();
}

void
CTextFileUndo::
undo()
{
  undo_.undo();
}

void
CTextFileUndo::
redo()
{
  undo_.redo();
}

//----

void
CTextFileUndo::
fileOpened(const std::string &)
{
  reset();
}

void
CTextFileUndo::
lineAdded(const std::string &line, uint line_num)
{
  addUndo(new CTextFileUndoDeleteLineCmd(this, line_num, line));
}

void
CTextFileUndo::
lineDeleted(const std::string &line, uint line_num)
{
  addUndo(new CTextFileUndoAddLineCmd(this, line_num, line));
}

void
CTextFileUndo::
lineReplaced(const std::string &line1, const std::string &line2, uint line_num)
{
  addUndo(new CTextFileUndoReplaceLineCmd(this, line_num, line1, line2));
}

void
CTextFileUndo::
charAdded(char c, uint line_num, uint char_num)
{
  addUndo(new CTextFileUndoDeleteCharCmd(this, line_num, char_num, c));
}

void
CTextFileUndo::
charDeleted(char c, uint line_num, uint char_num)
{
  addUndo(new CTextFileUndoAddCharCmd(this, line_num, char_num, c));
}

void
CTextFileUndo::
charReplaced(char c1, char c2, uint line_num, uint char_num)
{
  addUndo(new CTextFileUndoReplaceCharCmd(this, line_num, char_num, c1, c2));
}

void
CTextFileUndo::
startGroup()
{
  undo_.startGroup();
}

void
CTextFileUndo::
endGroup()
{
  undo_.endGroup();
}

void
CTextFileUndo::
addUndo(CTextFileUndoCmd *cmd)
{
  if (! undo_.locked())
    undo_.addUndo(cmd);
  else
    delete cmd;
}

//------

CTextFileUndoAddLineCmd::
CTextFileUndoAddLineCmd(CTextFileUndo *undo, uint line_num, const std::string &line) :
 CTextFileUndoCmd(undo), line_(line)
{
  line_num_ = line_num;

  if (undo_->getDebug())
    std::cerr << "Add: Add Line " << line_num << " '" << line << "'" << std::endl;
}

bool
CTextFileUndoAddLineCmd::
exec()
{
  if (getState() == UNDO_STATE) {
    if (undo_->getDebug())
      std::cerr << "Exec: Add Line " << line_num_ << " '" << line_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->addLineBefore(line_);
  }
  else {
    if (undo_->getDebug())
      std::cerr << "Exec: Delete Line " << line_num_ << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->deleteLineAt();
  }

  return true;
}

//------

CTextFileUndoDeleteLineCmd::
CTextFileUndoDeleteLineCmd(CTextFileUndo *undo, uint line_num, const std::string &line) :
 CTextFileUndoCmd(undo), line_(line)
{
  line_num_ = line_num;

  if (undo_->getDebug())
    std::cerr << "Add: Delete Line " << line_num << " '" << line << "'" << std::endl;
}

bool
CTextFileUndoDeleteLineCmd::
exec()
{
  if (getState() == UNDO_STATE) {
    if (undo_->getDebug())
      std::cerr << "Exec: Delete Line " << line_num_ << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->deleteLineAt();
  }
  else {
    if (undo_->getDebug())
      std::cerr << "Exec: Add Line " << line_num_ << " '" << line_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->addLineBefore(line_);
  }

  return true;
}

//------

CTextFileUndoReplaceLineCmd::
CTextFileUndoReplaceLineCmd(CTextFileUndo *undo, uint line_num, const std::string &line1,
                            const std::string &line2) :
 CTextFileUndoCmd(undo), line1_(line1), line2_(line2)
{
  line_num_ = line_num;

  if (undo_->getDebug())
    std::cerr << "Add: Replace Line " << line_num << " '" << line1 << "' '" <<
                 line2 << "'" << std::endl;
}

bool
CTextFileUndoReplaceLineCmd::
exec()
{
  if (getState() == UNDO_STATE) {
    if (undo_->getDebug())
      std::cerr << "Exec: Replace Line " << line_num_ << " '" << line1_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->replaceLine(line1_);
  }
  else {
    if (undo_->getDebug())
      std::cerr << "Exec: Replace Line " << line_num_ << " '" << line2_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->replaceLine(line2_);
  }

  return true;
}

//------

CTextFileUndoAddCharCmd::
CTextFileUndoAddCharCmd(CTextFileUndo *undo, uint line_num, uint char_num, char c) :
 CTextFileUndoCmd(undo), c_(c)
{
  line_num_ = line_num;
  char_num_ = char_num;

  if (undo_->getDebug())
    std::cerr << "Add: Add Char " << line_num << " " << char_num_ << " '" <<
                 c << "'" << std::endl;
}

bool
CTextFileUndoAddCharCmd::
exec()
{
  if (getState() == UNDO_STATE) {
    if (undo_->getDebug())
      std::cerr << "Exec: Add Char " << line_num_ << ":" << char_num_ << " '" <<
                 c_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->addCharBefore(c_);
  }
  else {
    if (undo_->getDebug())
      std::cerr << "Exec: Delete Char " << line_num_ << ":" << char_num_ << " " << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->deleteCharAt();
  }

  return true;
}

//------

CTextFileUndoDeleteCharCmd::
CTextFileUndoDeleteCharCmd(CTextFileUndo *undo, uint line_num, uint char_num, char c) :
 CTextFileUndoCmd(undo), c_(c)
{
  line_num_ = line_num;
  char_num_ = char_num;

  if (undo_->getDebug())
    std::cerr << "Add: Delete Char " << line_num << " " << char_num_ << " '" <<
                 c << "'" << std::endl;
}

bool
CTextFileUndoDeleteCharCmd::
exec()
{
  if (getState() == UNDO_STATE) {
    if (undo_->getDebug())
      std::cerr << "Exec: Delete Char " << line_num_ << ":" << char_num_ << " " << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->deleteCharAt();
  }
  else {
    if (undo_->getDebug())
      std::cerr << "Exec: Add Char " << line_num_ << ":" << char_num_ << " '" <<
                   c_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->addCharBefore(c_);
  }

  return true;
}

//------

CTextFileUndoReplaceCharCmd::
CTextFileUndoReplaceCharCmd(CTextFileUndo *undo, uint line_num, uint char_num, char c1, char c2) :
 CTextFileUndoCmd(undo), c1_(c1), c2_(c2)
{
  line_num_ = line_num;
  char_num_ = char_num;

  if (undo_->getDebug())
    std::cerr << "Add: Replace Char " << line_num << " " << char_num_ <<
            " '" << c1 << "' '" << c2 << "'" << std::endl;
}

bool
CTextFileUndoReplaceCharCmd::
exec()
{
  if (getState() == UNDO_STATE) {
    if (undo_->getDebug())
      std::cerr << "Exec: Replace Char " << line_num_ << ":" << char_num_ <<
            " '" << c1_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->replaceChar(c1_);
  }
  else {
    if (undo_->getDebug())
      std::cerr << "Exec: Replace Char " << line_num_ << ":" << char_num_ <<
                   " '" << c2_ << "'" << std::endl;

    undo_->getFile()->moveTo(char_num_, line_num_);

    undo_->getFile()->replaceChar(c2_);
  }

  return true;
}

//------

CTextFileUndoCmd::
CTextFileUndoCmd(CTextFileUndo *undo) :
 undo_(undo)
{
  undo_->getFile()->getPos(&char_num_, &line_num_);
}

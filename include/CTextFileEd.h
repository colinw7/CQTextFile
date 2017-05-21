#ifndef CTEXT_FILE_ED_H
#define CTEXT_FILE_ED_H

#include <CRegExp.h>
#include <CAutoPtr.h>
#include <CIPoint2D.h>
#include <list>
#include <sys/types.h>

class CReadLine;
class CTextFileEd;
class CTextFile;
class CTextFileUtil;
class CTextFileMarks;
class CTextFileUndo;
class CStrParse;

class CTextFileEdNotifierMgr;

class CTextFileEdNotifier {
 public:
  CTextFileEdNotifier();

  virtual void edNotifyQuit(bool force);
};

class CEdPointCondition {
 public:
  CEdPointCondition() { }

  virtual ~CEdPointCondition() { }

  virtual bool execute(CTextFileEd *) { return true; }
};

class CEdLineCondition : public CEdPointCondition {
 private:
  int line_;

 public:
  CEdLineCondition(int line=-1) :
   CEdPointCondition(), line_(line) {
  }

  void setLine(int line) { line_ = line; }
  int getLine() const { return line_; }
};

class CEdRegExpCondition : public CEdPointCondition {
 private:
  CRegExp expr_;

 public:
  CEdRegExpCondition(const std::string &expr) :
   CEdPointCondition(), expr_(expr) {
  }
};

class CTextFileEd {
 private:
  typedef std::vector<std::string> StringList;

  // structure to represent an entered command
  class InputData {
   private:
    CEdLineCondition start_; // start position
    CEdLineCondition end_;   // end position
    char             cmd_;   // command name
    StringList       lines_; // lines (for a, c, i, ...)

   public:
    InputData() :
     start_(), end_(), cmd_('\0'), lines_() {
    }

    bool isCmd(char cmd) const { return (cmd_ == cmd); }

    void setCmd(char cmd) { cmd_ = cmd; }

    void setStartLine(int line) { start_.setLine(line); }
    void setEndLine  (int line) { end_  .setLine(line); }

    int getStartLine() { return start_.getLine(); }
    int getEndLine  () { return end_  .getLine(); }

    const StringList &getLines() const { return lines_; }

    void addLine(const std::string &line) { lines_.push_back(line); }

    void clearLines() { lines_.clear(); }
  };

 public:
  enum Mode {
    COMMAND,
    INPUT
  };

 public:
  CTextFileEd(CTextFile *file);

  virtual ~CTextFileEd();

  void init();

  void setUndo(CTextFileUndo *undo);

  void setMarks(CTextFileMarks *marks);

  void addNotifier   (CTextFileEdNotifier *notifier);
  void removeNotifier(CTextFileEdNotifier *notifier);

  bool execFile(const std::string &fileName);

  bool execCmd(const std::string &cmd);

  bool findNext(const std::string &str, int *line_num, int *char_num);
  bool findPrev(const std::string &str, int *line_num, int *char_num);

  Mode getMode() const { return mode_; }

  bool isQuit() const { return quit_; }

  bool getEx() const { return ex_; }
  void setEx(bool ex) { ex_ = ex; }

  bool getCaseSensitive() const { return case_sensitive_; }
  void setCaseSensitive(bool flag) { case_sensitive_ = flag; }

  int getCurrentLine() const { return cur_line_; }

  void doSubstitute(int i1, int i2, const std::string &find,
                    const std::string &replace, char mod);

  void doFindNext(int i1, int i2, const std::string &find);

  void doFindPrev(int i1, int i2, const std::string &find);

  void doGlob(int i1, int i2, const std::string &find, const std::string &cmd);

  void doJoin(int i1, int i2);

  void doMove(int i1, int i2, int i3);

  void doCopy(int i1, int i2, int i3);

  void doDelete(int i1, int i2);

  void doCopy(int i1, int i2);

  void doPaste(int i1);

  void doUndo();

  void doMark(int i1, char c);

  void doExecute(int i1, int i2, const std::string &cmdStr);

  void doPrint(int i1, int i2, bool numbered, bool eol);

  virtual void output(const std::string &msg);
  virtual void error (const std::string &msg);

  void addLine(uint row, const std::string &str);

  void deleteLine(uint row);

  void setPos(const CIPoint2D &p);

  uint getRow() const;
  uint getCol() const;

  const std::string &getFindPattern() const { return findPattern_; }

  void edNotifyQuit(bool force);

 private:
  bool parseCmd(CStrParse &parse);
  bool parseLineNum(CStrParse &parse, int &line_num, int &char_num, bool &all);

 private:
  CTextFileMarks *getMarks() const { return (alt_marks_ ? alt_marks_ : marks_); }

  CTextFileUndo *getUndo() const { return (alt_undo_ ? alt_undo_ : undo_); }

 private:
  CTextFileEd(const CTextFileEd &rhs);
  CTextFileEd &operator=(const CTextFileEd &rhs);

 private:
  CTextFile *              file_;
  CTextFileUtil *          util_;
  CTextFileMarks *         marks_;
  CTextFileMarks *         alt_marks_;
  CTextFileUndo *          undo_;
  CTextFileUndo *          alt_undo_;
  CTextFileEdNotifierMgr * notifyMgr_;
  Mode                     mode_; // current mode, command or input
  int                      cur_line_, cur_char_;
  int                      line_num1_, line_num2_;
  int                      char_num1_, char_num2_;
  int                      num_lines_;
  InputData                input_data_;
  std::string              findPattern_;
  bool                     ex_; // vi/ex mode
  bool                     case_sensitive_; // vi/ex mode
  bool                     quit_;
};

class CTextFileEdNotifierMgr {
 public:
  CTextFileEdNotifierMgr(CTextFileEd *ed);

  void addNotifier   (CTextFileEdNotifier *notifier);
  void removeNotifier(CTextFileEdNotifier *notifier);

  void edNotifyQuit(bool force);

 private:
  typedef std::list<CTextFileEdNotifier *> NotifierList;

  CTextFileEd  *ed_;
  NotifierList  notifierList_;
};

#endif

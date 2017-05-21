#include <CTextFile.h>
#include <CUndo.h>

class CTextFileUndo;

class CTextFileUndoCmd : public CUndoData {
 public:
  CTextFileUndoCmd(CTextFileUndo *undo);

  virtual ~CTextFileUndoCmd() { }

  virtual const char *getName() const = 0;

 private:
  CTextFileUndoCmd(const CTextFileUndoCmd &rhs);
  CTextFileUndoCmd &operator=(const CTextFileUndoCmd &rhs);

 protected:
  CTextFileUndo *undo_ { nullptr };
  uint           line_num_ { 0 };
  uint           char_num_ { 0 };
};

//---

class CTextFileUndoAddLineCmd : public CTextFileUndoCmd {
 public:
  CTextFileUndoAddLineCmd(CTextFileUndo *undo, uint line_num, const std::string &line);

  const char *getName() const { return "add_line"; }

  bool exec();

 private:
  std::string line_;
};

//---

class CTextFileUndoDeleteLineCmd : public CTextFileUndoCmd {
 public:
  CTextFileUndoDeleteLineCmd(CTextFileUndo *undo, uint line_num, const std::string &line);

  const char *getName() const { return "delete_line"; }

  bool exec();

 private:
  std::string line_;
};

//---

class CTextFileUndoReplaceLineCmd : public CTextFileUndoCmd {
 public:
  CTextFileUndoReplaceLineCmd(CTextFileUndo *undo, uint line_num,
                              const std::string &line1, const std::string &line2);

  const char *getName() const { return "replace_line"; }

  bool exec();

 private:
  std::string line1_;
  std::string line2_;
};

//---

class CTextFileUndoAddCharCmd : public CTextFileUndoCmd {
 public:
  CTextFileUndoAddCharCmd(CTextFileUndo *undo, uint line_num, uint char_num, char c);

  const char *getName() const { return "add_char"; }

  bool exec();

 private:
  char c_ { '\0' };
};

//---

class CTextFileUndoDeleteCharCmd : public CTextFileUndoCmd {
 public:
  CTextFileUndoDeleteCharCmd(CTextFileUndo *undo, uint line_num, uint char_num, char c);

  const char *getName() const { return "delete_char"; }

  bool exec();

 private:
  char c_ { '\0' };
};

//---

class CTextFileUndoReplaceCharCmd : public CTextFileUndoCmd {
 public:
  CTextFileUndoReplaceCharCmd(CTextFileUndo *undo, uint line_num, uint char_num,
                              char c1, char c2);

  const char *getName() const { return "replace_char"; }

  bool exec();

 private:
  char c1_ { '\0' };
  char c2_ { '\0' };
};

//---

class CTextFileUndo : public CTextFileNotifier {
 public:
  CTextFileUndo(CTextFile *file);

  virtual ~CTextFileUndo();

  CTextFile *getFile() const { return file_; }

  // public API
  void reset();

  void undo();
  void redo();

  bool getDebug() const { return debug_; }

  // notifier interface
  void fileOpened  (const std::string &filename);
  void lineAdded   (const std::string &line, uint line_num);
  void lineDeleted (const std::string &line, uint line_num);
  void lineReplaced(const std::string &line1, const std::string &line2, uint line_num);
  void charAdded   (char c, uint line_num, uint char_num);
  void charDeleted (char c, uint line_num, uint char_num);
  void charReplaced(char c1, char c2, uint line_num, uint char_num);

  void startGroup();
  void endGroup  ();

 private:
  void addUndo(CTextFileUndoCmd *cmd);

 private:
  CTextFile *file_ { nullptr };
  CUndo      undo_;
  bool       debug_ { false };
};

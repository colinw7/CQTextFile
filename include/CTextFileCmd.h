#ifndef CTEXT_CMD_H
#define CTEXT_CMD_H

class CTextFile;

#include <string>
#include <map>

class CTextFileCmd {
 public:
  CTextFileCmd(CTextFile *file);

  bool exec(const std::string &cmd);

 private:
  typedef bool (CTextFileCmd::*CommandProc)(const std::string &cmd, const std::string &data);

  void addCommand(const std::string &name, CommandProc proc);

  CommandProc lookupCommand(const std::string &name);

  bool readCommand(const std::string &, const std::string &data);
  bool writeCommand(const std::string &, const std::string &data);
  bool moveToCommand(const std::string &, const std::string &data);
  bool rmoveToCommand(const std::string &, const std::string &data);
  bool addCharAfterCommand(const std::string &, const std::string &data);
  bool addCharBeforeCommand(const std::string &, const std::string &data);
  bool addLineAfterCommand(const std::string &, const std::string &data);
  bool addLineBeforeCommand(const std::string &, const std::string &data);
  bool deleteCharAtCommand(const std::string &, const std::string &data);
  bool deleteCharBeforeCommand(const std::string &, const std::string &data);
  bool deleteLineAtCommand(const std::string &, const std::string &data);
  bool deleteLineBeforeCommand(const std::string &, const std::string &data);
  bool replaceCharCommand(const std::string &, const std::string &data);
  bool replaceLineCommand(const std::string &, const std::string &data);

 private:
  typedef std::map<std::string,CommandProc> CmdMap;

  CTextFile *file_;
  CmdMap     cmds_;
};

#endif

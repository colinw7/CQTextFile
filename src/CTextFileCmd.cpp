#include <CTextFileCmd.h>
#include <CTextFile.h>
#include <CStrUtil.h>

CTextFileCmd::
CTextFileCmd(CTextFile *file) :
 file_(file)
{
  addCommand("r", &CTextFileCmd::readCommand);
  addCommand("w", &CTextFileCmd::writeCommand);
  addCommand("m", &CTextFileCmd::moveToCommand);
  addCommand("M", &CTextFileCmd::rmoveToCommand);
  addCommand("a", &CTextFileCmd::addCharAfterCommand);
  addCommand("A", &CTextFileCmd::addCharBeforeCommand);
  addCommand("l", &CTextFileCmd::addLineAfterCommand);
  addCommand("L", &CTextFileCmd::addLineBeforeCommand);
  addCommand("x", &CTextFileCmd::deleteCharAtCommand);
  addCommand("X", &CTextFileCmd::deleteCharBeforeCommand);
  addCommand("d", &CTextFileCmd::deleteLineAtCommand);
  addCommand("D", &CTextFileCmd::deleteLineBeforeCommand);
  addCommand("r", &CTextFileCmd::replaceCharCommand);
  addCommand("R", &CTextFileCmd::replaceLineCommand);
}

void
CTextFileCmd::
addCommand(const std::string &name, CommandProc proc)
{
  cmds_[name] = proc;
}

CTextFileCmd::CommandProc
CTextFileCmd::
lookupCommand(const std::string &name)
{
  CmdMap::const_iterator p = cmds_.find(name);

  if (p == cmds_.end())
    return NULL;

  return (*p).second;
}

#define CALL_MEMBER_FN(object,ptrToMember) ((object).*(ptrToMember))

bool
CTextFileCmd::
exec(const std::string &cmd)
{
  uint i = 0;

  CStrUtil::skipSpace(cmd, &i);

  uint i1 = i;

  CStrUtil::skipNonSpace(cmd, &i);

  std::string cmdName = cmd.substr(i1, i - i1);

  CStrUtil::skipSpace(cmd, &i);

  std::string cmdData = cmd.substr(i);

  if (cmdName == "")
    return true;

  CommandProc proc = lookupCommand(cmdName);

  if (proc == NULL)
    return false;

  CALL_MEMBER_FN(*this, proc)(cmdName, cmdData);

  return true;
}

bool
CTextFileCmd::
readCommand(const std::string &, const std::string &data)
{
  return file_->read(data.c_str());
}

bool
CTextFileCmd::
writeCommand(const std::string &, const std::string &data)
{
  return file_->write(data.c_str());
}

bool
CTextFileCmd::
moveToCommand(const std::string &, const std::string &data)
{
  std::vector<std::string> words;

  CStrUtil::toWords(data, words);

  long x, y;

  if (words.size() != 2 ||
      ! CStrUtil::toInteger(words[0], &x) ||
      ! CStrUtil::toInteger(words[1], &y))
    return false;

  file_->moveTo(x, y);

  return true;
}

bool
CTextFileCmd::
rmoveToCommand(const std::string &, const std::string &data)
{
  std::vector<std::string> words;

  CStrUtil::toWords(data, words);

  long x, y;

  if (words.size() != 2 ||
      ! CStrUtil::toInteger(words[0], &x) ||
      ! CStrUtil::toInteger(words[1], &y))
    return false;

  file_->rmoveTo(x, y);

  return false;
}

bool
CTextFileCmd::
addCharAfterCommand(const std::string &, const std::string &data)
{
  if (data.size() != 1) return false;

  file_->addCharAfter(data[0]);

  return true;
}

bool
CTextFileCmd::
addCharBeforeCommand(const std::string &, const std::string &data)
{
  if (data.size() != 1) return false;

  file_->addCharBefore(data[0]);

  return true;
}

bool
CTextFileCmd::
addLineAfterCommand(const std::string &, const std::string &data)
{
  if (data.empty()) return false;

  file_->addLineAfter(data);

  return true;
}

bool
CTextFileCmd::
addLineBeforeCommand(const std::string &, const std::string &data)
{
  if (data.empty()) return false;

  file_->addLineBefore(data);

  return true;
}

bool
CTextFileCmd::
deleteCharAtCommand(const std::string &, const std::string &)
{
  file_->deleteCharAt();

  return true;
}

bool
CTextFileCmd::
deleteCharBeforeCommand(const std::string &, const std::string &)
{
  file_->deleteCharBefore();

  return true;
}

bool
CTextFileCmd::
deleteLineAtCommand(const std::string &, const std::string &)
{
  file_->deleteLineAt();

  return true;
}

bool
CTextFileCmd::
deleteLineBeforeCommand(const std::string &, const std::string &)
{
  file_->deleteLineBefore();

  return true;
}

bool
CTextFileCmd::
replaceCharCommand(const std::string &, const std::string &data)
{
  if (data.size() != 1) return false;

  file_->replaceChar(data[0]);

  return true;
}

bool
CTextFileCmd::
replaceLineCommand(const std::string &, const std::string &data)
{
  file_->replaceLine(data);

  return true;
}

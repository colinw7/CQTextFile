#ifndef CTEXT_FILE_VI_KEY_H
#define CTEXT_FILE_VI_KEY_H

#include <CTextFileKey.h>
#include <CTextFile.h>
#include <map>

class CTextFileBuffer;
class CTextFileMarks;

class CTextFileViKey : public CTextFileKey, public CTextFileNotifier, public CTextFileEdNotifier {
 private:
  class LastCommand {
   public:
    LastCommand(CTextFile *file);

    CTextFile *file() const { return file_; }

    void clear();

    void addCount(uint n);

    void addKey(CKeyType key);

    void exec();

   private:
    CTextFile             *file_ { nullptr };
    std::vector<CKeyType>  keys_;
  };

 public:
  CTextFileViKey(CTextFile *file);
 ~CTextFileViKey();

  CTextFileEd *getEd() const { return ed_; }

  bool getVisual() const { return visual_; }
  void setVisual(bool b) { visual_ = b; }

  void fileOpened(const std::string &fileName);

  void setInsertMode(bool insert_mode);
  bool getInsertMode() const { return insertMode_; }

  bool getCmdLineMode() const { return cmdLineMode_; }

  void processChar(CKeyType key, const std::string &text, CEventModifier modifier);

  void execCmd(const std::string &cmd);

  void processInsertChar (CKeyType key, const std::string &text, CEventModifier modifier);
  void processCommandChar(CKeyType key, const std::string &text, CEventModifier modifier);
  void processNormalChar (CKeyType key, const std::string &text, CEventModifier modifier);
  void processControlChar(CKeyType key, const std::string &text, CEventModifier modifier);

  void normalInsertChar(CKeyType key, const std::string &text, CEventModifier modifier);

  bool processMoveChar(CKeyType key, const std::string &text,
                       CEventModifier modifier, CIPoint2D &new_pos);

  void setOptionString(const std::string &name, const std::string &arg);

  bool doFindChar(char c, uint count, bool forward, bool till);

  bool rmoveTo(int dx, int dy);
  bool moveTo(uint x, uint y);

  void edNotifyQuit(bool force);

  void error(const std::string &mgs) const;

 private:
  typedef std::map<std::string,std::string> OptionMap;

  struct Options {
    bool ignorecase { false };
    bool list       { false };
    bool number     { false };
    bool showmatch  { false };
    uint shiftwidth { 2 };

    Options() { }
  };

  CTextFileMarks  *marks_       { nullptr };
  CTextFileBuffer *buffer_      { nullptr };
  CTextFileEd     *ed_          { nullptr };
  CKeyType         lastKey_;
  uint             count_       { 0 };
  bool             insertMode_  { false };
  bool             cmdLineMode_ { false };
  char             register_    { '\0' };
  LastCommand      lastCommand_;
  char             findChar_    { '\0' };
  bool             findForward_ { false };
  bool             findTill_    { false };
  bool             visual_      { false };
  OptionMap        optionMap_;
  Options          options_;
};

#endif

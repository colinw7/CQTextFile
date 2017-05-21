#ifndef CTEXT_FILE_UTIL_H
#define CTEXT_FILE_UTIL_H

#include <sys/types.h>
#include <string>

class CTextFile;
class CRegExp;

class CTextFileUtil {
 public:
  CTextFileUtil(CTextFile *file);

  uint getShiftWidth() const { return shiftWidth_; }

  void deleteWord();
  void deleteWord(uint line_num, uint char_num);

  void deleteEOL();
  void deleteEOL(uint line_num, uint char_num);

  void shiftLeft(uint line_num1, uint line_num2);
  void shiftRight(uint line_num1, uint line_num2);

  void nextWord();
  void nextWord(uint *line_num, uint *char_num);
  void nextWORD();
  void nextWORD(uint *line_num, uint *char_num);

  void prevWord();
  void prevWord(uint *line_num, uint *char_num);
  void prevWORD();
  void prevWORD(uint *line_num, uint *char_num);

  void endWord();
  void endWord(uint *line_num, uint *char_num);
  void endWORD();
  void endWORD(uint *line_num, uint *char_num);

  bool getWord(std::string &word);
  bool getWord(uint line_num, uint char_num, std::string &word);

  void nextSentence();
  void nextSentence(uint *line_num, uint *char_num);
  void prevSentence();
  void prevSentence(uint *line_num, uint *char_num);

  void nextParagraph();
  void nextParagraph(uint *line_num, uint *char_num);
  void prevParagraph();
  void prevParagraph(uint *line_num, uint *char_num);

  void nextSection();
  void nextSection(uint *line_num, uint *char_num);
  void prevSection();
  void prevSection(uint *line_num, uint *char_num);

  bool nextLine(uint *line_num, uint *char_num);
  bool prevLine(uint *line_num, uint *char_num);

  void addChars(uint line_num, uint char_num, const std::string &chars);

  bool isWordChar(char c) const;

  bool isBlank(const std::string &line) const;

  bool isSentenceEnd(const std::string &line, uint pos, uint *n) const;

  bool isSection(const std::string &line, uint, uint *n) const;

  bool findNext(const std::string &pattern, uint line_num1, int char_num1,
                int line_num2, int char_num2, uint *fline_num, uint *fchar_num);
  bool findNext(const CRegExp &pattern, uint line_num1, int char_num1,
                int line_num2, int char_num2, uint *fline_num, uint *fchar_num, uint *len);

  bool lineFindNext(const std::string &line, const std::string &pattern,
                    int char_num1, int char_num2, uint *char_num) const;
  bool lineFindNext(const std::string &line, const CRegExp &pattern,
                    int char_num1, int char_num2, uint *spos, uint *epos) const;

  bool findPrev(const std::string &pattern, uint line_num1, int char_num1,
                int line_num2, int char_num2, uint *fline_num, uint *fchar_num);
  bool findPrev(const CRegExp &pattern, uint line_num1, int char_num1,
                int line_num2, int char_num2, uint *fline_num, uint *fchar_num, uint *len);

  bool lineFindPrev(const std::string &line, const std::string &pattern,
                    int char_num1, int char_num2, uint *char_num) const;
  bool lineFindPrev(const std::string &line, const CRegExp &pattern,
                    int char_num1, int char_num2, uint *spos, uint *epos) const;

  bool findNextChar(uint line_num, int char_num, char c, bool multiline);
  bool findNextChar(uint line_num, int char_num, const std::string &chars, bool multiline);

  bool findPrevChar(uint line_num, int char_num, char c, bool multiline);
  bool findPrevChar(uint line_num, int char_num, const std::string &chars, bool multiline);

  void joinLine();
  void joinLine(uint line_num);

  void splitLine();

  void moveLine(uint line_num1, int line_num2);

  void copyLine(uint line_num1, uint line_num2);

  void replace(uint line_num, uint char_num1, uint char_num2, const std::string &replaceStr);

  void deleteTo(uint line_num1, uint char_num1, uint line_num2, uint char_num2);

  //void deleteChars(uint line_num, uint char_num, int num);

  void deleteChars(uint line_num, uint char_num, uint n);

  void moveToFirstNonBlankUp();
  void moveToFirstNonBlankDown();
  void moveToFirstNonBlank();
  void moveToNonBlank();

  void swapChar();
  void swapChar(uint line_num, uint char_num);

 private:
  CTextFile *file_;
  uint       shiftWidth_;
};

#endif

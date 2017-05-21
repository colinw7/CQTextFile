#ifndef CTEXT_BUFFER_H
#define CTEXT_BUFFER_H

#include <string>
#include <vector>
#include <map>
#include <sys/types.h>

class CTextFile;
class CTextFileUtil;

// class to store set of named buffers (yanked parts of file)
class CTextFileBuffer {
 private:
  struct BufferLine {
    std::string line;
    bool        newline { false };

    BufferLine(const std::string &line1, bool newline1) :
     line(line1), newline(newline1) {
    }

    const std::string &getLine() const { return line; }

    bool getNewLine() const { return newline; }
  };

  struct Buffer {
    typedef std::vector<BufferLine> LineList;

    LineList lines;

    Buffer() :
     lines() {
    }

    void clear() {
      lines.clear();
    }

    uint getNumLines() const {
      return lines.size();
    }

    const BufferLine &getLine(uint i) const {
      return lines[i];
    }

    void addLine(const std::string &line, bool newline) {
      lines.push_back(BufferLine(line, newline));
    }
  };

 public:
  CTextFileBuffer(CTextFile *file);
 ~CTextFileBuffer();

  void yankLines(char id, uint n);
  void yankLines(char id, uint line_num, uint n);

  void yankWords(char id, uint n);
  void yankWords(char id, uint line_num, uint char_num, uint n);

  void yankChars(char id, uint n);
  void yankChars(char id, uint line_num, uint char_num, uint n);

  void yankTo(char id, uint line_num, uint char_num, bool is_line);
  void yankTo(char id, uint line_num1, uint char_num1,
              uint line_num2, uint char_num2, bool is_line);

  void yankClear(char id);

  void subYankTo(char id, uint line_num1, uint char_num1,
                 uint line_num2, uint char_num2, bool is_line);

  void pasteAfter(char id);
  void pasteAfter(char id, uint line_num, uint char_num);
  void pasteBefore(char id);
  void pasteBefore(char id, uint line_num, uint char_num);

 private:
  void cursorToLeft();

  void splitLine(uint line_num, uint char_num);

  void addChars(uint line_num, uint char_num, const std::string &chars);

  void addLine(uint line_num, const std::string &line);

  Buffer &getBuffer(char id);

 private:
  typedef std::map<char,Buffer> BufferMap;

  CTextFile     *file_ { nullptr };
  CTextFileUtil *util_ { nullptr };
  BufferMap      buffer_map_;
};

#endif

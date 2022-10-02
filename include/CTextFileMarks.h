#ifndef CTEXT_MARKS_H
#define CTEXT_MARKS_H

class CTextFile;

#include <CIPoint2D.h>
#include <string>
#include <map>
#include <sys/types.h>

class CTextFileMarks {
 public:
  typedef std::map<std::string,CIPoint2D> MarkList;

 public:
  CTextFileMarks(CTextFile *file) :
   file_(file) {
  }

  uint getNumMarks() const { return uint(marks_.size()); }

  const MarkList &getMarks() const { return marks_; }

  void markReturn();

  void setMarkPos(const std::string &mark);
  void setMarkPos(const std::string &mark, uint line_num, uint char_num);
  bool getMarkPos(const std::string &mark, uint *line_num, uint *char_num) const;
  void unsetMarkPos(const std::string &mark);
  void clearLineMarks(uint line_num);

  void displayMarks();

 private:
  CTextFile *file_ { nullptr };
  MarkList   marks_;
};

#endif

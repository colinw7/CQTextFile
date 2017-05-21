#ifndef CTEXT_FILE_NORMAL_KEY_H
#define CTEXT_FILE_NORMAL_KEY_H

#include <CTextFileKey.h>

class CTextFileNormalKey : public CTextFileKey {
 public:
  CTextFileNormalKey(CTextFile *file);
 ~CTextFileNormalKey();

  void processChar(CKeyType key, const std::string &text, CEventModifier modifier);

  void execCmd(const std::string &cmd);

  void processNormalChar(CKeyType key, const std::string &text, CEventModifier modifier);
  void processControlChar(CKeyType key, const std::string &text, CEventModifier modifier);

  void clearSelection();
};

#endif

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

extern bool FilterDBEntry(std::vector<std::string> &ICL, std::string &IF,
                          std::string &ID, std::string &Exe);
extern bool GetTestConfigurations(std::vector<std::string> &TC, std::string &X,
                                  std::string &E);
extern bool GetTestStages(std::vector<std::string> &TS, std::string &TC);
extern bool GetTestCommandLine(std::vector<std::string> &OCL,
                               std::vector<std::string> &ICL, std::string &TC,
                               std::string &TS, std::string &IF,
                               std::string &OF);
extern bool GetTestExtension(std::string &E, std::string &TS);
extern bool IsTestOk(int &I, std::string &TS);

extern bool GetS2SCommandLine(std::vector<std::string> &OCL,
                              std::vector<std::string> &ICL, std::string &IF,
                              std::string &OF, std::string &Exe);
extern bool GetS2SExtension(std::string &E);
extern bool IsS2SOk(int &I);

extern bool GetEditorCommandLine(std::vector<std::string> &OCL,
                                 std::vector<std::string> &ICL, std::string &IF,
                                 std::string &OF, std::string &Exe);
extern bool GetEditorExtension(std::string &E);
extern bool IsEditorOk(int &I);
extern bool IsOverWriteOk();
extern bool GetDiffCommandLine(std::vector<std::string> &OCL, std::string &AF,
                               std::string &BF);
extern bool IsDiffOk(int &I);
#endif

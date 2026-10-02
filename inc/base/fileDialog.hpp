#ifndef FILE_DIALOG_HPP
#define FILE_DIALOG_HPP

#define USE_NATIVE_FILE_DIALOG 1         // set to 1 to use nativefiledialog library, or 0 to use imgui_addons::ImGuiFileDialog
#define FILE_DIALOG_SIZE Vec2f(960, 690) // for imgui dialog (size of window)

#include <string>
#include <vector>

#if USE_NATIVE_FILE_DIALOG // use nativefiledialog library
#include <thread>
#include <nfd.h>
#else                      // use imgui_addons::ImGuiFileDialog
namespace imgui_addons { class ImGuiFileBrowser; }
#endif // USE_NATIVE_FILE_DIALOG


// forward declarations
class NodeGraph;

enum DialogType
  {
   DIALOG_LOAD = 0,
   DIALOG_SAVE,
  };
  
class FileDialog
{
private:
  NodeGraph  *mGraph = nullptr;
  std::vector<std::string> mExtensions;     // list of visible file extensions
  std::string mTitle        = "";           // title of dialog window
  DialogType  mType         = DIALOG_LOAD;  // type of file dialog
  std::string mDirectory    = "";           // starting directory of dialog
  std::string mFilters      = "";           // file extension filters (derived from mExtensions)
  std::string mPath         = "";           // returned path
  int  mSaveDefault  = -1;                  // default mExtensions index for save file path (no extension enforcement if -1)
  bool mOpen         = false;               // true if a file dialog is open
  bool mReady        = false;               // true if an open file dialog has finished
  bool mSuccess      = false;               // true if dialog was successful (valid path returned)
    
#if USE_NATIVE_FILE_DIALOG // use nativefiledialog library
  std::thread mThread;
#else                      // use imgui_addons::ImGuiFileDialog
  imgui_addons::ImGuiFileBrowser *mDialog = nullptr;
#endif // USE_NATIVE_FILE_DIALOG

  bool fixSaveExtension();
    
public:
  FileDialog(NodeGraph *graph=nullptr);
  ~FileDialog();

  void setGraph(NodeGraph *graph)    { mGraph = graph; }
  DialogType getType() const         { return mType; }
    
  bool isOpen() const                { return mOpen; }
  bool hasPath() const               { return (mReady && mSuccess && !mPath.empty()); }
  std::string& getPath()             { return mPath; }
  const std::string& getPath() const { return mPath; }

  bool success() const { return mSuccess; }
  bool ready() const { return mReady; }

  // set extension
  void setExtensions(const std::vector<std::string> &extensions, int saveDefault=-1)
  {
    mExtensions  = extensions;
    mSaveDefault = saveDefault;
  }
  bool open(const std::string &title, DialogType type, const std::string &directory, const std::vector<std::string> &extensions, int saveDefault=0);
  bool check();
    
  // tool for easy save/load button
  std::string drawButton(const std::string &btnStr, FileDialog *fd, const std::string title, DialogType type,
                         const std::string &directory=".", const std::vector<std::string> &extensions={"*.*"}, int saveDefault=0);
};


#endif // FILE_DIALOG_HPP

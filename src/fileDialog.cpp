#include "fileDialog.hpp"
using namespace astro;

#include "tools.hpp"
#include "nodeGraph.hpp"

#if USE_NATIVE_FILE_DIALOG
#else
#include "ImGuiFileBrowser.h"
using namespace imgui_addons;
#endif // USE_NATIVE_FILE_DIALOG

FileDialog::FileDialog(NodeGraph *graph)
  : mGraph(graph)
{
#if !USE_NATIVE_FILE_DIALOG
  // create ImGuiFileDialog
  // mDialog = new imgui_addons::ImGuiFileBrowser();
#endif // USE_NATIVE_FILE_DIALOG
}

FileDialog::~FileDialog()
{
  
#if !USE_NATIVE_FILE_DIALOG // imgui_addons::ImGuiFileBrowser
  if(mDialog) { delete mDialog; }
  mDialog = nullptr;
#endif // !USE_NATIVE_FILE_DIALOG
}


bool FileDialog::fixSaveExtension()
{
  if(!mPath.empty() && mSaveDefault >= 0 && mSaveDefault < mExtensions.size())
    {
      std::string fileExt = getFileExtension(mPath);
#if USE_NATIVE_FILE_DIALOG
      if(fileExt.size() > 0 && fileExt[0] == '.') { fileExt = fileExt.substr(1); } // remove period
#endif
      std::string ext = mExtensions[mSaveDefault];
      std::cout << "path extension: " << fileExt << " | needed: " << ext << "\n";
      if(ext != fileExt)
        { // fix extension
          std::cout << "FILE EXTENSION: '" << fileExt << "' --> adding .ags extension...\n";
          mPath += ".ags";
          std::cout << " --> " << mPath << "\n";
        }
      return true;
    }
  else { return false; }
}

// updates file dialog (if necessary) and checks for result -- returns true when file dialog has exited
bool FileDialog::check()
{
#if USE_NATIVE_FILE_DIALOG // check nfd thread
  
  if(!mOpen && mReady) { mThread.join(); }
  
#else  // update/draw ImGuiFileBrowser
  
  if(mOpen && mDialog)
    {
      if(mDialog->showFileDialog(mTitle, (mType == DIALOG_SAVE ? ImGuiFileBrowser::DialogMode::SAVE : ImGuiFileBrowser::DialogMode::OPEN),
                                 FILE_DIALOG_SIZE, mFilters))
        {
          mPath = mDialog->selected_path;
          std::string ext = getFileExtension(mPath);
          mOpen  = false;
          mReady = true;
          mSuccess = true;
        }
      else if(mDialog->isClosed)
        {
          std::cout << "File dialog cancelled.\n";
          mOpen = false;
          mReady = true;
        }
      if(!mOpen)
        { delete mDialog; mDialog = nullptr; }
    }

#endif // !USE_NATIVE_FILE_DIALOG
  
  if(mReady)
    {
      // make sure path is valid
      if(mPath.empty())
        { mSuccess = false; }
      if(mType == DIALOG_SAVE && !fixSaveExtension())
        { mSuccess = false; }

      mReady = false;
      if(mGraph) { mGraph->setLocked(false); }
      return true;
    }
  return false;
}

bool FileDialog::open(const std::string &title, DialogType type, const std::string &directory, const std::vector<std::string> &extensions, int saveDefault)
{
  if(!mOpen)
    {
      mReady   = false;
      mSuccess = false;
      mTitle      = title;
      mType       = type;
      mDirectory  = directory;
      mExtensions = extensions;
      mSaveDefault = (mExtensions.size() == 0) ? -1 : saveDefault; // use first extension as default
      mPath       = "";
      std::cout << "FILE DIALOG -->\n"
                << "   TITLE:     " << mTitle << "\n"
                << "   TYPE:      " << (mType == DIALOG_LOAD ? "LOAD" : "SAVE") << "\n"
                << "   DIRECTORY: " << mDirectory << "\n";
      
      if(mGraph) { mGraph->deselectAll(); mGraph->setLocked(true); }
      mOpen = true;
      
#if USE_NATIVE_FILE_DIALOG // nativefiledialog
      
      mThread =
        std::thread([&]()
                    {
                      // file extension filters
                      mFilters = "";
                      for(int i = 0; i < mExtensions.size(); i++)
                        {
                          if(mExtensions[i] == "*.*")
                            {
                              mExtensions.erase(mExtensions.begin()+i);
                              i--; continue;
                            }
                          // remove periods
                          mExtensions[i].erase(std::remove(mExtensions[i].begin(), mExtensions[i].end(), '.'), mExtensions[i].end());
                          // append to filters
                          mFilters.append(mExtensions[i]+(i < mExtensions.size()-1 ? "," : ""));
                        }

                      std::cout << "   FILTERS: " << mFilters << "\n";
      
                      nfdchar_t *outPath = nullptr;                      
                      nfdresult_t result;
                      if(mType == DIALOG_LOAD)
                        { result = NFD_OpenDialog((mFilters.empty() ? NULL : mFilters.c_str()), (mDirectory.empty() ? NULL : mDirectory.c_str()), &outPath); }
                      else if(mType == DIALOG_SAVE)
                        { result = NFD_SaveDialog((mFilters.empty() ? NULL : mFilters.c_str()), (mDirectory.empty() ? NULL : mDirectory.c_str()), &outPath); }
                      
                      if(result == NFD_OKAY)
                        {
                          mPath = std::string(outPath);
                          free(outPath);
                          std::cout << "File " << (mType == DIALOG_SAVE ? "saving" : "loading") << " success (path: " << mPath << ")\n";
                          mSuccess = true;
                        }
                      else if(result == NFD_CANCEL) { std::cout << "File dialog cancelled.\n"; }
                      else                          { std::cout << "Error: " << NFD_GetError() << "\n"; }
                      mReady = true;
                      mOpen  = false;
                    });
      
      
#else // imgui_addons::ImGuiFileBrowser
      // file extension filters
      mFilters = "";
      for(int i = 0; i < mExtensions.size(); i++)
        {
          // append to filters
          mFilters.append(mExtensions[i]);
          if(i != mExtensions.size()-1) { mFilters.append(","); } // commas between extensions
        }
      if(mFilters.empty()) { mFilters = "*.*"; }
      std::cout << "   FILTERS:   " << mFilters << "\n";

      if(mDialog) { delete mDialog; }
      mDialog = new imgui_addons::ImGuiFileBrowser(mDirectory);
      
      ImGui::OpenPopup(mTitle.c_str()); // open imgui popup
      
#endif // USE_NATIVE_FILE_DIALOG
    }
  return true;
}

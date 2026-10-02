#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "keyBinding.hpp"
#include "viewSettings.hpp"
#include "vector.hpp"

#include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;

#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <map>

#define CLOCK_T std::chrono::high_resolution_clock // clock type for FPS calculation
#define FPS_UPDATE_INTERVAL 0.1                    // FPS interval (seconds)

#define ENABLE_IMGUI_DEMO   true
#define DEFAULT_PROJECT_DIR "./projects"
#define CONFIG_FILE_PATH    "astro.conf"

#define NEW_PROJECT_NAME "NewProject"

#define GRAPH_PADDING  Vec2f( 5.0f,  5.0f)
#define TABBAR_PADDING Vec2f(10.0f, 10.0f)
#define POPUP_PADDING  Vec2f(10.0f, 10.0f)

#define TABBAR_WIDTH 0.0f

// forward declarations
class GLFWwindow;
class SettingsForm;

// forward declarations
class Node;
class NodeGraph;
class NodeList;
class TabMenu;
class FileDialog;

// for categorizing key bindings
struct KeyBindingGroup
{
  std::string name = "";             // group name
  std::vector<std::string> bindings; // binding names
  std::vector<int>         ids;      // binding indices
};

struct Popup
{
  std::string name = "";
  Vec2f       size = Vec2f(-1, -1);
  std::function<void(Popup &p)> draw = nullptr; // draw function
  int popupFlags; // window flags for outer modal popup window
  int childFlags; // window flags for inner child window (scrolls by default)
  bool childBorder = false; // if true, draws border around scrollable child window
    
  bool autoSizeX   = true;  // autosize popup window horizontally
  bool autoSizeY   = true;  // autosize popup window vertically
  bool open        = false; // not open to start
};

struct MainProject
{
  std::string path      = "";
  std::string name      = NEW_PROJECT_NAME;
  NodeGraph  *graph     = nullptr;
  bool        open      = true;
  bool        selected  = true;
};
  
class MainWindow
{
private:
  //// (used to be globals -- TODO: restructure) ///////////////

  // interaction thread
  CLOCK_T::time_point tNow;      // current frame time
  CLOCK_T::time_point tLast;     // last frame time
  double dt      = 0.0;          // time difference
  double tDiff   = 0.0;          // time difference accumulator
  int    nFrames = 0;            // number of frames this interval
  double fpsLast = 0.0;          // previous FPS value
  // update thread
  CLOCK_T::time_point tNowU;     // current frame time
  CLOCK_T::time_point tLastU;    // last frame time
  double dtU      = 0.0;         // time difference
  double tDiffU   = 0.0;         // time difference accumulator
  int    nFramesU = 0;           // number of frames this interval
  double fpsLastU = 0.0;         // previous FPS value

  static void windowCloseCallback(GLFWwindow *window);
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  /////////////////////////////////////////////////////////////

  std::vector<Popup> mPopups;
  std::map<std::string, Popup*> mPopupMap;
    
  bool mShowFps        = true;  // if true, FPS drawn in lower-left corner
  bool mShowDemo       = false; // true if imgui demo window showing (also shows node IDs)
  bool mUpdating       = true;  // set to false to stop update thread
  
  bool mCancelDebounce = false; // when program is closing, then false once escape key is released
  bool mClosing        = false; // set to true when program is being closed
  bool mNoSave         = false; // set to true if unsaved changes should be discarded

  GLFWwindow   *mWindow          = nullptr;
  FileDialog   *mFileDialog      = nullptr;
  ViewSettings *mViewSettings    = nullptr;
  SettingForm  *mKeyBindingForm  = nullptr;
  NodeList     *mNodeList        = nullptr;
  TabMenu      *mSideTabs        = nullptr;

  std::vector<MainProject> mProjects;
  int mActiveProject   =  0; // index of currently active project (selected in tab bar)
  int mSelectedProject = -1; // >= 0 if user selected a different project in the tab bar
  int mClosingProject  = -1; // >= 0 if user is closing project tab
  MainProject* activeProject()
  {
    return (mProjects.size()  >  0 ? ((mActiveProject >= 0) ?
                                      (mProjects[mActiveProject].graph ? &mProjects[mActiveProject] : nullptr)
                                      : &mProjects.back()) : nullptr);
  }
  MainProject* closingProject()  { return ((mClosingProject  >= 0) ? &mProjects[mClosingProject]  : nullptr); }
  MainProject* selectedProject() { return ((mSelectedProject >= 0) ? &mProjects[mSelectedProject] : nullptr); }
    
  std::thread mUpdateThread;
  std::mutex  mUpdateMutex;
  std::mutex  mProjectMutex;
  std::mutex  mKeyMutex;

  Vec2f mFrameSize;
    
  std::string mProjectDir = DEFAULT_PROJECT_DIR;

  // key binding definitions
  std::vector<KeyBinding>      mDefaultKeyBindings; // global window key bindings
  std::vector<KeyBinding>      mKeyBindings;        // global window key bindings
  std::vector<KeyBindingGroup> mKeyBindingGroups;   // named groups for displaying bindings
  std::vector<KeyPress>        mKeySequence;        // sequence of key presses (current)
  KeyBinding *mBindingEdit = nullptr;
  KeyBinding  mOldBinding;
  int mCancelKey = GLFW_KEY_ESCAPE; // needs debouncing -- TODO: centralize imgui key checks (?)

  // node placing
  bool        mPlacing   = false;
  std::string mPlaceType = "";
  Node       *mPlaceNode = nullptr;
  void startPlacing(const std::string &type);
  void stopPlacing(bool deleteNode=true, bool selectHovered=false);
  void handlePlacing();
    
  // copy/paste
  std::vector<Node*> mClipboard; // node clipboard (for copy/pasting between projects)
  bool mPasting = false; // true if pasting clipboard nodes
  void startPasting();
  void stopPasting();
  void handlePasting();
    
  bool checkProjectDir();
  std::vector<MainProject*> getUnsavedProjects();

  // actions
  void escape(); // escape key action
  void projectOpen();
  void projectSave();
  void projectSaveAs();
  MainProject* newProject();
  void prevProject();
  void nextProject();
  void quit();
  void saveConfig();
  void loadConfig();

  bool undo();
  bool redo();

  // graph actions
  void selectAll();
  void quitPlacing();
  void groupNodes();
  void ungroupNodes();

  // draw/update
  void drawMenuBar();
  void drawProjectTabs();
  void handleFileDialog();
  void handleKeyBindings();

  // popups
  void openPopup(const std::string &name);
  void togglePopup(const std::string &name);
  void drawPopup(Popup &popup);
  void drawAbout(Popup &popup);
  void drawKeyBinding(KeyBinding &kb, const KeyBinding &defaultKb); // draw single key binding
  void drawKeyBindings(Popup &popup);
  void drawViewSettings(Popup &popup);
  void drawExitUnsavedAlert(Popup &popup);
  void drawClosingUnsavedAlert(Popup &popup);
    
public:
  MainWindow(GLFWwindow *window);
  ~MainWindow();

  void init(bool mainThread=true);
  void keyPress(int mods, int key, bool press);

  void cut();
  void copy();
  bool isPasting() const { return mPasting; }
  bool isPlacing() const { return mPlacing; }
    
  NodeGraph* graph()           { return (activeProject() ? activeProject()->graph : nullptr); }
  ViewSettings* viewSettings() { return mViewSettings; }
    
  void draw(const Vec2i &frameSize);
  void update();
    
  double calcFps(bool updateThread=false);
};

#endif // MAIN_WINDOW_HPP

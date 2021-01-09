#ifndef ASTRO_WINDOW_HPP
#define ASTRO_WINDOW_HPP

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

#define NEW_PROJECT_NAME "New Project"

// forward declarations
class GLFWwindow;
class SettingsForm;

namespace astro
{
  // forward declarations
  class NodeGraph;
  class NodeList;
  class FileDialog;

  struct Popup
  {
    std::string name = "";
    Vec2f       size = Vec2f(-1, -1);
    std::function<void(Popup &p)> draw     = nullptr; // draw function
    int popupFlags; // window flags for outer modal popup window
    int childFlags; // window flags for inner child window (scrolls by default)
    bool childBorder = false; // if true, draws border around scrollable child window
    
    bool autoSizeX   = true;  // autosize popup window horizontally
    bool autoSizeY   = true;  // autosize popup window vertically
    bool open        = false; // not open to start
  };

  struct AstroProject
  {
    std::string path      = "";
    std::string name      = NEW_PROJECT_NAME;
    NodeGraph  *graph     = nullptr;
    bool        open      = true;
    bool        selected  = true;
  };
  
  class AstroWindow
  {
  private:
    //// (used to be globals -- TODO: restructure) ///////////////
    CLOCK_T::time_point tNow;      // current frame time
    CLOCK_T::time_point tLast;     // last frame time
    double dt      = 0.0;          // time difference
    double tDiff   = 0.0;          // time difference accumulator
    int    nFrames = 0;            // number of frames this interval
    double fpsLast = 0.0;          // previous FPS value
    static void windowCloseCallback(GLFWwindow *window);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    /////////////////////////////////////////////////////////////

    std::vector<Popup> mPopups;
    std::map<std::string, Popup*> mPopupMap;
    
    bool mShowFps        = true;  // if true, FPS drawn in lower-left corner
    bool mShowDemo       = false; // true if imgui demo window showing (also shows node IDs)
    bool mEscapeDebounce = false; // when program is closing, then false once escape key is released
    bool mClosing        = false; // set to true when program is being closed
    bool mNoSave         = false; // set to true if unsaved changes should be discarded

    GLFWwindow   *mWindow          = nullptr;
    FileDialog   *mFileDialog      = nullptr;
    ViewSettings *mViewSettings    = nullptr;
    SettingForm  *mKeyBindingForm  = nullptr;
    NodeList     *mNodeList        = nullptr;

    std::vector<AstroProject> mProjects;
    // AstroProject *mActiveProject   = nullptr;    // currently active project
    // AstroProject *mSelectedProject = nullptr;    // set if user selects a project tab
    // AstroProject *mClosingProject  = nullptr;    // set if user is closing a project without saving

    int mActiveProj   = 0;
    int mSelectedProj = -1;
    int mClosingProj  = -1;

    AstroProject* activeProject() { return ((mActiveProj >= 0) ? &mProjects[mActiveProj] : nullptr); }
    AstroProject* closingProject() { return ((mClosingProj >= 0) ? &mProjects[mClosingProj] : nullptr); }
    AstroProject* selectedProject() { return ((mSelectedProj >= 0) ? &mProjects[mSelectedProj] : nullptr); }
    
    std::thread mUpdateThread;
    std::mutex  mUpdateMutex;
    std::mutex  mProjectMutex;
    std::mutex  mKeyMutex;

    Vec2f mFrameSize;
    Vec2f mMenuBarSize;
    
    std::string mProjectDir = DEFAULT_PROJECT_DIR;

    // key binding definitions
    std::vector<KeyBinding> mDefaultKeyBindings; // global window key bindings
    std::vector<KeyBinding> mKeyBindings; // global window key bindings
    std::vector<KeyPress>   mKeySequence; // sequence of key presses
    KeyBinding *mBindingEdit = nullptr;
    KeyBinding  mOldBinding;
    int mCancelKey = GLFW_KEY_ESCAPE; // needs debouncing -- TODO: centralize imgui key checks (?)

    bool checkProjectDir();
    std::vector<AstroProject*> getUnsavedProjects();

    // actions
    void escape(); // escape key action
    void projectOpen();
    void projectSave();
    void projectSaveAs();
    AstroProject* newProject();
    void quit();
    void saveConfig();
    void loadConfig();

    // draw/update
    void drawMenuBar();
    void drawTabs();
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
    AstroWindow(GLFWwindow *window);
    ~AstroWindow();

    void init();

    void keyPress(int mods, int key, bool press);
    
    NodeGraph* graph()           { return (activeProject() ? activeProject()->graph : nullptr); }
    ViewSettings* viewSettings() { return mViewSettings; }
    
    void draw(const Vec2i &frameSize);
    void update();
    
    double calcFps();
  };
}

#endif // ASTRO_WINDOW_HPP

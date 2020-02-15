#ifndef VRB_VIEWER_DOT_H
#define VRB_VIEWER_DOT_H

#include "vrb/Forward.h"
#include "vrb/MacroUtils.h"
#include <jni.h>
#include <memory>
#include <string>

namespace vrb {

class Viewer;
typedef std::shared_ptr<Viewer> ViewerPtr;

class Viewer {
public:
  static ViewerPtr Create();
  void Pause();
  void Resume();
  void InitializeJava(JNIEnv* aEnv, jobject aActivity, jobject aAssets);
  void InitializeGL();
  void ShutdownJava();
  void ShutdownGL();
  void SetViewport(const int aWidth, const int aHeight);
  void Draw();
  void ScreenTap(const float aX, const float aY);
protected:
  struct State;
  Viewer(State& aState);
  ~Viewer();
private:
  State& m;
  Viewer() = delete;
  VRB_NO_DEFAULTS(Viewer)
};

} // namespace vrb

#endif // VRB_VIEWER_DOT_H

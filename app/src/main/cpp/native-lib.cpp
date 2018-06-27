#include <jni.h>
#include "Viewer.h"
#include "vrb/Logger.h"

vrb::Viewer&
GetViewer() {
  static vrb::ViewerPtr sViewer;
  if (!sViewer) {
    sViewer = vrb::Viewer::Create();
  }
  return *sViewer;
}

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
    Java_org_mozilla_vrbviewer_MainActivity_##method_name

extern "C" {

JNI_METHOD(void, updateViewport)
(JNIEnv*, jobject, jint aWidth, jint aHeight) {
  VRB_LOG("*** updateViewport");
  GetViewer().SetViewport(aWidth, aHeight);
}

JNI_METHOD(void, draw)
(JNIEnv*, jobject) {
  GetViewer().Draw();
}

JNI_METHOD(void, initializeJava)
(JNIEnv* aEnv, jobject aActivity, jobject aAssets) {
  VRB_LOG("*** initializeJava")
  GetViewer().InitializeJava(aEnv, aActivity, aAssets);
}

JNI_METHOD(void, shutdownJava)
(JNIEnv*, jobject) {
  VRB_LOG("*** shutdownJava")
  GetViewer().ShutdownJava();
}

JNI_METHOD(void, activityPaused)
(JNIEnv*, jobject) {
  VRB_LOG("*** activityPaused");
  GetViewer().Pause();
  GetViewer().ShutdownGL();
}

JNI_METHOD(void, activityResumed)
(JNIEnv*, jobject) {
  VRB_LOG("*** activityResumed");
  GetViewer().InitializeGL();
  GetViewer().Resume();
}

jint JNI_OnLoad(JavaVM* aVm, void*) {
  return JNI_VERSION_1_6;
}

void JNI_OnUnLoad(JavaVM* vm, void* reserved) {
}

} // extern "C"


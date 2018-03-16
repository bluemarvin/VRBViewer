#include <jni.h>
#include "Viewer.h"
#include "vrb/Logger.h"

static vrb::ViewerPtr sViewer;

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
    Java_org_mozilla_vrbviewer_MainActivity_##method_name

extern "C" {

JNI_METHOD(void, updateViewport)
(JNIEnv*, jobject, int aWidth, int aHeight) {
  VRB_LOG("*** updateViewport");
  sViewer->SetViewport(aWidth, aHeight);
}

JNI_METHOD(void, draw)
(JNIEnv*, jobject) {
  sViewer->Draw();
}

JNI_METHOD(void, initializeJava)
(JNIEnv* aEnv, jobject aActivity, jobject aAssets) {
  VRB_LOG("*** initializeJava")
  sViewer->InitializeJava(aEnv, aActivity, aAssets);
}

JNI_METHOD(void, shutdownJava)
(JNIEnv* aEnv, jobject aActivity, jobject aAssets) {
  VRB_LOG("*** shutdownJava")
  sViewer->ShutdownJava();
}

JNI_METHOD(void, activityPaused)
(JNIEnv*, jobject) {
  VRB_LOG("*** activityPaused");
  sViewer->Pause();
  sViewer->ShutdownGL();

}

JNI_METHOD(void, activityResumed)
(JNIEnv*, jobject) {
  VRB_LOG("*** activityResumed");
  sViewer->InitializeGL();
  sViewer->Resume();
}

jint JNI_OnLoad(JavaVM* aVm, void*) {
  sViewer = vrb::Viewer::Create();
  return JNI_VERSION_1_6;
}

void JNI_OnUnLoad(JavaVM* vm, void* reserved) {
  sViewer = nullptr;
}

} // extern "C"


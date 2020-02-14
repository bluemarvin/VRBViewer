#include "Viewer.h"
#include "vrb/ConcreteClass.h"

#include "vrb/AnimatedTransform.h"
#include "vrb/CameraSimple.h"
#include "vrb/Color.h"
#include "vrb/CullVisitor.h"
#include "vrb/DrawableList.h"
#include "vrb/GLError.h"
#include "vrb/Group.h"
#include "vrb/Light.h"
#include "vrb/Logger.h"
#include "vrb/Matrix.h"
#include "vrb/ModelLoaderAndroid.h"
#include "vrb/NodeFactoryObj.h"
#include "vrb/ParserObj.h"
#include "vrb/ProgramFactory.h"
#include "vrb/RenderContext.h"
#include "vrb/Transform.h"
#include "vrb/Toggle.h"
#include "vrb/Vector.h"

#include "vrb/gl.h"

namespace vrb {

struct Viewer::State {
  bool paused;
  bool initialized;
  float near;
  float far;
  RenderContextPtr context;
  CreationContextPtr creation;
  ModelLoaderAndroidPtr loader;
  GroupPtr root;
  LightPtr light;
  AnimatedTransformPtr model;
  CullVisitorPtr cullVisitor;
  DrawableListPtr drawList;
  CameraSimplePtr camera;
  Color clearColor;
  State() : paused(false), initialized(false), near(0.1f), far(100.0f) {
    context = RenderContext::Create();
    loader = ModelLoaderAndroid::Create(context);
    creation = context->GetRenderThreadCreationContext();
    context->GetProgramFactory()->SetLoaderThread(loader);
    root = Group::Create(creation);
    light = Light::Create(creation);
    root->AddLight(light);
    cullVisitor = CullVisitor::Create(creation);
    drawList = DrawableList::Create(creation);
    camera = CameraSimple::Create(creation);
    clearColor.SetRGB(1.0f, 0.0f, 0.0f);
  }
};

ViewerPtr
Viewer::Create() {
  return std::make_shared<ConcreteClass<Viewer, Viewer::State> >();
}

void
Viewer::Pause() {
  m.paused = true;
}

void
Viewer::Resume() {
  m.paused = false;
}

void
Viewer::InitializeJava(JNIEnv* aEnv, jobject aActivity, jobject aAssets) {
  m.context->InitializeJava(aEnv, aActivity, aAssets);
  m.loader->InitializeJava(aEnv, aActivity, aAssets);
  if (!m.model) {
    m.model = AnimatedTransform::Create(m.creation);
    m.model->AddRotationAnimation(Vector(0.0f, 1.0f, 0.0f), PI_FLOAT)
      .AddStaticTransform(Matrix::Position(Vector(0.3f, 0.0f, 0.0f)))
      .AddRotationAnimation(Vector(0.0f, 1.0f, 0.0f), -PI_FLOAT / 3.0f)
      .AddTranslationAnimation(Vector(0.0f, 0.0f, -1.0f), 1.0f);
    m.model->SetAnimationState(AnimationState::Play);
    m.root->AddNode(m.model);
    m.loader->LoadModel("vr_controller_oculusquest_right.obj", m.model);
  }
}

void
Viewer::InitializeGL() {
  if (m.initialized) {
    return;
  }
  m.initialized = m.context->InitializeGL();
  if (m.initialized) {
    m.loader->InitializeGL();
    VRB_LOG("*** Initialized GL");
    VRB_GL_CHECK(glEnable(GL_DEPTH_TEST));
    VRB_GL_CHECK(glEnable(GL_CULL_FACE));
    VRB_GL_CHECK(glEnable(GL_BLEND));
    VRB_GL_CHECK(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
  }
}

void
Viewer::ShutdownJava() {
  m.loader->ShutdownJava();
  m.context->ShutdownJava();
}

void
Viewer::ShutdownGL() {
  VRB_LOG("*** Shutdown GL");
  m.initialized = false;
  m.loader->ShutdownGL();
  m.context->ShutdownGL();
}

void
Viewer::LoadModel(const std::string& aUri) {

}

void
Viewer::ReloadCurrentModel() {

}

void
Viewer::SetViewport(const int aWidth, const int aHeight) {
  VRB_GL_CHECK(glViewport(0, 0, aWidth, aHeight));
  m.camera->SetClipRange(m.near, m.far);
  m.camera->SetViewport(aWidth, aHeight);
  if (aWidth > aHeight) {
    m.camera->SetFieldOfView(60.0f, -1.0f);
  } else {
    m.camera->SetFieldOfView(-1.0f, 60.0f);
  }
}


void
Viewer::Draw() {
  if (m.paused) {
    return;
  }
  if (!m.initialized) {
    InitializeGL();
    if (!m.initialized) {
      return;
    }
  }
  m.clearColor.SetRGB(
      (1.0f + std::sinf(std::fmod(static_cast<float>(m.context->GetTimestamp()), 2.0f * PI_FLOAT))) / 2.0f,
      (1.0f + std::sinf(std::fmod(static_cast<float>(m.context->GetTimestamp()) * 0.25f, 2.0f * PI_FLOAT))) / 2.0f,
      (1.0f + std::sinf(std::fmod(static_cast<float>(m.context->GetTimestamp()) + PI_FLOAT, 2.0f * PI_FLOAT))) / 2.0f
  );
  VRB_GL_CHECK(glClearColor(m.clearColor.Red(), m.clearColor.Green(), m.clearColor.Blue(), m.clearColor.Alpha()));
  VRB_GL_CHECK(glEnable(GL_BLEND));
  VRB_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  static double ltime = m.context->GetTimestamp();
  if ((m.context->GetTimestamp() - ltime) > 8.0) {
    ltime = m.context->GetTimestamp();
    m.model->ResetAnimations();
  }
  m.camera->SetTransform(Matrix::Translation(vrb::Vector(0.0f, 0.0f, 0.7f)));
  if (!m.context->IsOnRenderThread()) {
    VRB_LOG("UPDATING NOT ON RENDER THREAD!");
  }
  bool pause = std::fmod(std::floor(m.context->GetTimestamp()), 2.0f) == 0.0;
  m.model->SetAnimationState(pause ? AnimationState::Stop : AnimationState::Play);
  m.context->Update();
  m.drawList->Reset();
  m.root->Cull(*m.cullVisitor, *m.drawList);
  m.drawList->Draw(*m.camera);
}

Viewer::Viewer(State& aState) : m(aState) {}
Viewer::~Viewer() {}

} // namespace vrb
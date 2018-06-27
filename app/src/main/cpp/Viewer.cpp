#include "Viewer.h"
#include "vrb/ConcreteClass.h"

#include "vrb/CameraSimple.h"
#include "vrb/Color.h"
#include "vrb/CullVisitor.h"
#include "vrb/DrawableList.h"
#include "vrb/Light.h"
#include "vrb/Logger.h"
#include "vrb/GLError.h"
#include "vrb/Group.h"
#include "vrb/Matrix.h"
#include "vrb/NodeFactoryObj.h"
#include "vrb/ParserObj.h"
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
  NodeFactoryObjPtr factory;
  ParserObjPtr parser;
  GroupPtr root;
  LightPtr light;
  TransformPtr model;
  CullVisitorPtr cullVisitor;
  DrawableListPtr drawList;
  CameraSimplePtr camera;
  Color clearColor;
  State() : paused(false), initialized(false), near(0.1f), far(100.0f) {
    context = RenderContext::Create();
    creation = context->GetRenderThreadCreationContext();
    factory = NodeFactoryObj::Create(creation);
    parser = ParserObj::Create(creation);
    parser->SetObserver(factory);
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
  if (!m.model) {
    m.model = Transform::Create(m.creation);
    m.factory->SetModelRoot(m.model);
    m.parser->LoadModel("teapot.obj");
    //m.parser->LoadModel("TestCutting.obj");
    //m.parser->LoadModel("Landscape.obj");
    m.root->AddNode(m.model);
    m.factory->SetModelRoot(nullptr);
    // FIXME vrb::Transform should default to idenity.
    m.model->SetTransform(Matrix::Translation(Vector(0.0f, 0.0f, 0.0f)));
  }
}

void
Viewer::InitializeGL() {
  if (m.initialized) {
    return;
  }
  m.initialized = m.context->InitializeGL();
  if (m.initialized) {
    VRB_LOG("*** Initialized GL");
    VRB_GL_CHECK(glEnable(GL_DEPTH_TEST));
    VRB_GL_CHECK(glEnable(GL_CULL_FACE));
    VRB_GL_CHECK(glEnable(GL_BLEND));
    VRB_GL_CHECK(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
  }
}

void
Viewer::ShutdownJava() {
  m.context->ShutdownJava();
}

void
Viewer::ShutdownGL() {
  VRB_LOG("*** Shutdown GL");
  m.initialized = false;
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

static float sPitch = 0.0f; // PI_FLOAT / 4.0f; // 0.0f;
static float sHeading = 0.0f;
static const Vector sRight(1.0f, 0.0f, 0.0f);
static const Vector sUp(0.0f, 1.0f, 0.0f);
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
  VRB_GL_CHECK(glClearColor(m.clearColor.Red(), m.clearColor.Green(), m.clearColor.Blue(), m.clearColor.Alpha()));
  VRB_GL_CHECK(glEnable(GL_BLEND));
  VRB_GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  m.camera->SetTransform(Matrix::Translation(vrb::Vector(0.0f, 2.0f, 10.0f)));
  m.model->SetTransform(Matrix::Rotation(sRight, sPitch).PreMultiply(Matrix::Rotation(sUp, sHeading)));
  //m.model->SetTransform(Matrix::Rotation(sRight, PI_FLOAT / 8.0f));
  if (!m.context->IsOnRenderThread()) {
    VRB_LOG("UPDATING NOT ON RENDER THREAD!");
  }
  m.context->Update();
  m.drawList->Reset();
  m.root->Cull(*m.cullVisitor, *m.drawList);
  m.drawList->Draw(*m.camera);
  //sPitch += PI_FLOAT / 80.0f;
  if (sPitch >= (2 * PI_FLOAT)) { sPitch = 0.0f; }
  sHeading += PI_FLOAT / 200.0f;
  if (sHeading >= (2 * PI_FLOAT)) { sHeading = 0.0f; }
}

Viewer::Viewer(State& aState) : m(aState) {}
Viewer::~Viewer() {}

} // namespace vrb
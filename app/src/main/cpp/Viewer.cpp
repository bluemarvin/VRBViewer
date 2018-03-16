#include "Viewer.h"
#include "vrb/ConcreteClass.h"

#include "vrb/CameraSimple.h"
#include "vrb/Color.h"
#include "vrb/Context.h"
#include "vrb/CullVisitor.h"
#include "vrb/DrawableList.h"
#include "vrb/Light.h"
#include "vrb/Logger.h"
#include "vrb/GLError.h"
#include "vrb/Group.h"
#include "vrb/Matrix.h"
#include "vrb/NodeFactoryObj.h"
#include "vrb/ParserObj.h"
#include "vrb/Transform.h"
#include "vrb/Vector.h"

#include "vrb/gl.h"

namespace vrb {

struct Viewer::State {
  bool paused;
  bool initialized;
  float near;
  float far;
  ContextPtr contextPtr;
  ContextWeak context;
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
    contextPtr = Context::Create();
    context = contextPtr;
    factory = NodeFactoryObj::Create(context);
    parser = ParserObj::Create(context);
    parser->SetObserver(factory);
    root = Group::Create(context);
    light = Light::Create(context);
    root->AddLight(light);
    cullVisitor = CullVisitor::Create(context);
    drawList = DrawableList::Create(context);
    camera = CameraSimple::Create(context);
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
  m.contextPtr->InitializeJava(aEnv, aActivity, aAssets);
  if (!m.model) {
    m.model = Transform::Create(m.context);
    m.factory->SetModelRoot(m.model);
    m.parser->LoadModel("teapot.obj");
    m.root->AddNode(m.model);
    m.factory->SetModelRoot(nullptr);
    // FIXME vrb::Transform should default to idenity.
    m.model->SetTransform(Matrix::Translation(Vector(0.0f, -2.0f, 0.0f)));
  }
}

void
Viewer::InitializeGL() {
  if (m.initialized) {
    return;
  }
  m.initialized = m.contextPtr->InitializeGL();
  if (m.initialized) {
    VRB_LOG("*** Initialized GL");
    VRB_CHECK(glEnable(GL_DEPTH_TEST));
    VRB_CHECK(glEnable(GL_CULL_FACE));
  }
}

void
Viewer::ShutdownJava() {
  m.contextPtr->ShutdownJava();
}

void
Viewer::ShutdownGL() {
  VRB_LOG("*** Shutdown GL");
  m.initialized = false;
  m.contextPtr->ShutdownGL();
}

void
Viewer::LoadModel(const std::string& aUri) {

}

void
Viewer::ReloadCurrentModel() {

}

void
Viewer::SetViewport(const int aWidth, const int aHeight) {
  VRB_CHECK(glViewport(0, 0, aWidth, aHeight));
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
  VRB_CHECK(glClearColor(m.clearColor.Red(), m.clearColor.Green(), m.clearColor.Blue(), m.clearColor.Alpha()));
  VRB_CHECK(glEnable(GL_BLEND));
  VRB_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  m.camera->SetTransform(vrb::Matrix::Translation(vrb::Vector(0.0f, 0.0f, 10.0f)));
  m.contextPtr->Update();
  m.drawList->Reset();
  m.root->Cull(*m.cullVisitor, *m.drawList);
  m.drawList->Draw(*m.camera);
}

Viewer::Viewer(State& aState) : m(aState) {}
Viewer::~Viewer() {}

} // namespace vrb